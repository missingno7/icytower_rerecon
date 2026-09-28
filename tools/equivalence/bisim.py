"""Lockstep simulation of oracle vs candidate code modulo register allocation, stack slots and layout.
Both instruction streams are walked in parallel from the entry. Each side keeps an environment
register/slot -> value id (fresh id per definition, copies propagate ids). Two instructions match
when they have the same operation, immediates and memory shape and their used operand values
correspond under the incrementally built oracle<->candidate value bijection; their definitions are
then bound together. Unconditional jumps and padding are skipped; conditional branches are matched
by condition code, so inverted jumps and block order do not matter. Every state where the two sides
diverge is reported with the oracle line. Usage: bisim.py FUNCTION OBJ [DELTA]"""
import sys, re, collections
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import objfun as ltcompare
from objfun import DELTAS
REGS = ['eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp']
PARENT = {}
for r, subs in {'eax': ['ax', 'al', 'ah'], 'ebx': ['bx', 'bl', 'bh'], 'ecx': ['cx', 'cl', 'ch'], 'edx': ['dx', 'dl', 'dh'], 'esi': ['si'], 'edi': ['di'], 'ebp': ['bp'], 'esp': ['sp']}.items():
    PARENT[r] = r
    for s in subs: PARENT[s] = r
NOPS = ('nop', 'xchg %ax,%ax', 'lea 0x0(%esi),%esi', 'lea 0x0(%edi),%edi', 'mov %esi,%esi', 'mov %edi,%edi')
INVERT = {'e': 'ne', 'ne': 'e', 'l': 'ge', 'ge': 'l', 'le': 'g', 'g': 'le', 'b': 'ae', 'ae': 'b', 'be': 'a', 'a': 'be', 's': 'ns', 'ns': 's', 'p': 'np', 'np': 'p'}
SWAPCC = {'l': 'g', 'g': 'l', 'le': 'ge', 'ge': 'le', 'b': 'a', 'a': 'b', 'be': 'ae', 'ae': 'be', 'e': 'e', 'ne': 'ne'}
CALL_CLOBBER = ['eax', 'ecx', 'edx']

def is_nop(t): return any(t.startswith(n) for n in NOPS)

def split_ops(s):
    ops, depth, cur = [], 0, ''
    for ch in s:
        if ch == '(': depth += 1
        elif ch == ')': depth -= 1
        if ch == ',' and depth == 0: ops.append(cur.strip()); cur = ''
        else: cur += ch
    if cur.strip(): ops.append(cur.strip())
    return ops

def parse(text):
    parts = text.split(None, 1); mn = parts[0]; ops = split_ops(parts[1]) if len(parts) > 1 else []
    return mn, ops

def op_regs(op):
    return [PARENT[r] for r in re.findall(r'%(\w+)', op) if r in PARENT]

def slot_of(op):
    m = re.fullmatch(r'(-0x[0-9a-f]+)\(%ebp\)', op)
    return m.group(1) if m else None

def defs_uses(mn, ops):
    """Return (uses: list of operand descriptors, defs: list of registers/slots defined)."""
    base = re.sub(r'[bwlq]$', '', mn) if mn not in ('movzbl', 'movsbl', 'movzwl', 'movswl', 'cltd', 'call', 'sahf', 'leave', 'ret', 'idivl', 'idiv') else mn
    uses, defs = [], []
    if mn.startswith('j') or mn == 'ret' or mn == 'leave': return ops, []
    if mn == 'call': return ops, ['eax', 'ecx', 'edx']
    if mn == 'cltd': return ['%eax'], ['edx']
    if mn in ('idiv', 'idivl', 'div', 'divl'): return ops + ['%eax', '%edx'], ['eax', 'edx']
    if mn in ('mul', 'mull', 'imull', 'imul') and len(ops) == 1: return ops + ['%eax'], ['eax', 'edx']
    if mn.startswith('rep'): return ops + ['%esi', '%edi', '%ecx', '%eax'], ['esi', 'edi', 'ecx']
    if mn in ('fnstsw',): return [], ['eax']
    if mn == 'sahf': return ['%eax'], []
    if mn == 'push': return ops, []
    if mn == 'pop': return [], op_regs(ops[0]) if ops else []
    if mn.startswith('set'): return [], op_regs(ops[0])
    if mn.startswith('f'):  # x87: memory operands are uses (loads) or defs (stores)
        if mn.startswith('fst') or mn.startswith('fist') or mn.startswith('fnst'):
            return [o for o in ops if not slot_of(o)], [slot_of(o) for o in ops if slot_of(o)]
        return ops, []
    if not ops: return [], []
    dest = ops[-1]; srcs = ops[:-1]
    if mn.startswith('cmp') or mn.startswith('test'): return ops, []
    if (mn.startswith('xor') or mn.startswith('sbb') or mn.startswith('sub')) and len(ops) == 2 and ops[0] == ops[1] and ops[0].startswith('%'): return [], op_regs(dest)
    if base in ('mov', 'lea') or mn in ('movzbl', 'movsbl', 'movzwl', 'movswl'):
        uses = srcs + ([dest] if not (dest.startswith('%') or slot_of(dest)) else [])
    else:  # arithmetic: dest is also a use
        uses = srcs + [dest]
    if dest.startswith('%'): defs = op_regs(dest)
    elif slot_of(dest): defs = [slot_of(dest)]
    else: uses = uses  # memory store elsewhere: address regs are uses (already included via dest)
    return uses, defs


