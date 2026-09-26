"""Compiler-pipeline diagnostics for the last-mile DIFFER functions (search guidance only).

Answers pass-level questions about how the locked GCC 4.4.1 treats one candidate body:

  isolate  compile the TU's declaration prelude plus ONE function (about 1 s instead of a whole
           TU) and print the diag.py block classification.  Per-function passes (profile, jump
           threading, IRA, bb-reorder) see the same function; TU-wide state (peephole2 scratch
           registers, DECL_UID counts) may differ, so confirm with diag.py / search.py.
  threads  every tree jump-threading event: pass, path block, threaded-through block and its
           condition (with the operand's defining statement) in that pass's INPUT CFG.
           --class NAME=REGEX labels events; --want NAME@PASSES / --forbid NAME turn the list
           into a pass/fail fingerprint usable as a search objective.
  predict  every branch as the profile pass (037t) predicted it: each heuristic's P(then),
           first-match vs Dempster-Shafer combination, the result, and the CFG facts that make
           the structural heuristics apply (guarded side-effect calls, paths into the return).
  whatif   DIAGNOSTIC ONLY: force one if-condition at a time to probability P with a patched
           copy of cc1 (the __builtin_expect hitrate) and report the isolated diag outcome.
  stc      bb-reorder (STC) traces and heap keys, replayed with an exact libiberty fibheap
           model; --key BB=K re-runs the replay with a changed key to test tie hypotheses.

Nothing here can produce a candidate: probes, patched compilers and extra flags exist only to
establish causality.  Machine-code equality through search.py/promote.py remains the only
acceptance criterion.

  python tools/pipeline.py isolate handle_player_collision_vector_2 --body candidates\\x.c
  python tools/pipeline.py threads handle_player_collision_vector_2 --body candidates\\x.c ^
      --class "SUM=left_\\d+ \\+ right" --class "EDGE=left_\\d+ != right" --want SUM@vrp2,dom2 --forbid EDGE
  python tools/pipeline.py predict handle_player_collision_original --body candidates\\x.c
  python tools/pipeline.py whatif load_character_bmp --body candidates\\x.c 300 1000 3900
  python tools/pipeline.py stc handle_player_collision_vector_2 --body candidates\\x.c --key 12=-1000
"""
import argparse, os, re, shutil, struct, sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import diag
from common import ROOT, TC
from scratch import resolve_function

DIAG = ROOT / 'build' / 'diag'


# ------------------------------------------------------------------ isolated compilation
_prelude_cache = {}


def prelude(config, fn):
    """Source before FUNC with non-static function bodies replaced by prototypes, and FUNC's header."""
    key = (config['source'], fn)
    if key in _prelude_cache: return _prelude_cache[key]
    src = (ROOT / config['source']).read_bytes().decode('cp1252')
    m = re.search(r'\n([^\n;{}]*\b%s\s*\([^;{]*\))\s*\{' % re.escape(fn), src)
    if not m: raise SystemExit('definition of %s not found' % fn)
    text, res, pos = src[:m.start()], [], 0
    for fm in re.finditer(r'\n((?!static)(?!inline)[A-Za-z_][^\n;{}()=]*\([^;{}]*\))\s*\{', text):
        if fm.start() < pos: continue
        j = text.index('{', fm.end() - 1); d = 0; k = j
        while k < len(text):
            d += {'{': 1, '}': -1}.get(text[k], 0); k += 1
            if d == 0: break
        res.append(text[pos:fm.start()]); res.append('\n' + fm[1] + ';'); pos = k
    res.append(text[pos:])
    _prelude_cache[key] = (''.join(res), m[1].strip())
    return _prelude_cache[key]


def body_text(fn, body):
    """The candidate body (braced), or the canonical body when BODY is None."""
    if body: return Path(body).read_bytes().decode('cp1252')
    t, c, f = resolve_function(fn)
    src = (ROOT / c['source']).read_bytes().decode('cp1252')
    m = re.search(r'\n[^\n;{}]*\b%s\s*\([^;{]*\)\s*\{' % re.escape(fn), src)
    i = src.index('{', m.start()); d = 0; j = i
    while True:
        d += {'{': 1, '}': -1}.get(src[j], 0); j += 1
        if d == 0: return src[i:j]


