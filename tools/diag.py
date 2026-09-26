"""Experimental compiler-archaeology diagnostics (search guidance only, never a gate).

Answers: which basic blocks of a candidate differ from the original, in what way
(CFG / layout / register allocation / stack slots / x87 / late local codegen / true
mismatch), which source lines the first causal divergence belongs to, and -- with
--why -- what the locked GCC 4.4.1 recorded about that decision (tree branch
predictions, IRA assignments, bb-reorder traces).

The complete historical TU is always compiled in its real context with the locked
toolchain and flags; objects and dumps go to ignored build/diag/.  Machine-code
equality remains the only authority.

  python tools/diag.py my_strcmp
  python tools/diag.py my_strcmp --body candidates/x.c --why
  python tools/diag.py replay_selector --patch candidates/p.json --blocks
  python tools/diag.py --triage [FUNC[=body.c|patch.json] ...]

Triage buckets: 1 source/structure (unpaired content, CFG shape), 2 context/frequency
(register/stack-slot/x87/late-local differences inside paired blocks), 3 layout tie
(same blocks and CFG, different placement or epilogue copying), 4 unexplained (most
blocks unpaired).  A bucket routes hypotheses; it never grants or implies a match.
"""
import argparse, bisect, collections, difflib, hashlib, re, subprocess
from pathlib import Path
from common import ROOT, BUILD, ORACLE, ANALYSIS, TC, environment, read_json
from build import targets, flags_for
from scratch import resolve_function, overlay, patched_source

DIAG = BUILD / 'diag'
DUMP_FLAGS = ('-fdump-tree-profile-details-lineno', '-fdump-tree-optimized-blocks-lineno', '-fdump-rtl-ira', '-fira-verbose=5',
              '-fdump-rtl-bbro-details', '-fdump-rtl-stack', '-fdump-rtl-peephole2')
PAD = re.compile(r'^(nop|xchg\s+%ax,%ax|lea\s+0x0\(%[a-z]+(,%[a-z]+,1)?\),%[a-z]+|lea\s+0x0\(%[a-z]+,%eiz,1\),%[a-z]+)$')
CC = {'e': 'eq', 'ne': 'eq', 'z': 'eq', 'nz': 'eq', 'l': 'lt', 'ge': 'lt', 'g': 'gt', 'le': 'gt',
      'a': 'ab', 'be': 'ab', 'b': 'bl', 'ae': 'bl', 'jae': 'bl', 's': 'sg', 'ns': 'sg', 'p': 'pa', 'np': 'pa',
      'o': 'ov', 'no': 'ov', 'nb': 'bl', 'nae': 'bl', 'c': 'bl', 'nc': 'bl'}
REG = re.compile(r'%(e?[abcd]x|[abcd][lh]|e?[sd]i)\b')
STACK = re.compile(r'-?0x[0-9a-f]+(?=\(%e[bs]p\))')
X87 = re.compile(r'^f')
EPI = re.compile(r'^(add \$0x[0-9a-f]+,%esp|pop %e[a-z]+|leave|lea -?0x[0-9a-f]+\(%ebp\),%esp)$')


# ---------------------------------------------------------------- compilation
def source_text(target, config, name, body=None, patch=None):
    if patch: return patched_source(Path(patch), target).decode('cp1252')
    src = (ROOT / config['source']).read_bytes()
    if body: src = overlay(src, name, Path(body).read_bytes())
    return src.decode('cp1252')


def compile_tu(config, text, dumps=False):
    """Compile the whole TU with the locked flags; cached by content."""
    extra = DUMP_FLAGS if dumps else ()
    key = hashlib.sha256(text.encode('cp1252') + repr((extra, config)).encode()).hexdigest()[:20]
    d = DIAG / key; d.mkdir(parents=True, exist_ok=True)
    obj = d / 'u.o'
    if not obj.exists():
        src = d / Path(config['source']).name
        src.write_bytes(text.encode('cp1252'))
        flags = [x if not x.startswith('-I') else '-I' + str(ROOT / x[2:]) for x in flags_for(config)]
        r = subprocess.run([str(TC / 'bin/gcc.exe'), *flags, *extra, '-c', str(src), '-o', str(obj)],
                           cwd=str(d), env=environment(), capture_output=True)
        if r.returncode: raise SystemExit(r.stderr.decode(errors='replace')[-3000:])
    return obj, d


# ---------------------------------------------------------------- disassembly
def _callee(sym):
    return re.sub(r'@\d+$', '', sym).lstrip('_')


def original_rows(f):
    out = subprocess.run([str(ANALYSIS), '-d', '-w', '--start-address=%d' % f['va'],
                          '--stop-address=%d' % (f['va'] + f['size']), str(ORACLE)],
                         capture_output=True, text=True).stdout
    rows = []
    for l in out.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s)+)\s*(.*)', l)
        if m: rows.append({'off': int(m[1], 16) - f['va'], 'asm': m[3].strip(), 'reloc': [], 'va': int(m[1], 16)})
    return rows