def touched(text):
    mn, ops = parse(text); uses, defs = defs_uses(mn, ops)
    ur = set(); dr = set(defs); memstore = False; memload = False
    for u in uses:
        ur |= set(op_regs(u)); sl = slot_of(u)
        if sl: ur.add(sl)
        elif '(' in u or (not u.startswith('%') and not u.startswith('$')): memload = True
    for d in defs: dr.add(d)
    if ops:
        dest = ops[-1]
        if not dest.startswith('%') and not slot_of(dest) and not mn.startswith(('cmp','test','push','j')) and not mn.startswith('f') and mn != 'call': memstore = True
        if mn.startswith('fst') or mn.startswith('fist') or mn.startswith('fnst'):
            if any(not slot_of(o) for o in ops): memstore = True
        for d in ops[:-1] if mn.startswith(('cmp','test')) else []:
            pass
    if mn == 'call': memstore = True; memload = True
    if mn.startswith('rep'): memstore = True; memload = True
    return ur, dr, memload, memstore

def independent(a, b):
    if a.split()[0].startswith('f') and b.split()[0].startswith('f'): return False
    ua, da, la, sa = touched(a); ub, db, lb, sb = touched(b)
    if da & (ub | db) or db & ua: return False
    if (sa and (lb or sb)) or (sb and (la or sa)): return False
    return True

import copy
_NEXT = 0
def resync_line(o, c, ko, kc, eo, ec):
    """Advance both sides to the next line tag present ahead on both sides (structural realignment)."""
    lo = {}; lc = {}
    for k in range(ko, len(o)):
        ln = o[k]['line'][1] if o[k]['line'] else None
        if ln is not None and ln not in lo: lo[ln] = k
    for k in range(kc, len(c)):
        ln = c[k]['line'][1] if c[k]['line'] else None
        if ln is not None and ln not in lc: lc[ln] = k
    cur = o[ko]['line'][1] if ko < len(o) and o[ko]['line'] else -1
    common = [ln for ln in lo if ln in lc and ln > cur]
    if not common: return len(o), len(c), 0
    ln = min(common, key=lambda x: (lo[x] + lc[x]))
    skipped = 0
    for k in range(ko, lo[ln]):
        for d in defs_uses(*parse(o[k]['norm']))[1]: eo.v[d] = eo.fresh()
        skipped += 1
    for k in range(kc, lc[ln]):
        for d in defs_uses(*parse(c[k]['norm']))[1]: ec.v[d] = ec.fresh()
        skipped += 1
    return lo[ln], lc[ln], skipped

class Env:
    def __init__(self): self.v = {}; self.n = 0
    def clone(self):
        e = Env(); e.v = dict(self.v); e.n = self.n; return e
    def fresh(self):
        global _NEXT
        _NEXT += 1; return _NEXT
    def get(self, key): return self.v.get(key)

def operand_shape(op, env, side_ids):
    """Shape of an operand with value ids substituted for registers/slots; returns (shape, ids)."""
    s = slot_of(op)
    if s is not None:
        vid = env.get(s); return 'SLOT', [('slot', s, vid)]
    ids = []
    def repl(m):
        r = m.group(1)
        if r in PARENT and PARENT[r] not in ('ebp', 'esp'):
            ids.append(('reg', PARENT[r], env.get(PARENT[r])))
            return '%' + ('R' if r == PARENT[r] else 'r' + r[-1])  # keep sub-register kind
        return m.group(0)
    shape = re.sub(r'%(\w+)', repl, op)
    return shape, ids