def compile_isolated(fn, body=None, flags=(), cc1=None, dumps=False):
    t, c, f = resolve_function(fn)
    pre, header = prelude(c, fn)
    text = pre + '\n' + header + '\n' + body_text(fn, body) + '\n'
    diag.CC1_DIR = cc1
    old = diag.DUMP_FLAGS
    diag.DUMP_FLAGS = tuple(flags)
    try:
        obj, d = diag.compile_tu(c, text, dumps=dumps or bool(flags))
    finally:
        diag.DUMP_FLAGS = old
        diag.CC1_DIR = None
    return obj, Path(d), (t, c, f, text)


def analyse_isolated(fn, body=None, flags=(), cc1=None):
    obj, d, (t, c, f, text) = compile_isolated(fn, body, flags, cc1)
    a = diag.classify(diag.original_rows(f), diag.candidate_rows(obj, f['name']), f['name'])
    a.update(target=t, config=c, f=f, text=text, ltab=diag.source_lines(f, c), obj=obj)
    return a


def score(fn, body=None, flags=(), cc1=None):
    """(bucket, issue count, displaced block count) for the isolated compile."""
    s = diag.summarize(analyse_isolated(fn, body, flags, cc1))
    return s['bucket'], sum(s['kinds'].values()), len(s['displaced'])


# ------------------------------------------------------------------ dump parsing
PASS_RE = re.compile(r'\.(\d+)([tri])\.(\w+)$')
BLOCK_RE = re.compile(r'# BLOCK (\d+)[^\n]*\n(.*?)(?=\n\s*# BLOCK \d+|\n}\s*\n|\Z)', re.S)
LOC_RE = re.compile(r'\[[^\]]*\] ?')
LINE_RE = re.compile(r' : (\d+)\]')
CALL_RE = re.compile(r'([A-Za-z_]\w*) \(')
IF_RE = re.compile(r'\bif \(')


def passes(d):
    """Dump files of one compile in pipeline order: [(number, kind, name, path)]."""
    out = []
    for x in os.listdir(d):
        m = PASS_RE.search(x)
        if m: out.append((int(m[1]), m[2], m[3], Path(d) / x))
    return sorted(out)


def fn_part(text, fn):
    i = text.find(';; Function %s ' % fn)
    if i < 0: return None
    j = text.find(';; Function', i + 10)
    return text[i:j if j > 0 else None]


def parse_blocks(text):
    """Blocks of a tree dump written with -blocks: bb -> stmts/cond/then/else/succ/calls/returns/line."""
    blocks = {}
    for m in BLOCK_RE.finditer(text):
        b, txt = int(m[1]), m[2]
        raw = [x.strip() for x in txt.split('\n') if x.strip() and not x.strip().startswith('#')]
        stm = [LOC_RE.sub('', x) for x in raw]
        cond = next((x for x in stm if IF_RE.search(x)), None)
        sm = re.search(r'# SUCC:([^\n]*)', txt)
        succ = re.findall(r'(\d+) (?:\[[^\]]*\] )?\(([^)]*)\)', sm[1]) if sm else []
        lines = [int(x) for r in raw for x in LINE_RE.findall(r)]
        blocks[b] = {'stmts': stm, 'cond': cond, 'succ': [int(x) for x, _ in succ],
                     'then': next((int(x) for x, fl in succ if 'true' in fl), None),
                     'else': next((int(x) for x, fl in succ if 'false' in fl), None),
                     'calls': [c for x in stm if not IF_RE.search(x) for c in CALL_RE.findall(x)],
                     'returns': any(re.search(r'\breturn\b', x) for x in stm),
                     'line': (next((int(x) for r in raw if IF_RE.search(r) for x in LINE_RE.findall(r)), None)
                              or (lines[0] if lines else None))}
    return blocks


def definitions(blocks):
    out = {}
    for b in blocks.values():
        for s in b['stmts']:
            m = re.match(r'(\S+) = (.*);$', s)
            if m: out[m[1]] = m[2]
    return out


def describe_cond(block, defs):
    """Condition text plus the defining statement of an SSA temporary operand."""
    cond = block.get('cond') or ''
    m = re.match(r'if \((\S+) \S+ (\S+)\)', cond)
    extra = ''
    if m and m[1] in defs and not re.match(r'\w+_\d+$', defs[m[1]]):
        extra = '   [%s = %s]' % (m[1], defs[m[1]])
    return cond + extra