def candidate_rows(obj, name):
    out = subprocess.run([str(ANALYSIS), '-d', '-w', '-r', str(obj)], capture_output=True, text=True,
                         env=environment()).stdout
    rows, on, base = [], False, None
    for l in out.splitlines():
        m = re.match(r'^([0-9a-f]+) <(.+)>:', l)
        if m:
            on = m[2] == '_' + name
            if on: base = int(m[1], 16)
            continue
        if not on: continue
        m = re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s)+)\s*(.*)', l)
        if m:
            asm, rel, at = m[3], [], int(m[1], 16)
            row = {'off': at - base, 'asm': '', 'reloc': rel, 'len': len(m[2].split()), 'rpos': []}
            for r in re.finditer(r'	([0-9a-f]+): (\S+)	(\S+)', asm):
                rel.append(r[3]); row['rpos'].append(int(r[1], 16) - at)
            row['asm'] = re.split(r'	[0-9a-f]+: \S+	', asm)[0].strip()
            rows.append(row); continue
        m = re.match(r'^\s*([0-9a-f]+):\s+(\S+)\s+(\S+)', l)
        if m and rows and m[2].startswith(('DISP32', 'dir32', 'R_')):
            rows[-1]['reloc'].append(m[3]); rows[-1]['rpos'].append(int(m[1], 16) - base - rows[-1]['off'])
    return rows


def normalize(row, fname):
    """Placement-independent text: absolute addresses -> A, intra-function targets -> @off, callees by name."""
    asm = re.sub(r'\s+', ' ', row['asm'])
    mn, _, ops = asm.partition(' ')
    target = None
    if mn.startswith('j') or mn == 'call':
        m = re.match(r'^([0-9a-f]+) <(.+?)(\+0x[0-9a-f]+)?>$', ops)
        if m:
            sym, off = m[2], int(m[3][3:], 16) if m[3] else 0
            if row['reloc']: return mn + ' ' + _callee(row['reloc'][0]), None
            if mn == 'call': return 'call ' + _callee(sym), None
            if sym == '_' + fname or sym == fname: target = off
            else: return mn + ' ' + _callee(sym), None
            return mn, target
        if ops.startswith('*'):
            return mn + ' *' + _mask_abs(ops[1:], row, True), None
    return mn + (' ' + _mask_abs(ops, row, False) if ops else ''), None


def _mask_abs(ops, row, force):
    # absolute memory operands (no base register) are image addresses -> A
    ops = re.sub(r'(?<![\w$])0x[0-9a-f]+(?=\(,)', 'A', ops)
    ops = re.sub(r'(?<![\w$(])0x[0-9a-f]+(?![\w(])', 'A', ops)
    image = lambda m, text: text if 0x400000 <= int(m[1], 16) < 0x600000 else m[0]
    if 'va' in row:   # original: image addresses as immediates or as displacements over a base register
        ops = re.sub(r'\$0x([0-9a-f]+)', lambda m: image(m, '$A'), ops)
        ops = re.sub(r'(?<![\w$])0x([0-9a-f]+)(?=\(%)', lambda m: image(m, 'A'), ops)
    elif row['reloc'] and 'A' not in ops:
        # candidate: a relocation in the last 4 bytes of an insn with an immediate patches the immediate,
        # otherwise it patches the memory displacement
        for pos in row.get('rpos') or [None]:
            imm = '$0x' in ops and (pos is None or pos + 4 == row.get('len'))
            ops = re.sub(r'\$0x[0-9a-f]+', '$A', ops, count=1) if imm else \
                re.sub(r'(?<![\w$])-?0x[0-9a-f]+(?=\(%)', 'A', ops, count=1)
    return ops


# ---------------------------------------------------------------- basic blocks
class Block:
    def __init__(self, i, rows):
        self.id, self.rows = i, rows
        self.start = rows[0]['off']
        self.body, self.term, self.targets = [], None, []
        for r in rows:
            if PAD.match(r['norm']): continue
            mn = r['norm'].split(' ')[0]
            if r['tgt'] is not None and mn.startswith('j'):
                self.term, self.targets = mn, [r['tgt']]
            elif mn == 'ret' or mn.startswith('jmp'):
                self.term = mn
                if mn != 'ret': self.body.append(r['norm'])
            else:
                self.body.append(r['norm'])
        self.fall = None
        self.ext, self.absorbed, self.preds = None, None, []

    def is_tail(self):
        return self.term == 'ret' or (self.term == 'jmp' and not self.targets and bool(self.body))

    def key(self, tier):
        b = self.ext if self.ext is not None else self.body
        if tier >= 1: b = [REG.sub('%r', x) for x in b]
        if tier >= 2: b = [STACK.sub('S', x) for x in b]
        if tier >= 3: b = [x87(x) for x in b if not x.startswith('fxch')]
        t = 'J' + CC.get(self.term[1:], self.term) if self.term and self.term.startswith('j') and self.term != 'jmp' else ''
        return tuple(b) + ((t,) if t else ())

    def core(self):
        """Own body without epilogue rows and without an absorbed or shared return tail."""
        return tuple(x87(STACK.sub('S', REG.sub('%r', x))) for x in self.body if not EPI.match(x))


