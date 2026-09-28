"""Equivalence certificate for a body-only candidate that is not byte-exact.

certify.py FUNCTION CANDIDATE.c [--note TEXT] [--out evidence/equivalence/FUNCTION.json]

Materialises the braced body on the canonical TU (never touching canonical files), compiles it with the
locked toolchain, runs the lockstep simulation against the oracle's exported assembly and line table,
and records: simulation states/pairs/divergences, identical line-table lines, sizes, exact-peer changes
(must be none), the candidate body hash the certificate binds to, and the tool/oracle identities.
The certificate never grants FUNCTION_MATCH; promote.py --equivalent consumes it."""
import sys, json, argparse, hashlib
from pathlib import Path
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE)); sys.path.insert(0, str(HERE.parent))
from common import ROOT, BUILD, ORACLE, identity, write_json
from scratch import resolve_function, fragment, materialized, acceptance_syntax
from build import compile_target
from source_scope import body_hash
import objfun, bisim

def certify(name, path, note='', target=None):
    target, config, function = resolve_function(name, target); name = function['name']
    body = fragment(path, name); acceptance_syntax(body)
    baseline_obj, _ = compile_target(target, BUILD/'equivalence/baseline'/target)
    with materialized(target, name, body) as shadow:
        obj, compilation = compile_target(target, shadow/'verified', shadow)
        text = (shadow/config['source']).read_bytes().decode('cp1252')
        bsha = body_hash(text, name)
        delta = objfun.delta_for(obj, name)
        steps, matched, failures, states, reordered, skipped, o, c, io, ic, narrowed = bisim.run(name, obj, delta)
        same, diff, report, o2, c2 = objfun.compare(name, obj, delta)
        fns = objfun.functions(obj); lo, hi, raw, rel = fns[name]
        peers = objfun.peers_changed(obj, baseline_obj, skip=(name,))
        oracle_asm = objfun.oracle_files(name)['asm']
        seen = set(); div = []
        for ln, po, pc, msg in failures:
            k = (ln[1] if ln else None, msg[:60])
            if k in seen: continue
            seen.add(k); div.append({'line': ln[1] if ln else None, 'oracle_offset': po, 'candidate_offset': pc, 'message': msg[:200]})
        cert = {'function': name, 'target': target, 'source': config['source'], 'candidate_body_sha256': bsha,
                'candidate_file': str(Path(path).as_posix()), 'method': 'lockstep simulation modulo register allocation, stack slots and layout (tools/equivalence/bisim.py) plus per-line line-table comparison (objfun.compare)',
                'simulation': {'states': states, 'pairs': steps, 'matched': matched, 'reordered': reordered, 'divergences': len(failures), 'distinct_divergences': div, 'unverified_skipped': skipped, 'narrowed_immediates': narrowed},
                'line_table': {'identical_lines': same, 'differing_lines': diff, 'oracle_instructions': len(o), 'candidate_instructions': len(c)},
                'sizes': {'original': oracle_asm['size'], 'candidate_span': hi - lo},
                'exact_peers_changed': peers, 'note': note,
                'oracle': {'path': str(ORACLE), 'identity': identity(ORACLE)},
                'tools': {p.name: identity(p) for p in sorted(HERE.glob('*.py'))},
                'line_delta': delta, 'object': compilation['object']}
        return cert

if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__); ap.add_argument('function'); ap.add_argument('candidate', type=Path)
    ap.add_argument('--note', default=''); ap.add_argument('--out'); ap.add_argument('--target'); a = ap.parse_args()
    cert = certify(a.function, a.candidate, a.note, a.target)
    out = Path(a.out) if a.out else ROOT/'evidence/equivalence'/(cert['function'] + '.json')
    out.parent.mkdir(parents=True, exist_ok=True); write_json(out, cert)
    s = cert['simulation']; l = cert['line_table']
    print('%s: states %d pairs %d divergences %d skipped %d | identical lines %d/%d | insns %d vs %d | peers changed %s | body %s' % (
        cert['function'], s['states'], s['pairs'], s['divergences'], s['unverified_skipped'], l['identical_lines'], l['identical_lines'] + l['differing_lines'], l['candidate_instructions'], l['oracle_instructions'], cert['exact_peers_changed'], cert['candidate_body_sha256'][:12]))
    print('wrote', out)