# ------------------------------------------------------------------ jump threading
def thread_events(fn, body=None, flags=(), cc1=None):
    """Every tree 'Threaded jump A --> B to C' with the pass and B's condition in the pass input."""
    obj, d, ctx = compile_isolated(fn, body, ('-fdump-tree-all-details-blocks',) + tuple(flags), cc1)
    prev, out = None, []
    for num, kind, name, path in passes(d):
        if kind != 't': continue
        part = fn_part(path.read_text(errors='replace'), fn)
        if part is None: continue
        for m in re.finditer(r'Threaded jump (\d+) --> (\d+) to (\d+)', part):
            blocks = parse_blocks(prev) if prev else {}
            blk = blocks.get(int(m[2]), {})
            out.append({'pass': name, 'num': num, 'from': int(m[1]), 'through': int(m[2]), 'to': int(m[3]),
                        'cond': describe_cond(blk, definitions(blocks)), 'stmts': blk.get('stmts', [])})
        if '# BLOCK' in part: prev = part
    return out, obj


def classify_events(events, classes):
    for e in events:
        e['class'] = next((n for n, rx in classes if re.search(rx, e['cond'])), '-')
    return events


def fingerprint_ok(events, want, forbid):
    """want: {class: set(passes) or None}; forbid: set(class).  True when satisfied."""
    got = {}
    for e in events: got.setdefault(e['class'], set()).add(e['pass'])
    for c, ps in want.items():
        if c not in got or (ps and not (got[c] & ps)): return False
    return not any(c in got for c in forbid)


# ------------------------------------------------------------------ branch prediction
def parse_predictions(text):
    """bb -> [(heuristic, ignored, P(then) in %)] from a -details profile dump."""
    preds = {}
    for m in re.finditer(r'Predictions for bb (\d+)\n((?:  .*\n)+)', text):
        items = re.findall(r'  (.+?) heuristics( \(ignored\))?: ([\d.]+)%', m[2])
        preds[int(m[1])] = [(n, bool(ig), float(p)) for n, ig, p in items]
    return preds


def ds_combine(probs):
    """Dempster-Shafer combination of P(then) values in [0, 1] (GCC's non-first-match rule)."""
    t = f = 1.0
    for p in probs: t *= p; f *= 1 - p
    return t / (t + f) if t + f else 0.5


SUMMARY = ('combined', 'DS theory', 'first match', 'no prediction')


def predictions(fn, body=None, flags=(), cc1=None):
    obj, d, (t, c, f, text) = compile_isolated(fn, body, ('-fdump-tree-all-details-lineno-blocks',) + tuple(flags), cc1)
    prof, cfg = None, None
    for num, kind, name, path in passes(d):
        if kind != 't': continue
        part = fn_part(path.read_text(errors='replace'), fn)
        if part is None: continue
        if name == 'profile': prof = part; break
        if '# BLOCK' in part: cfg = part
    if prof is None: raise SystemExit('no profile dump for %s' % fn)
    preds, blocks = parse_predictions(prof), parse_blocks(cfg or '')
    # map compiled line numbers onto historical ones via the function's opening line
    comp_first = prelude(c, fn)[0].count('\n') + 3      # the opening-brace line (GCC's prologue line)
    orig_first = diag.source_lines(f, c)[0][1]
    out = []
    for b, items in sorted(preds.items()):
        blk = blocks.get(b, {})
        facts = []
        for side in ('then', 'else'):
            s = blk.get(side)
            if s is None: continue
            sb = blocks.get(s, {})
            if sb.get('calls'): facts.append('%s->bb%d calls %s' % (side, s, ', '.join(sb['calls'][:3])))
            nxt = sb.get('succ', [])
            if sb.get('returns') or (len(nxt) == 1 and blocks.get(nxt[0], {}).get('returns')):
                facts.append('%s->bb%d is/reaches the return block' % (side, s))
        ln = blk.get('line')
        out.append({'bb': b, 'line': ln - comp_first + orig_first if ln else None, 'cond': blk.get('cond') or '?',
                    'heuristics': [(n.replace(' (on trees)', ''), p) for n, ig, p in items if n not in SUMMARY],
                    'method': 'first-match' if any(n == 'first match' and not ig for n, ig, p in items) else 'DS',
                    'combined': next((p for n, ig, p in items if n == 'combined'), None), 'facts': facts})
    return out