def x87(x):
    if not X87.match(x): return x
    x = re.sub(r'%st\(\d\)', '%st', x)
    return re.sub(r'^f(div|sub)r?(p?)', r'f\1X\2', x)


def blocks(rows, fname):
    for r in rows: r['norm'], r['tgt'] = normalize(r, fname)
    offs = [r['off'] for r in rows]
    leaders = {0}
    for i, r in enumerate(rows):
        mn = r['norm'].split(' ')[0]
        if r['tgt'] is not None: leaders.add(r['tgt'])
        if (mn.startswith('j') or mn == 'ret') and i + 1 < len(rows): leaders.add(rows[i + 1]['off'])
    starts = sorted(x for x in leaders if x in set(offs))
    out, cur, bi = [], [], 0
    for r in rows:
        if r['off'] in starts and cur:
            out.append(Block(len(out), cur)); cur = []
        cur.append(r)
    if cur: out.append(Block(len(out), cur))
    by_start = {b.start: b for b in out}
    for i, b in enumerate(out):
        b.targets = [by_start[t].id for t in b.targets if t in by_start]
        if b.term not in ('jmp', 'ret') and not (b.term or '').startswith('jmp') and i + 1 < len(out):
            b.fall = out[i + 1].id
        elif b.term and b.term.startswith('jmp') and b.rows[-1]['tgt'] is not None:
            b.targets = [by_start[b.rows[-1]['tgt']].id] if b.rows[-1]['tgt'] in by_start else []
    # Return tails are compared as part of every block that flows unconditionally into
    # them: epilogue duplication vs sharing is then a layout fact, not a content fact.
    for b in out:
        for t in b.targets: out[resolve(out, t)].preds.append((b.id, 'jump'))
        if b.fall is not None: out[resolve(out, b.fall)].preds.append((b.id, 'fall'))
    for b in out:
        if b.is_tail() or not b.body: continue
        if b.term and b.term.startswith('j') and b.term != 'jmp': continue
        nxt = resolve(out, b.targets[0]) if b.term == 'jmp' and b.targets else (resolve(out, b.fall) if b.fall is not None else None)
        if nxt is not None and out[nxt].is_tail():
            b.ext, b.absorbed = b.body + out[nxt].body, nxt
    return out


def pairable(bs, b):
    if not b.body: return False
    if b.is_tail() and b.preds:
        return any(bs[p].absorbed != b.id for p, _ in b.preds)
    return True


def resolve(bs, i, seen=()):
    """Follow empty forwarding blocks (pure jmp / padding)."""
    b = bs[i]
    if b.body or i in seen: return i
    nxt = b.targets[0] if b.term == 'jmp' and b.targets else b.fall
    return i if nxt is None else resolve(bs, nxt, seen + (i,))


def succ(bs, b):
    s = [resolve(bs, t) for t in b.targets]
    if b.fall is not None: s.append(resolve(bs, b.fall))
    return {x if x != b.absorbed else 'exit' for x in s}


TIERS = ['exact', 'registers', 'stack-slots', 'x87', 'near(late/local)', 'epilogue-dup']


def match(ob, cb):
    """Pair non-empty blocks placement-independently, tier by tier."""
    pairs, ofree, cfree = {}, [b for b in ob if pairable(ob, b)], [b for b in cb if pairable(cb, b)]
    for tier in range(4):
        idx = collections.defaultdict(list)
        for b in cfree: idx[b.key(tier)].append(b)
        rest = []
        for b in ofree:
            cand = idx.get(b.key(tier))
            if cand:
                c = min(cand, key=lambda c: abs(c.start / max(1, cb[-1].start) - b.start / max(1, ob[-1].start)))
                cand.remove(c); pairs[b.id] = (c.id, tier)
            else: rest.append(b)
        ofree = rest; cfree = [c for c in cfree if c.id not in {p for p, _ in pairs.values()}]
    idx = collections.defaultdict(list)  # epilogue copied vs shared: same own body, different return tail
    for c in cfree: idx[c.core()].append(c)
    for b in list(ofree):
        cand = idx.get(b.core()) if b.core() else None
        if cand:
            c = cand.pop(0); pairs[b.id] = (c.id, 5); cfree.remove(c); ofree.remove(b)
    for b in list(ofree):  # near: same mnemonic sequence similarity
        best, br = None, 0.0
        for c in cfree:
            r = difflib.SequenceMatcher(None, [x.split(' ')[0] for x in b.key(3)], [x.split(' ')[0] for x in c.key(3)]).ratio()
            if r > br: best, br = c, r
        if best is not None and br >= 0.75:
            pairs[b.id] = (best.id, 4); cfree.remove(best); ofree.remove(b)
    refine(ob, cb, pairs)
    return pairs, ofree, cfree


