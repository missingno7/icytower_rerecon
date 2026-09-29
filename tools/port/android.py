"""Build, install and run the Android version of the portable SDL3 game.

    python tools/port/android.py                 # debug APK (arm64-v8a + x86_64)
    python tools/port/android.py --abis x86_64   # emulator only (faster)
    python tools/port/android.py --release       # release APK, debug-signed
    python tools/port/android.py --install --run # adb install -r, then start it
    python tools/port/android.py --logcat        # follow the game's log output

The APK contains the local, git-ignored game data (assets/ by default,
--game-data DIR otherwise); it is for personal use and never published.

Environment: the Android SDK (ANDROID_HOME, default %LOCALAPPDATA%/Android/Sdk)
with NDK 27 and CMake 3.22.1, and a JDK 17 or 21 for Gradle 8.12 (JAVA_HOME
is used if it points to one; otherwise a JDK 17/21 is searched for).
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ANDROID = ROOT / 'android'
PACKAGE = 'io.github.icytowerport'
ACTIVITY = f'{PACKAGE}/.IcyTowerActivity'


def find_sdk():
    for v in (os.environ.get('ANDROID_HOME'), os.environ.get('ANDROID_SDK_ROOT'),
              os.path.join(os.environ.get('LOCALAPPDATA', ''), 'Android', 'Sdk'),
              os.path.expanduser('~/Android/Sdk')):
        if v and Path(v, 'platform-tools').exists():
            return Path(v)
    sys.exit('Android SDK not found (set ANDROID_HOME)')


def jdk_major(home):
    rel = Path(home, 'release')
    if rel.exists():
        m = re.search(r'JAVA_VERSION="(\d+)', rel.read_text(errors='replace'))
        if m:
            return int(m.group(1))
    return 0


def find_jdk():
    """Gradle 8.12 runs on JDK 17..23."""
    cur = os.environ.get('JAVA_HOME')
    if cur and 17 <= jdk_major(cur) <= 23:
        return cur
    roots = [Path(r'C:\Program Files\Java'), Path(r'C:\Program Files\Eclipse Adoptium'),
             Path(r'C:\Program Files\Microsoft'), Path(r'C:\Program Files\Android\Android Studio\jbr'),
             Path('/usr/lib/jvm')]
    found = []
    for r in roots:
        if (r / 'release').exists():
            found.append(r)
        elif r.exists():
            found += [d for d in r.iterdir() if (d / 'release').exists()]
    good = sorted((jdk_major(d), str(d)) for d in found if 17 <= jdk_major(d) <= 23)
    if not good:
        sys.exit('no JDK 17..23 found for Gradle 8.12 (set JAVA_HOME)')
    return good[-1][1]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--release', action='store_true')
    ap.add_argument('--abis', default=None, help='comma list, e.g. x86_64 or arm64-v8a')
    ap.add_argument('--game-data', default=None)
    ap.add_argument('--no-build', action='store_true')
    ap.add_argument('--install', action='store_true')
    ap.add_argument('--run', action='store_true')
    ap.add_argument('--logcat', action='store_true')
    ap.add_argument('--serial', default=None, help='adb device serial')
    a = ap.parse_args()

    sdk = find_sdk()
    env = dict(os.environ)
    env['JAVA_HOME'] = find_jdk()
    env['ANDROID_HOME'] = str(sdk)
    # local.properties (git-ignored) tells Gradle where the SDK is
    (ANDROID / 'local.properties').write_text('sdk.dir=' + str(sdk).replace('\\', '/') + '\n')

    variant = 'Release' if a.release else 'Debug'
    if not a.no_build:
        gradlew = str(ANDROID / ('gradlew.bat' if os.name == 'nt' else 'gradlew'))
        cmd = [gradlew, f'assemble{variant}', '--console=plain']
        if a.abis:
            cmd.append(f'-Pabis={a.abis}')
        if a.game_data:
            cmd.append(f'-PgameData={Path(a.game_data).resolve()}')
        subprocess.run(cmd, cwd=ANDROID, env=env, check=True)
    apk = ANDROID / 'app' / 'build' / 'outputs' / 'apk' / variant.lower() / f'app-{variant.lower()}.apk'
    print('apk:', apk)

    adb = [str(sdk / 'platform-tools' / 'adb')] + (['-s', a.serial] if a.serial else [])
    if a.install:
        subprocess.run(adb + ['install', '-r', str(apk)], check=True)
    if a.run:
        subprocess.run(adb + ['shell', 'am', 'start', '-n', ACTIVITY], check=True)
    if a.logcat:
        subprocess.run(adb + ['logcat', '-s', 'SDL', 'SDL/APP', 'SDL/ERROR', 'libc', 'DEBUG', 'AndroidRuntime'])


if __name__ == '__main__':
    main()
