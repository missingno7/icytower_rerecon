#!/usr/bin/env python3
"""Replay regression: frozen reference build vs portable SDL3 build.

Runs the historical headless replay checker (`-check REPLAY -all`) of both
executables on the same replay files and compares the complete XML output
(claimed and actual results, every combo, every jump sequence, key counts and
the anti-cheat timing table).  The reference executable is the ordinary link
of the frozen sources (docs/port/BASELINE.md); it is never the original game.

    python tools/port/replay_check.py REPLAY_OR_DIR [...]
        [--ref build/oracle-ref/icytower-ref.exe]
        [--port build/port/release/icytower.exe]
        [--jobs N] [--timeout SEC] [--save DIR] [--quiet]

Exit status 0 when every replay produced identical output on both builds.
"""
import argparse, concurrent.futures, os, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def find_port_exe():
    for p in [ROOT / 'build/port/release/icytower.exe', ROOT / 'build/port/release/icytower',
              ROOT / 'build/port/debug/icytower.exe', ROOT / 'build/port/debug/icytower']:
        if p.exists():
            return p
    return ROOT / 'build/port/release/icytower.exe'


def run_check(exe, replay, timeout, extra_env=None, workdir=None, extra_args=()):
    env = dict(os.environ)
    if extra_env:
        env.update(extra_env)
    try:
        r = subprocess.run([str(exe), '-check', str(replay), '-all', *extra_args], cwd=workdir,
                           capture_output=True, timeout=timeout, env=env)
    except subprocess.TimeoutExpired:
        return None, 'timeout'
    out = r.stdout.decode('latin-1').replace('\r\n', '\n')
    i = out.find('<itrcheck_results')
    if i < 0:
        return None, f'no XML (exit {r.returncode}): {r.stderr.decode("latin-1")[:200]}'
    return out[i:], None


def collect(paths):
    files = []
    for p in paths:
        p = Path(p)
        if p.is_dir():
            files += sorted(p.rglob('*.itr'))
        else:
            files.append(p)
    return [f.resolve() for f in files]


def first_diff(a, b):
    la, lb = a.splitlines(), b.splitlines()
    for i, (x, y) in enumerate(zip(la, lb)):
        if x != y:
            return f'line {i+1}: ref {x.strip()!r} / port {y.strip()!r}'
    return f'length {len(la)} vs {len(lb)} lines'


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('replays', nargs='+')
    ap.add_argument('--ref', default=str(ROOT / 'build/oracle-ref/icytower-ref.exe'))
    ap.add_argument('--port', default=None)
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--timeout', type=float, default=120)
    ap.add_argument('--save', default=None, help='write both XML outputs here')
    ap.add_argument('--quiet', action='store_true')
    a = ap.parse_args()
    ref, port = Path(a.ref), Path(a.port) if a.port else find_port_exe()
    files = collect(a.replays)
    if not files:
        sys.exit('no replay files')
    save = Path(a.save) if a.save else None
    if save:
        save.mkdir(parents=True, exist_ok=True)
    userdir = Path(tempfile.mkdtemp(prefix='itower-check-'))

    def one(f):
        xr, er = run_check(ref, f, a.timeout, workdir=ref.parent)
        for _ in range(3):
            # the reference build occasionally exits without output when many
            # instances start at once (Allegro/DirectX start-up); retry
            if er is None or not er.startswith('no XML'):
                break
            xr, er = run_check(ref, f, a.timeout, workdir=ref.parent)
        xp, ep = run_check(port, f, a.timeout, extra_env={'ITOWER_USER_DIR': str(userdir)})
        if save:
            if xr: (save / (f.stem + '.ref.xml')).write_text(xr, encoding='latin-1')
            if xp: (save / (f.stem + '.port.xml')).write_text(xp, encoding='latin-1')
        if er == 'timeout' and ep == 'timeout':
            # the input never ends the game (e.g. the player never climbs, so
            # the tower never scrolls): both builds agree, but nothing to compare
            return f, 'HANG', 'both builds still running at the timeout'
        if er or ep:
            return f, 'ERROR', f'ref: {er or "ok"}; port: {ep or "ok"}'
        if xr == xp:
            return f, 'SAME', ''
        return f, 'DIFF', first_diff(xr, xp)

    counts = {'SAME': 0, 'DIFF': 0, 'ERROR': 0, 'HANG': 0}
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as pool:
        for f, status, detail in pool.map(one, files):
            counts[status] += 1
            if status != 'SAME' or not a.quiet:
                print(f'{status:5} {f.name} {detail}')
    print(f"{counts['SAME']} identical, {counts['DIFF']} different, {counts['ERROR']} errors, "
          f"{counts['HANG']} endless on both ({len(files)} replays)")
    sys.exit(0 if counts['DIFF'] == 0 and counts['ERROR'] == 0 else 1)


if __name__ == '__main__':
    main()