# ------------------------------------------------------------------ what-if probes (DIAGNOSTIC ONLY)
def _pe_sections(b):
    pe = struct.unpack_from('<I', b, 0x3c)[0]
    nsec = struct.unpack_from('<H', b, pe + 6)[0]
    opt = struct.unpack_from('<H', b, pe + 20)[0]
    base = struct.unpack_from('<I', b, pe + 24 + 28)[0]
    secs = []
    for i in range(nsec):
        vsz, va, rsz, raw = struct.unpack_from('<IIII', b, pe + 24 + opt + 40 * i + 8)
        secs.append((base + va, vsz, raw))
    return secs


def _cstring(b, secs, va):
    for s, sz, raw in secs:
        if s <= va < s + sz:
            o = raw + va - s
            e = b.index(b'\0', o)
            return b[o:e].decode('latin-1')
    return None


def predictor_table(b):
    """File offset of GCC's predictor_info[] (12-byte {name, hitrate, flags}) found by its names."""
    secs = _pe_sections(b)
    for pos in range(0, len(b) - 24, 4):
        ptr, hit, fl = struct.unpack_from('<Iii', b, pos)
        if hit != 10000: continue
        if _cstring(b, secs, ptr) != 'combined': continue
        if _cstring(b, secs, struct.unpack_from('<I', b, pos + 12)[0]) == 'DS theory':
            table = {}
            for k in range(64):
                p, h, _ = struct.unpack_from('<Iii', b, pos + 12 * k)
                name = _cstring(b, secs, p)
                if not name: break
                table[name] = (pos + 12 * k + 4, h)
            return table
    raise SystemExit('predictor table not found in cc1.exe')


def patched_cc1(predictor, hitrate):
    """build/diag/cc1_<predictor>_<hitrate>/ with one predictor hitrate changed (causal probes only)."""
    base = DIAG / 'cc1_base'
    if not (base / 'cc1.exe').exists():
        src = next(TC.glob('libexec/gcc/*/*/cc1.exe'))
        shutil.copytree(src.parent, base, ignore=shutil.ignore_patterns('*.a', 'include*', 'install-tools', 'plugin'))
    tag = re.sub(r'\W+', '_', predictor)
    d = DIAG / ('cc1_%s_%d' % (tag, hitrate))
    if not (d / 'cc1.exe').exists():
        shutil.copytree(base, d)
        b = bytearray((d / 'cc1.exe').read_bytes())
        off, _ = predictor_table(bytes(b))[predictor]
        struct.pack_into('<i', b, off, hitrate)
        (d / 'cc1.exe').write_bytes(bytes(b))
    return str(d)


def conditions(text):
    out = []
    for m in re.finditer(r'\bif\s*\(', text):
        i, depth = m.end(), 1
        while depth:
            depth += {'(': 1, ')': -1}.get(text[i], 0); i += 1
        out.append((m.end(), i - 1))
    return out


def whatif(fn, body, probs, only=None):
    """[(index, line, condition, [(P, score)])] forcing one condition at a time."""
    src = body_text(fn, body)
    dirs = {p: patched_cc1('__builtin_expect', p) for p in probs}
    probe_dir = DIAG / 'whatif'; probe_dir.mkdir(parents=True, exist_ok=True)
    rows = []
    for k, (a, b) in enumerate(conditions(src)):
        if only is not None and k not in only: continue
        probe = src[:a] + '__builtin_expect(!!(' + src[a:b] + '), 1)' + src[b:]
        path = probe_dir / ('%s_%d.c' % (fn, k))
        path.write_bytes(probe.encode('cp1252'))
        res = []
        for p in probs:
            try: res.append((p, score(fn, str(path), cc1=dirs[p])))
            except SystemExit: res.append((p, None))
        rows.append((k, src[:a].count('\n') + 1, src[a:b].replace('\n', ' '), res))
    return rows


# ------------------------------------------------------------------ bb-reorder heap model
class _Node:
    __slots__ = ('key', 'data', 'left', 'right', 'parent', 'child', 'degree', 'mark')

    def __init__(s, k, d):
        s.key, s.data, s.left, s.right, s.parent, s.child, s.degree, s.mark = k, d, s, s, None, None, 0, 0


def _ins_after(a, b):
    if a is a.right: a.right = b; a.left = b; b.right = a; b.left = a
    else: b.right = a.right; a.right.left = b; a.right = b; b.left = a


def _remove(n):
    ret = None if n is n.left else n.left
    if n.parent is not None and n.parent.child is n: n.parent.child = ret
    n.right.left = n.left; n.left.right = n.right; n.parent = None; n.left = n; n.right = n
    return ret