def refine(ob, cb, pairs):
    """Identical-content blocks are ambiguous; swap partners while CFG agreement improves."""
    def agree(o):
        if o not in pairs: return 0
        c = cb[pairs[o][0]]
        want = {x if x == 'exit' else (pairs[x][0] if x in pairs else None) for x in succ(ob, ob[o])}
        return len(want & succ(cb, c))
    groups = collections.defaultdict(list)
    for o, (c, tier) in pairs.items(): groups[(tier, ob[o].key(tier))].append(o)
    changed = True
    while changed:
        changed = False
        for g in groups.values():
            for i in range(len(g)):
                for j in range(i + 1, len(g)):
                    x, y = g[i], g[j]
                    near = {x, y} | {p for p, _ in ob[x].preds} | {p for p, _ in ob[y].preds}
                    before = sum(agree(o) for o in near)
                    pairs[x], pairs[y] = pairs[y], pairs[x]
                    if sum(agree(o) for o in near) > before: changed = True
                    else: pairs[x], pairs[y] = pairs[y], pairs[x]


# ---------------------------------------------------------------- report
def source_lines(f, config):
    from evidence import Evidence
    db = Evidence()
    rows = [r for r in db.lines(f['va'], f['va'] + f['size'])
            if Path(r['file']).name.lower() == Path(config['source']).name.lower()]
    db.close()
    return sorted((r['address'] - f['va'], r['line']) for r in rows)


def lines_of(block, ltab):
    offs = [a for a, _ in ltab]; out = set()
    for r in block.rows:
        i = bisect.bisect_right(offs, r['off']) - 1
        if i >= 0: out.add(ltab[i][1])
    return sorted(out)


def epi_region(ob, b, pairs):
    """b's successors differ only because a return tail was copied on one side and shared on the other."""
    tails = {x.id for x in ob if x.is_tail()} | {o for o, (_, t) in pairs.items() if t == 5}
    if b.id in tails:
        return b.id in pairs
    return any(x == 'exit' or x in tails for x in succ(ob, b))


def mnemonics(b):
    return [x.split(' ')[0] for x in b.key(3)]


def classify(orig_rows, cand_rows, fname):
    """Placement-independent block comparison of two instruction row lists."""
    ob, cb = blocks(orig_rows, fname), blocks(cand_rows, fname)
    pairs, ofree, cfree = match(ob, cb)
    inv = {c: o for o, (c, _) in pairs.items()}
    issues = []
    for b in ob:
        if not pairable(ob, b): continue
        if b.id not in pairs:
            issues.append((b, 'true', None)); continue
        c, tier = pairs[b.id]
        cand = cb[c]
        cs_ = succ(cb, cand)
        mapped = {x if x == 'exit' else (pairs[x][0] if x in pairs else ('o?', x)) for x in succ(ob, b)}
        cfg_ok = mapped == cs_
        fall_o = resolve(ob, b.fall) if b.fall is not None and b.absorbed is None else None
        fall_c = resolve(cb, cand.fall) if cand.fall is not None and cand.absorbed is None else None
        fall_ok = (pairs.get(fall_o, (None,))[0] == fall_c) if fall_o is not None else fall_c is None
        if tier == 5:
            issues.append((b, 'layout(epilogue-dup)', cand)); continue
        if tier == 4 and mnemonics(b) == mnemonics(cand):
            issues.append((b, 'operands', cand))   # same instructions, different constants/fields/operands
        elif tier: issues.append((b, TIERS[tier], cand))
        if not cfg_ok and epi_region(ob, b, pairs):
            issues.append((b, 'layout(epilogue-dup)', cand))
        elif not cfg_ok:
            unpaired_o = {x for x in succ(ob, b) if x != 'exit' and x not in pairs}
            unpaired_c = {x for x in cs_ if x != 'exit' and x not in inv}
            issues.append((b, 'cfg-edges(consequence)' if (unpaired_o or unpaired_c) else 'cfg-edges', cand))
        elif not fall_ok: issues.append((b, 'layout', cand))
    return dict(ob=ob, cb=cb, pairs=pairs, ofree=ofree, cfree=cfree, issues=issues, ltab=[])


def analyse(name, target=None, body=None, patch=None):
    target, config, f = resolve_function(name, target)
    text = source_text(target, config, f['name'], body, patch)
    obj, _ = compile_tu(config, text)
    a = classify(original_rows(f), candidate_rows(obj, f['name']), f['name'])
    a.update(target=target, config=config, f=f, text=text, ltab=source_lines(f, config), obj=obj)
    return a


