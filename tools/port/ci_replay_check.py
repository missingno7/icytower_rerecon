"""Replay determinism check without the reference build or any game data.

Regenerates the synthetic replays (tools/port/mkreplay.py, fixed seeds), runs
the port's headless replay checker on each, and compares the SHA-256 of its
XML output with tests/port/replays/expected.txt, which was recorded from the
frozen reference build.  Used by CI on every platform and CPU.

    python tools/port/ci_replay_check.py EXE [--jobs 4] [--timeout 60]
        [--prefix "arch -x86_64"]      # e.g. run the Intel slice under Rosetta

Exit status 0 when every replay matches.
"""
import argparse
import hashlib
import os
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SETS = {'a': ['--count', '150', '--seed', '11'],
        'b': ['--count', '150', '--seed', '12', '--custom']}


def norm(text):
    text = text.replace('\r\n', '\n')
    i = text.find('<itrcheck_results')
    return text[i:].strip() + '\n' if i >= 0 else ''


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('exe')
    ap.add_argument('--expected', default=str(ROOT / 'tests' / 'port' / 'replays' / 'expected.txt'))
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 2)
    ap.add_argument('--timeout', type=float, default=60)
    ap.add_argument('--prefix', default='', help='command prefix, e.g. "arch -x86_64"')
    a = ap.parse_args()

    expected = []
    for line in open(a.expected, encoding='utf-8'):
        if line.strip() and not line.startswith('#'):
            s, name, digest = line.split()
            expected.append((s, name, digest))

    work = Path(tempfile.mkdtemp(prefix='itower-ci-replays-'))
    for s, args in SETS.items():
        subprocess.run([sys.executable, str(ROOT / 'tools' / 'port' / 'mkreplay.py'), str(work / s), *args],
                       check=True, stdout=subprocess.DEVNULL)
    user = work / 'user'
    user.mkdir()
    env = dict(os.environ, ITOWER_USER_DIR=str(user), ITOWER_HEADLESS='1')
    exe = str(Path(a.exe).resolve())
    prefix = shlex.split(a.prefix)

    def one(item):
        s, name, digest = item
        replay = work / s / (name + '.itr')
        try:
            r = subprocess.run(prefix + [exe, '-check', str(replay), '-all'], cwd=work, env=env,
                               capture_output=True, timeout=a.timeout)
        except subprocess.TimeoutExpired:
            return name, 'timeout'
        out = norm(r.stdout.decode('latin-1'))
        if not out:
            return name, 'no output (exit %d)' % r.returncode
        got = hashlib.sha256(out.encode('latin-1')).hexdigest()
        return name, None if got == digest else 'different result'

    with ThreadPoolExecutor(max_workers=a.jobs) as pool:
        results = list(pool.map(one, expected))
    bad = [(n, e) for n, e in results if e]
    for n, e in bad:
        print('FAIL', n, e)
    print('%d/%d replays identical to the reference build' % (len(results) - len(bad), len(results)))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
