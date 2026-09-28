#!/usr/bin/env python3
"""Configure and build the portable SDL3 game with CMake + Ninja.

Finds cmake/ninja on PATH or in a Visual Studio installation, and on Windows
prefers the MSYS2 MinGW-w64 GCC toolchain.  Output goes to build/port/<type>.

    python tools/port/build.py                 # Release build
    python tools/port/build.py --type Debug
    python tools/port/build.py --clean
    python tools/port/build.py --target icytower_tests
"""
import argparse, glob, os, shutil, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def find_tool(name, extra_globs):
    exe = shutil.which(name)
    if exe:
        return exe
    for pattern in extra_globs:
        hits = sorted(glob.glob(pattern), reverse=True)
        if hits:
            return hits[0]
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--type', default='Release', choices=['Release', 'Debug', 'RelWithDebInfo'])
    ap.add_argument('--clean', action='store_true')
    ap.add_argument('--target', default=None)
    ap.add_argument('--cmake-arg', action='append', default=[])
    ap.add_argument('--dir', default=None, help='build directory name under build/port (default: build type)')
    a = ap.parse_args()

    vs = r'C:\Program Files\Microsoft Visual Studio\*\*\Common7\IDE\CommonExtensions\Microsoft\CMake'
    cmake = find_tool('cmake', [vs + r'\CMake\bin\cmake.exe'])
    ninja = find_tool('ninja', [vs + r'\Ninja\ninja.exe'])
    if not cmake or not ninja:
        sys.exit('cmake and ninja are required (PATH or Visual Studio CMake component)')
    env = dict(os.environ)
    if os.name == 'nt':
        mingw = Path(r'C:\msys64\mingw64\bin')
        if not shutil.which('gcc') and mingw.exists():
            env['PATH'] = str(mingw) + os.pathsep + env['PATH']
        env['PATH'] = str(Path(ninja).parent) + os.pathsep + env['PATH']
    out = ROOT / 'build' / 'port' / (a.dir or a.type.lower())
    if a.clean and out.exists():
        shutil.rmtree(out)
    if not (out / 'build.ninja').exists():
        cfg = [cmake, '-S', str(ROOT), '-B', str(out), '-G', 'Ninja',
               f'-DCMAKE_BUILD_TYPE={a.type}', f'-DCMAKE_MAKE_PROGRAM={ninja}']
        if os.name == 'nt':
            cfg += ['-DCMAKE_C_COMPILER=gcc']
        subprocess.run(cfg + a.cmake_arg, check=True, env=env)
    cmd = [cmake, '--build', str(out)]
    if a.target:
        cmd += ['--target', a.target]
    subprocess.run(cmd, check=True, env=env)
    print('build output:', out)


if __name__ == '__main__':
    main()