FAMILIES = {
    'true': ['source statement content', 'missing/extra statement', 'expression spelling'],
    'cfg-edges': ['if/else inversion', 'branch nesting', '&&/|| grouping', 'ternary vs branch',
                  'loop form', 'redundant test removed later (threading)'],
    'cfg-edges(consequence)': ['fix the unpaired block first'],
    'layout(epilogue-dup)': ['frequency of the edge into return (>= 10% of entry copies the epilogue)',
                             'early return vs join block', 'prediction of the preceding test (orientation, heuristics)',
                             'threaded-away extra tests'],
    'layout': ['if/else inversion', 'branch orientation', 'early return vs join block',
               'guarded call placement (call heuristic)', 'case order / empty case', 'independent if vs else-if'],
    'order': ['bb-reorder trace keys (block frequency ties)', 'connect_traces tie order'],
    'registers': ['declaration order', 'local scope/lifetime', 'initializer placement', 'missing local',
                  'independent if vs else-if (reference frequency)', 'temporary vs direct expression'],
    'stack-slots': ['declaration order', 'reference frequency of spilled locals', 'local scope/lifetime',
                    'missing local', 'dead store / extra test changing IRA frequency'],
    'operands': ['constant or field', 'wrong variable/global', 'argument value', 'expression operand'],
    'x87': ['expression association/order', 'temporary vs direct expression', 'constant reuse across statements'],
    'near(late/local)': ['neighbouring function bodies (peephole2 state)', 'expression spelling',
                         'comparison form'],
}
ORDER = ['true', 'operands', 'cfg-edges', 'x87', 'near(late/local)', 'stack-slots', 'registers', 'layout', 'layout(epilogue-dup)',
         'cfg-edges(consequence)']
# Causal precedence: content/structure first, then allocation, then pure placement.
GROUPS = [('true', 'operands', 'cfg-edges', 'x87', 'near(late/local)'), ('stack-slots', 'registers'),
          ('layout', 'layout(epilogue-dup)', 'cfg-edges(consequence)')]
# The four triage buckets: 1 source/structural, 2 compiler context/frequency, 3 late layout tie, 4 unexplained.
BUCKET = {'true': 1, 'operands': 1, 'cfg-edges': 1, 'cfg-edges(consequence)': 1, 'x87': 2, 'near(late/local)': 2,
          'stack-slots': 2, 'registers': 2, 'layout': 3, 'layout(epilogue-dup)': 3, 'order': 3}
BUCKET_NAME = {0: 'exact', 1: 'source/structure', 2: 'context/frequency', 3: 'layout tie', 4: 'unexplained'}


def summarize(a):
    """Deterministic facts about one comparison; report() and --triage print from this."""
    ob, cb, pairs, issues, ltab = a['ob'], a['cb'], a['pairs'], a['issues'], a['ltab']
    lo_line = ltab[0][1] if ltab else 0   # prologue line; lower lines are inlined helpers
    def own(x): return min([l for l in lines_of(x[0], ltab) if l >= lo_line] or [10 ** 9])
    ne_o = sum(1 for b in ob if pairable(ob, b)); ne_c = sum(1 for b in cb if pairable(cb, b))
    fo = [b.id for b in ob if pairable(ob, b) and b.id in pairs]
    s = dict(blocks=(ne_o, ne_c), paired=len(pairs),
             tiers=collections.Counter(TIERS[t] for _, t in pairs.values()),
             kinds=collections.Counter(k for _, k, _ in issues),
             tails=(sum(1 for b in ob if b.is_tail()), sum(1 for b in cb if b.is_tail())),
             displaced=displaced_blocks(fo, [pairs[i][0] for i in fo]), earliest=[], first=None)
    for k in ORDER:
        xs = sorted((x for x in issues if x[1] == k), key=lambda x: (own(x), x[0].id))
        if xs: s['earliest'].append((k, xs[0], own(xs[0]), len(xs)))
    for group in GROUPS:
        xs = [x for x in issues if x[1] in group]
        if xs:
            s['first'] = min(xs, key=lambda x: (own(x), ORDER.index(x[1]), x[0].id)); s['first_line'] = own(s['first'])
            break
    if s['first']:
        s['bucket'] = BUCKET[s['first'][1]]
    elif s['displaced'] or s['tails'][0] != s['tails'][1]:
        s['bucket'] = 3
    else:
        s['bucket'] = 0
    # Pairing breaks down when most blocks have no counterpart: the block model explains little.
    if ne_o and len(pairs) < ne_o / 2: s['bucket'] = 4
    return s


