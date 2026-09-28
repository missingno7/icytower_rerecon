#!/usr/bin/env python3
"""Render-rate independence test.

Plays one replay in the real (windowed) portable build under many
presentation settings - faithful/modern renderer, vsync on/off, frame caps
50/60/120/144/240/uncapped, interpolation on/off, several window sizes - and
compares the simulation fingerprint (--sim-trace: tick count and a hash of
the complete gameplay state after every step) with the headless replay
checker's.  Any difference means presentation leaked into the simulation.

    python tools/port/render_rate_test.py [--replay FILE] [--data DIR] [--speed N] [--quick]

--speed runs the simulation N times faster than real time (debug option of
the scheduler) to shorten the test; it changes how many frames fall between
ticks, which is exactly what is being varied anyway.
"""
import argparse, os, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

CONFIGS = [
    ('faithful vsync', ['--renderer', 'faithful', '--vsync', 'on']),
    ('modern vsync interp', ['--renderer', 'modern', '--vsync', 'on', '--interpolation', 'on']),
    ('modern vsync no-interp', ['--renderer', 'modern', '--vsync', 'on', '--interpolation', 'off']),
    ('modern cap 50', ['--renderer', 'modern', '--vsync', 'off', '--max-fps', '50']),
    ('modern cap 60', ['--renderer', 'modern', '--vsync', 'off', '--max-fps', '60']),
    ('modern cap 120', ['--renderer', 'modern', '--vsync', 'off', '--max-fps', '120']),
    ('modern cap 144', ['--renderer', 'modern', '--vsync', 'off', '--max-fps', '144']),
    ('modern cap 240', ['--renderer', 'modern', '--vsync', 'off', '--max-fps', '240']),
    ('modern uncapped', ['--renderer', 'modern', '--vsync', 'off', '--max-fps', '0']),
    ('modern 640x480 linear', ['--renderer', 'modern', '--window', '640x480', '--filter', 'linear']),
    ('modern 3440x1440', ['--renderer', 'modern', '--window', '3440x1440']),
    ('modern 3840x2160', ['--renderer', 'modern', '--window', '3840x2160']),
    ('faithful uncapped', ['--renderer', 'faithful', '--vsync', 'off', '--max-fps', '0']),
]


def exe_path():
    for p in [ROOT / 'build/port/release/icytower.exe', ROOT / 'build/port/release/icytower']:
        if p.exists():
            return p
    sys.exit('build the game first: python tools/port/build.py')


def trace_line(path):
    try:
        lines = [l.strip() for l in Path(path).read_text().splitlines() if l.startswith('play ')]
    except FileNotFoundError:
        return None
    return lines[0] if lines else None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--replay', default=str(ROOT / 'build/oracle-ref/replays/MissingNO_best_jj5_4.itr'))
    ap.add_argument('--data', default=str(ROOT / 'build/oracle-ref'))
    ap.add_argument('--speed', type=float, default=2.0)
    ap.add_argument('--quick', action='store_true', help='only a few configurations')
    a = ap.parse_args()
    exe = exe_path()
    tmp = Path(tempfile.mkdtemp(prefix='itower-rate-'))

    ref_trace = tmp / 'headless.txt'
    subprocess.run([str(exe), '-check', a.replay, '--sim-trace', str(ref_trace)], capture_output=True, timeout=300)
    ref = trace_line(ref_trace)
    if not ref:
        sys.exit('headless run produced no trace')
    ticks = int(ref.split()[1].split('=')[1])
    print(f'headless          : {ref}')

    configs = CONFIGS[:4] if a.quick else CONFIGS
    bad = 0
    for name, args in configs:
        t = tmp / (name.replace(' ', '_') + '.txt')
        ud = tmp / ('ud_' + name.replace(' ', '_'))
        ud.mkdir()
        # after the replay the game returns to the menu; stop a little later
        cmd = [str(exe), '--data', a.data, '--user-dir', str(ud), '--sim-trace', str(t),
               '--sim-speed', str(a.speed), '--exit-after-ticks', str(ticks + 300), *args, a.replay]
        try:
            subprocess.run(cmd, capture_output=True, timeout=ticks / 50 / a.speed + 90)
        except subprocess.TimeoutExpired:
            pass
        got = trace_line(t)
        ok = got == ref
        bad += not ok
        print(f'{name:18}: {"OK  " if ok else "DIFF"} {got}')
    print('render-rate independence:', 'PASS' if not bad else f'FAIL ({bad})')
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