class FibHeap:
    """libiberty fibheap.c, operation for operation (ties are what bb-reorder depends on)."""

    def __init__(s): s.root = None; s.min = None; s.nodes = 0

    def _ins_root(s, n):
        if s.root is None: s.root = n; n.left = n; n.right = n; return
        _ins_after(s.root, n)

    def _rem_root(s, n):
        if n.left is n: s.root = None
        else: s.root = _remove(n)

    def insert(s, k, d):
        n = _Node(k, d); s._ins_root(n)
        if s.min is None or n.key < s.min.key: s.min = n
        s.nodes += 1; return n

    def _link(s, node, parent):
        if parent.child is None: parent.child = node
        else: _ins_after(parent.child.left, node)
        node.parent = parent; parent.degree += 1; node.mark = 0

    def _consolidate(s):
        a = [None] * 64
        while s.root is not None:
            w = s.root; x = w; s._rem_root(w); d = x.degree
            while a[d] is not None:
                y = a[d]
                if x.key > y.key: x, y = y, x
                s._link(y, x); a[d] = None; d += 1
            a[d] = x
        s.min = None
        for n in a:
            if n is not None:
                s._ins_root(n)
                if s.min is None or n.key < s.min.key: s.min = n

    def extract(s):
        ret = s.min; x = ret.child; orig = None
        while x is not orig and x is not None:
            if orig is None: orig = x
            y = x.right; x.parent = None; s._ins_root(x); x = y
        s._rem_root(ret); s.nodes -= 1
        if s.nodes == 0: s.min = None
        else: s.min = ret.right; s._consolidate()
        return ret.data

    def _cut(s, n, p):
        _remove(n); p.degree -= 1; s._ins_root(n); n.parent = None; n.mark = 0

    def _cascade(s, y):
        while y.parent is not None:
            z = y.parent
            if y.mark == 0: y.mark = 1; return
            s._cut(y, z); y = z

    def replace(s, n, k):
        if k > n.key: return
        ok, n.key, y = n.key, k, n.parent
        if ok == k and k != -2 ** 63: return
        if y is not None and n.key < y.key: s._cut(n, y); s._cascade(y)
        if n.key <= s.min.key: s.min = n


STC_EVENT = re.compile(r'(STC - round|Getting bb|\s+Possible start|Changing key|Basic block \d+ was visited)')


def stc_segments(lines):
    """(round, bb) -> the heap events recorded while that block's trace was built."""
    segs, rnd, cur = {}, None, None
    for l in lines:
        if not STC_EVENT.match(l): continue
        m = re.match(r'STC - round (\d+)', l)
        if m: rnd = int(m[1]); continue
        m = re.match(r'Getting bb (\d+)', l)
        if m: cur = (rnd, int(m[1])); segs[cur] = []; continue
        if cur is not None: segs[cur].append(l)
    return segs


def stc_replay(lines, keys=None):
    """Free-running replay of the STC heaps; keys={bb: key} overrides every insertion key of bb."""
    segs = stc_segments(lines)
    if not segs: return []
    keys = keys or {}
    rounds = sorted({r for r, _ in segs})
    cur, new = FibHeap(), FibHeap()
    node, where, out = {}, {}, []

    def run(r, b):
        out.append((r, b))
        for l in segs.get((r, b), []):
            m = re.match(r'\s+Possible start (?:point )?of (next|this) round: (\d+) \(key: (-?\d+)\)', l)
            if m:
                h = new if m[1] == 'next' else cur
                x = int(m[2]); node[x] = h.insert(keys.get(x, int(m[3])), x); where[x] = h; continue
            m = re.match(r'Basic block (\d+) was visited', l)
            if m:
                x = int(m[1])
                if x in where and x != b:
                    h = where.pop(x); h.replace(node[x], -2 ** 63); h.extract()
                continue
            m = re.match(r'Changing key for bb (\d+) from (-?\d+) to (-?\d+)', l)
            if m:
                x = int(m[1])
                if x in where: where[x].replace(node[x], keys.get(x, int(m[3])))

    first = min(segs)
    run(*first)
    for r in rounds:
        if r != first[0]: cur, new = new, FibHeap()
        while cur.nodes:
            e = cur.extract(); where.pop(e, None)
            run(r, e)
    return out