def report(a, show_blocks=False):
    ob, cb, pairs, issues = a['ob'], a['cb'], a['pairs'], a['issues']
    s = summarize(a)
    print('function: %s  (%s)' % (a['f']['name'], a['target']))
    if s['tails'][0] != s['tails'][1]:
        print('return tails: original %d, candidate %d  (epilogue duplication differs: bb-reorder tail copy)' % s['tails'])
    print('blocks: original %d, candidate %d, paired %d' % (*s['blocks'], s['paired']))
    print('pairing tiers: ' + ', '.join('%s %d' % (k, s['tiers'][k]) for k in TIERS if s['tiers'][k]))
    print('issues: ' + (', '.join('%s %d' % (k, s['kinds'][k]) for k in ORDER if s['kinds'][k]) or 'none'))
    if a['cfree']:
        print('unpaired candidate blocks: ' + ', '.join('c%d@%x' % (c.id, c.start) for c in a['cfree']))
    print('physical order of paired blocks agrees: %s' % (not s['displaced']))
    if s['displaced']:
        print('trace-placement: %d block(s) outside the longest order-preserving run:' % len(s['displaced']))
        for o in s['displaced'][:12]:
            print('  o%-3d @%-5x lines %s  (candidate c%d@%x)' % (o, ob[o].start, lines_of(ob[o], a['ltab'])[:5],
                                                              pairs[o][0], cb[pairs[o][0]].start))
    print('triage bucket: %d %s' % (s['bucket'], BUCKET_NAME[s['bucket']]))
    if not issues:
        print('verdict: block sets, CFG and fallthroughs agree' +
              ('; only trace order differs (bb-reorder connect_traces / cold placement)' if s['displaced'] else ''))
        return ('order', s['displaced']) if s['displaced'] else None
    print('\nearliest block per class (source lines inside the function):')
    for k, x, line, n in s['earliest']:
        print('  %-24s o%-3d @%-5x line %s   (%d blocks)' % (k, x[0].id, x[0].start, line, n))
    b, kind, c = s['first']
    print('\nfirst likely causal divergence (earliest source line of the most basic class present):')
    print('  original block o%d @%x  lines %s  -> %s' % (b.id, b.start, lines_of(b, a['ltab']), kind))
    if c is not None: print('  candidate block c%d @%x' % (c.id, c.start))
    detail(b, c, kind, ob, cb, pairs)
    print('  hypothesis families: ' + '; '.join(FAMILIES[kind]))
    if show_blocks:
        print('\nall issues:')
        for b, k, c in sorted(issues, key=lambda x: (x[0].start, ORDER.index(x[1]))):
            print('  o%-3d @%-5x lines %-18s %-16s %s' % (b.id, b.start, lines_of(b, a['ltab'])[:4], k,
                                                     'c%d@%x' % (c.id, c.start) if c else '-'))
    return s['first']


def displaced_blocks(order, positions):
    """Original block ids not in a longest increasing subsequence of candidate positions."""
    n = len(positions)
    if not n: return []
    best, prev = [1] * n, [-1] * n
    for i in range(n):
        for j in range(i):
            if positions[j] < positions[i] and best[j] + 1 > best[i]: best[i], prev[i] = best[j] + 1, j
    k = max(range(n), key=lambda i: best[i]); keep = set()
    while k >= 0: keep.add(k); k = prev[k]
    return [order[i] for i in range(n) if i not in keep]


def detail(b, c, kind, ob, cb, pairs):
    if c is None:
        for r in b.rows: print('    O %5x %s' % (r['off'], r['asm'][:70]))
        return
    if kind.startswith(('layout', 'cfg-edges')):
        def desc(bs, x):
            s = sorted(succ(bs, x)); f = resolve(bs, x.fall) if x.fall is not None else None
            return 'term %s succ %s fall %s' % (x.term, s, f)
        inv = {cc: o for o, (cc, _) in pairs.items()}
        print('    original : ' + desc(ob, b))
        print('    candidate: ' + desc(cb, c) + '  (as original ids: %s, fall %s)' % (
            sorted((str(inv.get(x, '?%d' % x)) for x in succ(cb, c))),
            inv.get(resolve(cb, c.fall), '?') if c.fall is not None else None))
        return
    A, B = [r['asm'] for r in b.rows], [r['asm'] for r in c.rows]
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, A, B, autojunk=False).get_opcodes():
        if tag == 'equal': continue
        for k in range(i1, i2): print('    O %s' % A[k][:70])
        for k in range(j1, j2): print('    C %s' % B[k][:70])


# ---------------------------------------------------------------- compiler evidence (--why)
def why(a, first):
    if first[0] == 'order':
        o = first[1][0]
        b, kind, c = a['ob'][o], 'order', a['cb'][a['pairs'][o][0]]
    else:
        b, kind, c = first
    lines = lines_of(b, a['ltab'])
    lo, hi = min(lines), max(lines)
    print('\ncompiler evidence for original lines %d..%d (candidate compiled with dumps):' % (lo, hi))
    cand_lines = candidate_line_map(a, c) if c is not None else []
    print('  candidate block lines: %s' % cand_lines)
    obj, d = compile_tu(a['config'], a['text'], dumps=True)
    fn = a['f']['name']
    want = set(cand_lines) or set(range(lo, hi + 1))
    prof = section(d, 'profile', fn)
    if prof and kind in ('layout(epilogue-dup)', 'layout', 'cfg-edges', 'true', 'registers', 'stack-slots', 'order'):
        print('  -- tree branch predictions (037t.profile) for blocks at those lines --')
        for bb, text in prediction_blocks(prof, want):
            print('    bb %s:' % bb)
            for l in text: print('      ' + l)
    opt = section(d, 'optimized', fn)
    if opt:
        print('  -- final tree blocks at those lines (123t.optimized: frequency, successors) --')
        for l in tree_blocks(opt, want): print('    ' + l)
    if opt and kind == 'layout(epilogue-dup)':
        print('  -- edges into the return block (bb-reorder copies it when edge freq >= 1000, 10% of entry) --')
        for l in return_edges(opt): print('    ' + l)
    ira = section(d, 'ira', fn)
    if ira and kind in ('registers', 'stack-slots', 'layout'):
        print('  -- IRA (172r.ira): pseudos referenced at those lines --')
        for l in ira_summary(ira, want): print('    ' + l)
    bbro = section(d, 'bbro', fn)
    if bbro and kind in ('layout', 'order'):
        mine = rtl_bbs(bbro, want)
        print('  -- bb-reorder (187r.bbro): RTL blocks at those lines %s; trace log (* = those blocks) --' % sorted(mine))
        log = [x for x in bbro.split('\n') if x.startswith(('STC', 'Getting bb', 'Basic block ', '  Possible', 'Duplicated', 'Connection'))
               and 'loop_depth' not in x]
        for l in log[:90]:
            ids = set(re.findall(r'\b(?:bb|block|start of (?:this|next) round:|Connection:)\s*(\d+)', l)) | set(re.findall(r'\b(\d+)\b', l) if l.startswith('Connection') else [])
            print('   %s %s' % ('*' if ids & {str(x) for x in mine} else ' ', l))


