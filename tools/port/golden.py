#!/usr/bin/env python3
"""Golden-image comparison: real Allegro 4.4.1 vs compat/allegro4-sdl3.

Builds tests/port/golden/*.c twice:
  * against real Allegro 4.4.1 with the historical GCC 4.4 toolchain
    (build/local/toolchain, liballeg.a from `python tools/link.py
    --libraries-only`), output build/port/golden/a4_golden_allegro.exe;
  * against the compat layer (CMake target a4_golden_compat).
Runs both on the same asset directory and compares the record streams.

    python tools/port/golden.py --data DIR [--area gfx|text|data|sound] [--dump-diffs]

DIR must contain the game's data/ directory (and characters/ for some
scenarios).  Nothing is executed from the original game.
"""
import argparse, os, struct, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build' / 'port' / 'golden'
TC = ROOT / 'build' / 'local' / 'toolchain'


def historical_libs():
    r = subprocess.run([sys.executable, str(ROOT / 'tools' / 'link.py'), '--libraries-only'],
                       cwd=ROOT, capture_output=True, text=True, check=True)
    return Path(r.stdout.strip().splitlines()[-1])


def build_allegro_runner():
    OUT.mkdir(parents=True, exist_ok=True)
    lib = historical_libs()
    srcs = sorted(p for p in (ROOT / 'tests/port/golden').glob('*.c') if p.name != 'main_compat.c')
    exe = OUT / 'a4_golden_allegro.exe'
    cmd = [str(TC / 'bin/gcc.exe'), '-O2', '-mfpmath=387', '-DALLEGRO_STATICLINK', '-DALLEGRO_NO_MAGIC_MAIN', '-mconsole',
           '-I' + str(ROOT / 'include'), '-I' + str(ROOT / 'third_party/allegro-4.4.1/include'),
           '-I' + str(ROOT / 'tests/port/golden'),
           *map(str, srcs), str(lib / 'liballeg.a'),
           '-lkernel32', '-luser32', '-lgdi32', '-lcomdlg32', '-lole32', '-ldinput', '-lddraw', '-ldxguid',
           '-lwinmm', '-ldsound', '-o', str(exe)]
    env = dict(os.environ)
    env['PATH'] = str(TC / 'bin') + os.pathsep + env.get('PATH', '')
    subprocess.run(cmd, check=True, env=env)
    return exe


def build_compat_runner():
    subprocess.run([sys.executable, str(ROOT / 'tools/port/build.py'), '--target', 'a4_golden_compat'], check=True)
    exe = ROOT / 'build/port/release/a4_golden_compat.exe'
    if not exe.exists():
        exe = ROOT / 'build/port/release/a4_golden_compat'
    return exe


def read_records(path):
    data = path.read_bytes()
    i, recs = 0, []
    while i < len(data):
        z = data.index(b'\0', i)
        name = data[i:z].decode('latin-1'); i = z + 1
        kind, = struct.unpack_from('<i', data, i); i += 4
        if kind == 1:
            w, h, d, n = struct.unpack_from('<iiii', data, i); i += 16
            recs.append((name, kind, (w, h, d), data[i:i + n])); i += n
        elif kind == 2:
            n, = struct.unpack_from('<i', data, i); i += 4
            recs.append((name, kind, None, data[i:i + n])); i += n
        elif kind == 3:
            v, = struct.unpack_from('<i', data, i); i += 4
            recs.append((name, kind, None, v))
        else:
            raise ValueError(f'bad record kind {kind} at {i}')
    return recs


def compare(a, b, dump):
    bad = 0
    names_a = [r[0] for r in a]
    names_b = [r[0] for r in b]
    if names_a != names_b:
        print('record lists differ:')
        print('  allegro only:', sorted(set(names_a) - set(names_b)))
        print('  compat only :', sorted(set(names_b) - set(names_a)))
    bmap = {r[0]: r for r in b}
    for ra in a:
        rb = bmap.get(ra[0])
        if rb is None:
            continue
        name, kind = ra[0], ra[1]
        if ra[2] != rb[2] or ra[1] != rb[1]:
            print(f'MISMATCH {name}: shape allegro={ra[2]} compat={rb[2]}'); bad += 1; continue
        if ra[3] == rb[3]:
            continue
        bad += 1
        if kind == 1:
            w, h, d = ra[2]
            bpp = 1 if d == 8 else 2 if d <= 16 else 3 if d == 24 else 4
            diffs = [k for k in range(0, len(ra[3]), bpp) if ra[3][k:k + bpp] != rb[3][k:k + bpp]]
            k = diffs[0]
            print(f'MISMATCH {name}: {len(diffs)} of {w*h} pixels differ; first at '
                  f'({(k//bpp) % w},{(k//bpp)//w}) allegro={ra[3][k:k+bpp].hex()} compat={rb[3][k:k+bpp].hex()}')
            if dump:
                OUT.mkdir(parents=True, exist_ok=True)
                for tag, rec in (('allegro', ra), ('compat', rb)):
                    (OUT / f'{name}.{tag}.raw').write_bytes(rec[3])
        elif kind == 2:
            n = min(len(ra[3]), len(rb[3]))
            first = next((k for k in range(n) if ra[3][k] != rb[3][k]), n)
            print(f'MISMATCH {name}: bytes len {len(ra[3])}/{len(rb[3])}, first diff at {first}')
        else:
            print(f'MISMATCH {name}: allegro={ra[3]} compat={rb[3]}')
    ok = len(a) - bad
    print(f'{ok}/{len(a)} records identical')
    return bad == 0 and names_a == names_b


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--data', required=True)
    ap.add_argument('--area', default=None)
    ap.add_argument('--dump-diffs', action='store_true')
    ap.add_argument('--no-build', action='store_true')
    a = ap.parse_args()
    if a.no_build:
        ea, ec = OUT / 'a4_golden_allegro.exe', ROOT / 'build/port/release/a4_golden_compat.exe'
    else:
        ea, ec = build_allegro_runner(), build_compat_runner()
    OUT.mkdir(parents=True, exist_ok=True)
    fa, fc = OUT / 'allegro.bin', OUT / 'compat.bin'
    extra = [a.area] if a.area else []
    subprocess.run([str(ea), a.data, str(fa), *extra], check=True)
    subprocess.run([str(ec), a.data, str(fc), *extra], check=True)
    sys.exit(0 if compare(read_records(fa), read_records(fc), a.dump_diffs) else 1)


if __name__ == '__main__':
    main()
