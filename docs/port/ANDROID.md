# Android build

The Android app is the same portable SDL3 game: the repository's `CMakeLists.txt` builds it as
`libmain.so` for SDL's Android activity. `android/` holds a small Gradle project around it.

## Build and run

Requirements:

- Android SDK with platform 36, NDK 27.0.12077973 and CMake 3.22.1 (the SDK manager
  versions);
- a JDK 17 or 21 for Gradle 8.12;
- your own copy of the game data (see below).

```
python tools/port/android.py                        # debug APK, arm64-v8a + x86_64
python tools/port/android.py --abis x86_64 --install --run   # emulator
python tools/port/android.py --release --install    # phone (release, debug-signed)
python tools/port/android.py --no-build --logcat    # follow the game's log
```

The script finds the SDK (`ANDROID_HOME`, or `%LOCALAPPDATA%/Android/Sdk`) and a suitable
JDK, writes the git-ignored `android/local.properties`, and runs `gradlew`. The APK ends up
in `android/app/build/outputs/apk/<variant>/`.

## What goes into the APK

- **Native code.** `libSDL3.so` and `libmain.so`, built by the repository's own CMake with
  `-DITOWER_BUILD_TESTS=OFF`. SDL comes from the pinned, hash-checked tarball in
  `build/deps`, the same one the desktop build uses.
- **SDL's Java glue** (`org.libsdl.app.*`). It must match the native SDL exactly, so Gradle
  unpacks it from that same tarball rather than copying it into the repository. The version
  and hash are read from `cmake/FetchSDL3.cmake`.
- **Game data.** The git-ignored `assets/` folder (or `-PgameData=DIR`) supplies `data/`,
  `characters/` and `gamepad.txt`. They are packaged under `assets/gamedata/` with an
  `index.txt` (a content hash, then one line per file). APKs built this way contain the
  original game's files and are for personal use; they are never published.

On first start (and whenever the packaged data changes) `port/platform/plat_sdl.c` unpacks
the data into the app's private storage. That copy becomes the asset root, because the game
reads plain files and lists directories, which APK assets cannot do. Profiles, replays,
`tower.cfg` and `icytower-port.ini` live in the app's private files directory
(`SDL_GetPrefPath`).

## Platform behaviour

- **Display.** Landscape only. The default display mode is `borderless`: the app covers the
  whole screen and the modern renderer uses its full width. Display cutouts are part of the
  safe area the HUD respects.
- **Back button.** Acts as Esc: it pauses a game and shows the exit prompt, and goes back in
  menus.
- **Backgrounding.** SDL freezes the game thread while the app is in the background. On
  return the game pauses itself (a one-shot pause flag set by an SDL event watcher), so it
  never resumes mid-jump.
- **Saving.** Android can kill a backgrounded app without any exit path. So besides the
  game's own saves (after every game, and on exit), `tower.cfg` and the current profile are
  also written after creating or changing a profile and whenever a submenu such as Options
  closes (`port_save_state`).
- **Short presses.** Taps and injected key events can go down and up within one event pump.
  Every press stays down for at least 30 ms, longer than one 20 ms tick, so the game's
  key-state polling sees it.
- **Mouse cursor.** Never shown.

## Determinism on ARM

Two compiler settings keep ARM builds identical to x86 (`CMakeLists.txt`, all platforms):

- `-fsigned-char`: ARM's `char` is unsigned by default. Built that way, two of the real
  replays are rejected as invalid files (`build/port` experiment: 308/310).
- `-ffp-contract=off`: no fused multiply-add. An x86 build with forced FMA still matched
  310/310, because the software x87 arithmetic covers the rounding-sensitive gameplay code,
  so this is a safeguard.

Portability bugs found by the first Android run, fixed for every platform:

- `log2file` consumed one `va_list` twice. That works on Windows but reads garbage on
  x86-64 System V.
- `hisc.c` declared `malloc(unsigned int)` itself.
- The GFX menu read a bitmap pointer as a 32-bit word (fixed earlier).

## Status and next steps

1. Done: the project, CMake Android target, packaged data, back button, auto-pause, saving,
   and landscape fullscreen. Verified on the x86_64 emulator (API 36, 2340x1080) with
   keyboard input: first-run profile creation, the menus, gameplay, Back, and pausing when
   backgrounded. arm64-v8a builds and has not been run yet.
2. Touch controls: zones, slide and tilt schemes, selectable in the game's menus; tap
   latching; a touch remote for non-gameplay screens.
3. Tap-to-select menus and the soft keyboard for name entry.
4. Replay check on a real ARM phone (`-check` of all replays through adb) and polish.