def rtl_bbs(dump, want):
    """RTL basic blocks (by note) that contain insns located at the wanted source lines."""
    out, cur = set(), None
    for l in dump.split('\n'):
        m = re.match(r'\(note \d+ \d+ \d+ (\d+) \[bb \d+\] NOTE_INSN_BASIC_BLOCK\)', l)
        if m: cur = int(m[1]); continue
        m = re.match(r'\((?:insn|jump_insn|call_insn)(?:/\w+)? \d+ \d+ \d+ (\d+) \S+:(\d+) ', l)
        if m and int(m[2]) in want: out.add(int(m[1]))
    return out


def candidate_line_map(a, c):
    out = subprocess.run([str(ANALYSIS), '--dwarf=decodedline', str(a['obj'])], capture_output=True, text=True,
                         env=environment()).stdout
    rows = []
    for l in out.splitlines():
        m = re.match(r'^(\S+)\s+(\d+)\s+(0x[0-9a-f]+)', l)
        if m and Path(m[1]).name.lower() == Path(a['config']['source']).name.lower(): rows.append((int(m[3], 16), int(m[2])))
    sym = subprocess.run([str(ANALYSIS), '-t', str(a['obj'])], capture_output=True, text=True, env=environment()).stdout
    base = None
    for l in sym.splitlines():
        m = re.search(r'\(sec\s+1\).*0x([0-9a-f]+) (\S+)$', l)
        if m and m[2] == '_' + a['f']['name']: base = int(m[1], 16)
    if base is None: return []
    rows = sorted((x - base, ln) for x, ln in rows if x >= base)
    offs = [x for x, _ in rows]; out = set()
    for r in c.rows:
        i = bisect.bisect_right(offs, r['off']) - 1
        if i >= 0: out.add(rows[i][1])
    return sorted(out)


def section(d, suffix, fn):
    files = sorted(d.glob('*.' + suffix))
    if not files: return None
    s = files[0].read_text(encoding='utf-8', errors='replace')
    parts = re.split(r'(?m)^(?=;; Function )', s)
    mine = [p for p in parts if re.match(r';; Function %s(\s|$)' % re.escape(fn), p)]
    return '\n'.join(mine) or None


def prediction_blocks(prof, want):
    """Pair 'Predictions for bb N' blocks with the source lines of their statements."""
    body = prof.split('\n')
    lines_by_bb, cur = collections.defaultdict(set), None
    for l in body:
        m = re.match(r'^<bb (\d+)>:', l.strip()) or re.match(r'^# BLOCK (\d+)', l.strip())
        if m: cur = m[1]; continue
        for x in re.findall(r'\[[^\]]*:\s*(\d+)\]', l):
            if cur: lines_by_bb[cur].add(int(x))
    preds, cur = collections.OrderedDict(), None
    for l in body:
        m = re.match(r'^Predictions for bb (\d+)', l)
        if m: cur = m[1]; preds[cur] = []; continue
        if cur and l.startswith('  '): preds[cur].append(l.strip())
        elif cur and not l.startswith('  '): cur = None
    return [(bb, t) for bb, t in preds.items() if lines_by_bb.get(bb, set()) & want]


def tree_blocks(opt, want):
    out = []
    for blk in re.split(r'\n(?=\s*# BLOCK \d+)', opt):
        m = re.search(r'# BLOCK (\d+) freq:(\d+)', blk)
        if not m: continue
        ls = {int(x) for x in re.findall(r'\[[^\]]*:\s*(\d+)\]', blk)}
        if not ls & want: continue
        stm = [re.sub(r'\[[^\]]*:\s*\d+\] ', '', x).strip() for x in blk.split('\n')
               if x.strip() and not x.strip().startswith('#') and 'goto' not in x and x.strip() not in ('else',)]
        sc = re.search(r'# SUCC: (.*)', blk)
        out.append('bb %s freq %s  %s  -> %s' % (m[1], m[2], ' ; '.join(stm)[:110], sc[1].strip() if sc else ''))
    return out