def run(fn, obj, delta=None):
    if delta is None: delta = ltcompare.delta_for(obj, fn)
    o = ltcompare.annotate(ltcompare.original_listing(fn), False)
    c = ltcompare.annotate(ltcompare.candidate_listing(obj, fn, delta), True)
    def index(ins): return {i['off']: k for k, i in enumerate(ins)}
    io, ic = index(o), index(c)
    def tgt(i):
        m = re.search(r'<_?' + fn + r'(?:\+0x([0-9a-f]+))?>', i['text']); return (int(m.group(1), 16) if m.group(1) else 0) if m else None
    bind = {}      # oracle value id -> candidate value id
    rbind = {}
    def correspond(ids_o, ids_c, where):
        if len(ids_o) != len(ids_c): return 'operand count'
        for (ko, no, vo), (kc, nc, vc) in zip(ids_o, ids_c):
            if ko != kc: return 'operand kind'
            if vo is None and vc is None: continue
            if vo is None or vc is None: return 'undefined %s %s vs %s' % (ko, no, nc)
            if vo in bind and bind[vo] != vc: return 'value mismatch %s: oracle %s(%d) is candidate %s(%d)' % (ko, no, vo, nc, vc)
            if vc in rbind and rbind[vc] != vo: return 'value mismatch %s: candidate %s(%d) already bound' % (ko, nc, vc)
            bind[vo] = vc; rbind[vc] = vo
        return None
    visited = {}; queue = collections.deque([(0, 0, Env(), Env(), {'o': set(), 'c': set()})]); failures = []; steps = 0; matched = 0; reordered = 0; skipped = 0; narrowed = 0; swapcc = set()
    while queue:
        po, pc, eo, ec, consumed = queue.popleft()
        key = (po, pc)
        if key in visited: continue
        visited[key] = True
        ko, kc = io.get(po), ic.get(pc)
        while ko is not None and kc is not None and ko < len(o) and kc < len(c):
            if ko in consumed['o']: ko += 1; continue
            if kc in consumed['c']: kc += 1; continue
            i, j = o[ko], c[kc]
            if is_nop(i['norm']): ko += 1; continue
            if is_nop(j['norm']): kc += 1; continue
            if i['mn'] == 'jmp': t = tgt(i); ko = io.get(t); continue
            if j['mn'] == 'jmp': t = tgt(j); kc = ic.get(t); continue
            steps += 1
            if i['mn'].startswith('j') and j['mn'].startswith('j'):
                cco, ccc = i['mn'][1:], j['mn'][1:]
                to, fo = tgt(i), (o[ko + 1]['off'] if ko + 1 < len(o) else None)
                tc, fc = tgt(j), (c[kc + 1]['off'] if kc + 1 < len(c) else None)
                exp = SWAPCC.get(cco, cco) if (kc - 1) in swapcc else cco
                if exp == ccc: succ = [(to, tc), (fo, fc)]
                elif INVERT.get(exp) == ccc: succ = [(to, fc), (fo, tc)]
                else:
                    failures.append((i['line'], i['off'], j['off'], 'condition %s vs %s' % (cco, ccc))); succ = [(to, tc), (fo, fc)]
                for a, b in succ:
                    if a is not None and b is not None: queue.append((a, b, eo.clone(), ec.clone(), {'o': set(consumed['o']), 'c': set(consumed['c'])}))
                matched += 1; break
            if i['mn'].startswith('j') or j['mn'].startswith('j') or i['mn'] == 'ret' or j['mn'] == 'ret':
                if i['mn'] == 'ret' and j['mn'] == 'ret': matched += 1; break
                failures.append((i['line'], i['off'], j['off'], 'control mismatch: %s vs %s' % (i['norm'], j['norm'])))
                ko, kc, sk = resync_line(o, c, ko, kc, eo, ec); skipped += sk
                if ko >= len(o) or kc >= len(c): break
                continue
            mo, oo = parse(i['norm']); mc, occ = parse(j['norm'])
            # candidate register copies that the oracle lacks (or vice versa): propagate ids and skip
            def is_copy(mn, ops): return mn in ('mov', 'movl') and len(ops) == 2 and ops[0].startswith('%') and ops[1].startswith('%') and slot_of(ops[0]) is None
            if mo != mc or len(oo) != len(occ):
                if is_copy(mc, occ) and not is_copy(mo, oo):
                    ec.v[op_regs(occ[1])[0]] = ec.get(op_regs(occ[0])[0]) or ec.fresh(); kc += 1; continue
                if is_copy(mo, oo) and not is_copy(mc, occ):
                    eo.v[op_regs(oo[1])[0]] = eo.get(op_regs(oo[0])[0]) or eo.fresh(); ko += 1; continue
                # local reorder: look ahead on the candidate for the oracle op, or on the oracle for the candidate op
                found = None
                for side, stream, k0, other in (('c', c, kc, i), ('o', o, ko, j)):
                    for kk in range(k0 + 1, min(k0 + 8, len(stream))):
                        x = stream[kk]
                        if x['mn'].startswith('j') or x['mn'] == 'ret' or kk in consumed[side]: break
                        if is_nop(x['norm']): continue
                        mx, ox = parse(x['norm']); mt, ot = parse(other['norm'])
                        if mx == mt and len(ox) == len(ot) and all(independent(stream[q]['norm'], x['norm']) for q in range(k0, kk) if q not in consumed[side] and not is_nop(stream[q]['norm'])):
                            found = (side, kk); break
                    if found: break
                if found:
                    side, kk = found
                    consumed[side].add(kk); reordered += 1
                    # process the pair (i, stream[kk]) or (stream[kk], j) now, without advancing the other side
                    if side == 'c': pending = (ko, kk, True)
                    else: pending = (kk, kc, False)
                    ii, jj = o[pending[0]], c[pending[1]]
                    mo2, oo2 = parse(ii['norm']); mc2, occ2 = parse(jj['norm'])
                    uo2, do2 = defs_uses(mo2, oo2); uc2, dc2 = defs_uses(mc2, occ2)
                    ids_o2, ids_c2 = [], []
                    for a in uo2: ids_o2 += operand_shape(a, eo, 'o')[1]
                    for b in uc2: ids_c2 += operand_shape(b, ec, 'c')[1]
                    err = correspond(ids_o2, ids_c2, ii['line'])
                    if err: failures.append((ii['line'], ii['off'], jj['off'], 'reordered pair: ' + err)); break
                    for a, b in zip(do2, dc2):
                        vo, vc = eo.fresh(), ec.fresh(); eo.v[a] = vo; ec.v[b] = vc; bind[vo] = vc; rbind[vc] = vo
                    matched += 1
                    if side == 'c': ko += 1
                    else: kc += 1
                    continue
                failures.append((i['line'], i['off'], j['off'], 'op mismatch: %s | %s' % (i['norm'], j['norm'])))
                # resynchronise at the next control transfer on both sides; values written meanwhile become unknown
                ko, kc, sk = resync_line(o, c, ko, kc, eo, ec); skipped += sk
                continue
            uo, do = defs_uses(mo, oo); uc, dc = defs_uses(mc, occ)
            if mo == 'push' and mc == 'push' and ko > 0 and kc > 0 and o[ko-1]['mn'] == 'call' and c[kc-1]['mn'] == 'call':
                matched += 1; ko += 1; kc += 1; continue
            ids_o, ids_c, shapes_o, shapes_c = [], [], [], []
            for a in uo:
                s, ids = operand_shape(a, eo, 'o'); shapes_o.append(s); ids_o += ids
            for b in uc:
                s, ids = operand_shape(b, ec, 'c'); shapes_c.append(s); ids_c += ids
            # destination shapes (non-value part) must agree too
            sd_o = [operand_shape(a, eo, 'o')[0] for a in oo]; sd_c = [operand_shape(b, ec, 'c')[0] for b in occ]
            if mo.startswith('mov') and mc.startswith('mov') and len(oo) == 2 and len(occ) == 2 and oo[0] == occ[0] and oo[0].startswith('$') and oo[1].startswith('%') and occ[1].startswith('%') and PARENT.get(oo[1][1:]) == PARENT.get(occ[1][1:]) and oo[1] != occ[1]:
                narrowed += 1
                for a, b in zip(op_regs(oo[1]), op_regs(occ[1])):
                    vo, vc = eo.fresh(), ec.fresh(); eo.v[a] = vo; ec.v[b] = vc; bind[vo] = vc; rbind[vc] = vo
                matched += 1; ko += 1; kc += 1; continue
            if mo.startswith('cmp') and mc.startswith('cmp') and len(oo) == 2 and oo == occ[::-1]:
                swapcc.add(kc); matched += 1; ko += 1; kc += 1; continue
            if shapes_o != shapes_c or sd_o != sd_c:
                found = None
                for side, stream, k0, other in (('c', c, kc, i), ('o', o, ko, j)):
                    for kk in range(k0 + 1, min(k0 + 16, len(stream))):
                        x = stream[kk]
                        if x['mn'].startswith('j') or x['mn'] == 'ret' or kk in consumed[side]: break
                        if is_nop(x['norm']): continue
                        if x['norm'] == other['norm'] and all(independent(stream[q]['norm'], x['norm']) for q in range(k0, kk) if q not in consumed[side] and not is_nop(stream[q]['norm'])):
                            found = (side, kk); break
                    if found: break
                if found:
                    side, kk = found; consumed[side].add(kk); reordered += 1
                    ii, jj = (o[ko], c[kk]) if side == 'c' else (o[kk], c[kc])
                    uo2, do2 = defs_uses(*parse(ii['norm'])); uc2, dc2 = defs_uses(*parse(jj['norm']))
                    ids_o2, ids_c2 = [], []
                    for a in uo2: ids_o2 += operand_shape(a, eo, 'o')[1]
                    for b in uc2: ids_c2 += operand_shape(b, ec, 'c')[1]
                    err = correspond(ids_o2, ids_c2, ii['line'])
                    if not err:
                        for a, b in zip(do2, dc2):
                            vo, vc = eo.fresh(), ec.fresh(); eo.v[a] = vo; ec.v[b] = vc; bind[vo] = vc; rbind[vc] = vo
                        matched += 1
                        if side == 'c': ko += 1
                        else: kc += 1
                        continue
                failures.append((i['line'], i['off'], j['off'], 'shape mismatch: %s | %s' % (i['norm'], j['norm']))); 
                ko, kc, sk = resync_line(o, c, ko, kc, eo, ec); skipped += sk
                continue
            err = correspond(ids_o, ids_c, i['line'])
            if err:
                failures.append((i['line'], i['off'], j['off'], err + ': %s | %s' % (i['norm'], j['norm']))); 
                ko, kc, sk = resync_line(o, c, ko, kc, eo, ec); skipped += sk
                continue
            # copies propagate ids, other definitions create fresh bound ids
            if is_copy(mo, oo):
                eo.v[op_regs(oo[1])[0]] = eo.get(op_regs(oo[0])[0]) or eo.fresh()
                ec.v[op_regs(occ[1])[0]] = ec.get(op_regs(occ[0])[0]) or ec.fresh()
                vo, vc = eo.v[op_regs(oo[1])[0]], ec.v[op_regs(occ[1])[0]]
                if vo in bind and bind[vo] != vc: failures.append((i['line'], i['off'], j['off'], 'copy binding conflict')); break
                bind[vo] = vc; rbind[vc] = vo
            else:
                if len(do) != len(dc): failures.append((i['line'], i['off'], j['off'], 'def count')); break
                for a, b in zip(do, dc):
                    vo, vc = eo.fresh(), ec.fresh(); eo.v[a] = vo; ec.v[b] = vc; bind[vo] = vc; rbind[vc] = vo
            matched += 1; ko += 1; kc += 1
    return steps, matched, failures, len(visited), reordered, skipped, o, c, io, ic, narrowed

if __name__ == '__main__':
    fn, obj = sys.argv[1], sys.argv[2]; delta = int(sys.argv[3]) if len(sys.argv) > 3 else None
    steps, matched, failures, states, reordered, skipped, o, c, io, ic, narrowed = run(fn, obj, delta)
    print('%s: lockstep states %d, pairs compared %d, matched %d (reordered %d), divergences %d, unverified instructions skipped %d, narrowed immediates %d' % (fn, states, steps, matched, reordered, len(failures), skipped, narrowed))
    seen = set()
    for ln, po, pc, msg in failures:
        k = (ln[1] if ln else None, msg[:40])
        if k in seen: continue
        seen.add(k)
        print('  line %s oracle@%x cand@%x: %s' % (ln[1] if ln else '?', po, pc, msg[:150]))
        ko, kc = io.get(po), ic.get(pc)
        if ko is not None and kc is not None:
            print('      oracle: ' + ' ; '.join(x['norm'] for x in o[ko:ko+4]))
            print('      cand  : ' + ' ; '.join(x['norm'] for x in c[kc:kc+4]))