def stc(fn, body=None, flags=(), cc1=None):
    obj, d, ctx = compile_isolated(fn, body, ('-fdump-rtl-bbro-details',) + tuple(flags), cc1)
    path = next(p for n, k, name, p in passes(d) if name == 'bbro')
    part = fn_part(path.read_text(errors='replace'), fn) or ''
    return part.split('\n')


# ------------------------------------------------------------------ command line
def _pairs(values):
    out = []
    for v in values or ():
        n, _, rx = v.partition('=')
        out.append((n, rx))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('command', choices=['isolate', 'threads', 'predict', 'whatif', 'stc'])
    ap.add_argument('function')
    ap.add_argument('probs', nargs='*', type=int, help='whatif: probabilities in 1/10000')
    ap.add_argument('--body', help='braced candidate body (default: canonical source)')
    ap.add_argument('--flag', action='append', default=[], help='DIAGNOSTIC ONLY: extra gcc flag')
    ap.add_argument('--cc1', help='DIAGNOSTIC ONLY: directory with an alternative cc1.exe')
    ap.add_argument('--class', dest='classes', action='append', help='threads: NAME=REGEX on the condition')
    ap.add_argument('--want', action='append', default=[], help='threads: NAME[@pass,pass]')
    ap.add_argument('--forbid', action='append', default=[], help='threads: NAME')
    ap.add_argument('--lines', help='predict: FROM-TO historical source lines')
    ap.add_argument('--only', help='whatif: comma-separated condition indexes')
    ap.add_argument('--key', action='append', default=[], help='stc: BB=KEY replay override')
    a = ap.parse_intermixed_args()
    fn, flags = a.function, tuple(a.flag)

    if a.command == 'isolate':
        diag.report(analyse_isolated(fn, a.body, flags, a.cc1), show_blocks=True)
    elif a.command == 'threads':
        ev, _ = thread_events(fn, a.body, flags, a.cc1)
        classify_events(ev, _pairs(a.classes))
        for e in ev:
            print('%-10s %4d -> [%4d] -> %4d  %-8s %s' % (e['pass'], e['from'], e['through'], e['to'], e['class'], e['cond']))
        if a.want or a.forbid:
            want = {}
            for w in a.want:
                n, _, ps = w.partition('@')
                want[n] = set(ps.split(',')) if ps else None
            ok = fingerprint_ok(ev, want, set(a.forbid))
            print('fingerprint', 'MATCH' if ok else 'MISMATCH')
            return 0 if ok else 1
    elif a.command == 'predict':
        lo, hi = (int(x) for x in a.lines.split('-')) if a.lines else (None, None)
        for r in predictions(fn, a.body, flags, a.cc1):
            if lo is not None and (r['line'] is None or not lo <= r['line'] <= hi): continue
            h = ' '.join('%s=%.0f' % (n, p) for n, p in r['heuristics'])
            print('L%-5s bb%-3d %-52s P(then)=%5s%% %-11s %s' % (r['line'], r['bb'], r['cond'][:52], r['combined'], r['method'], h))
            for x in r['facts']: print(' ' * 12, '-', x)
    elif a.command == 'whatif':
        if not a.probs: raise SystemExit('whatif needs probabilities, e.g. 300 1000 3900')
        only = {int(x) for x in a.only.split(',')} if a.only else None
        print('base (bucket, issues, displaced):', score(fn, a.body, flags))
        for k, line, cond, res in whatif(fn, a.body, a.probs, only):
            cells = '  '.join('P%g:%s' % (p / 100, '%d/%d' % s[1:] if s else 'ERR') for p, s in res)
            print('if#%-2d L%-4d %-46s %s' % (k, line, cond[:46], cells))
    elif a.command == 'stc':
        lines = stc(fn, a.body, flags, a.cc1)
        for l in lines:
            if l.startswith('Trace ') or l.startswith('Connection') or l.startswith('Duplicated'): print(l)
        recorded = [int(m[1]) for l in lines for m in [re.match(r'Getting bb (\d+)', l)] if m]
        base = [b for _, b in stc_replay(lines)]
        print('replay validates:', base == recorded)
        if a.key:
            keys = {int(k): int(v) for k, _, v in (x.partition('=') for x in a.key)}
            for r in sorted({r for r, _ in stc_replay(lines)}):
                print('round %d:' % r, [b for rr, b in stc_replay(lines, keys) if rr == r])
    return 0


if __name__ == '__main__':
    sys.exit(main())