def return_edges(opt):
    blks = {}
    for blk in re.split(r'\n(?=\s*# BLOCK \d+)', opt):
        m = re.search(r'# BLOCK (\d+) freq:(\d+)', blk)
        if m: blks[m[1]] = (int(m[2]), blk)
    out = []
    for bb, (freq, blk) in blks.items():
        if not re.search(r'# SUCC: EXIT', blk): continue
        pm = re.search(r'# PRED: (.*)', blk)
        for src, pct in re.findall(r'(\d+) \[([\d.]+)%\]', pm[1] if pm else ''):
            f = blks.get(src, (0,))[0] * float(pct) / 100
            out.append('bb %s -> return bb %s  edge freq %6.0f  %s' % (src, bb, f, 'copy' if f >= 1000 else 'shared'))
    return out


def ira_summary(ira, want):
    out, names = [], {}
    for m in re.finditer(r'\[orig:(\d+) ([\w.]+) \]', ira): names.setdefault(m[1], m[2])
    for m in re.finditer(r'\(reg(?:/[a-z]+)*:\w+ (\d+) \[ ([\w.]+) \]\)', ira): names.setdefault(m[1], m[2])
    # hard registers / spill decisions for pseudos whose insns carry the wanted lines
    touched = set()
    for blk in re.split(r'\n\n', ira):
        m = re.search(r':(\d+) \(', blk)
        if m and int(m[1]) in want:
            touched |= set(re.findall(r'orig:(\d+) ', blk)) | set(re.findall(r'\(reg(?:/[a-z]+)*:\w+ (\d+) ', blk))
    for l in ira.split('\n'):
        m = re.search(r'(Popping|Pushing) a\d+\(r(\d+),', l)
        if m and m[2] in touched: out.append('%-22s %s' % (names.get(m[2], 'r' + m[2]), l.strip()))
        m = re.search(r'Slot \d+ \(freq,size\)', l)
        if m: out.append(l.strip())
    return out[:60]


def differ_functions():
    state = read_json(ROOT / 'recovery.json')
    return sorted(n for u in state['units'].values() for n, f in u['functions'].items() if f['status'] != 'FUNCTION_MATCH')


def triage(specs):
    """One compile per function; a compact table of where each first diverges."""
    print('insns = original instructions inside unpaired/non-exact blocks (placement-independent)')
    print('%-34s %5s %5s %-9s %-7s %-18s %-24s %6s  %s' % ('function', 'size', 'insns', 'blocks', 'tails', 'bucket',
                                                           'first class', 'line', 'issue classes'))
    for spec in specs or differ_functions():
        name, _, cand = spec.partition('=')
        body, patch = (None, cand) if cand.endswith('.json') else (cand or None, None)
        try:
            a = analyse(name, None, body, patch)
        except SystemExit as e:
            print('%-34s compile failed: %s' % (name, str(e).strip().splitlines()[-1][:80])); continue
        s = summarize(a)
        # placement-independent mismatch: original instructions in unpaired or non-exact blocks
        bad = {b.id for b, k, _ in a['issues'] if BUCKET.get(k) in (1, 2)}
        diffs = sum(len([r for r in b.rows if not PAD.match(r['norm'])]) for b in a['ob'] if b.id in bad)
        first = s['first'][1] if s['first'] else ('order' if s['displaced'] else '-')
        kinds = ' '.join('%s:%d' % (k, s['kinds'][k]) for k in ORDER if s['kinds'][k])
        print('%-34s %5d %5s %-9s %-7s %-18s %-24s %6s  %s' % (
            name + ('*' if cand else ''), a['f']['size'], diffs, '%d/%d/%d' % (*s['blocks'], s['paired']),
            '%d/%d' % s['tails'], '%d %s' % (s['bucket'], BUCKET_NAME[s['bucket']]), first,
            s.get('first_line', '-') if s.get('first_line', 10 ** 9) < 10 ** 9 else '-', kinds))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('function', nargs='*', help='one function, or with --triage FUNC[=body.c|patch.json] specs')
    ap.add_argument('--target'); ap.add_argument('--body'); ap.add_argument('--patch')
    ap.add_argument('--why', action='store_true', help='compile with GCC dumps and show evidence for the first divergence')
    ap.add_argument('--blocks', action='store_true', help='list every divergent block')
    ap.add_argument('--triage', action='store_true', help='table over DIFFER functions (or the given specs)')
    a = ap.parse_args()
    if a.triage:
        return triage(a.function)
    if len(a.function) != 1: ap.error('exactly one function expected')
    res = analyse(a.function[0], a.target, a.body, a.patch)
    first = report(res, a.blocks)
    if first and a.why: why(res, first)


if __name__ == '__main__':
    main()
