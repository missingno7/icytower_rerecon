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
- **Game data, personal builds only.** The git-ignored `assets/` folder (or
  `-PgameData=DIR`) supplies `data/`, `characters/` and `gamepad.txt`. They are packaged
  under `assets/gamedata/` with an `index.txt` (a content hash, then one line per file).
  APKs built this way contain the original game's files and are never published.

**Distributable builds** (`android.py --release --no-game-data`, Gradle `-PnoGameData`)
contain no game files. The player copies the `data` and `characters` folders of their own
Icy Tower 1.5 installation into the app's external files folder:
`Android/data/io.github.icytowerport/files/`, which a PC can reach over USB. Without the
data, the app shows exactly that instruction and exits.

Asset roots, in order: that external folder, then packaged data. On first start (and
whenever the packaged data changes) `port/platform/plat_sdl.c` unpacks packaged data into
the app's private storage. That copy becomes the asset root, because the game
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
- **Soft keyboard.** While a name is being typed and the on-screen keyboard is up, the frame
  is drawn lifted so the text field stays visible above the keyboard (presentation only).
- **Short presses.** Taps and injected key events can go down and up within one event pump.
  Every press stays down for at least 30 ms, longer than one 20 ms tick, so the game's
  key-state polling sees it.
- **Mouse cursor.** Never shown.

## Touch controls

`port/input/touch.c` feeds the game's own control flags: left, right, jump (fire) and pause.
Replays and the simulation therefore cannot tell touch from a keyboard. Every press lasts at
least one 20 ms tick, so short taps are never lost.

During gameplay one of three schemes is active. You choose it in the game, under
*Options > Controls > Touch* (Android builds only), or with `[input] touch_scheme` in
`icytower-port.ini`:

| scheme | left side of the screen | right side |
|---|---|---|
| **Zones** (default) | outer half = run left, inner half = run right; rocking the thumb across the boundary switches direction | jump |
| **Slide** | drag left or right from where the thumb touched down; the anchor trails the thumb, so reversing needs only a short move | jump |
| **Tilt** | tilt the device to run (about 9 degrees, with hysteresis); tap anywhere to jump | |
| **Off** | keyboard or gamepad only | |

The controls sit in the widescreen side areas, so they don't cover the tower. A pause button
sits top-right, and `touch_opacity` / `touch_left_handed` adjust the overlay.

Outside gameplay (menus, results, pause and exit prompts, replays) a small remote replaces
the game controls:

- ▲ ▼ ◀ ▶ arrows, OK (Enter; Space while watching a replay) and Back (Esc).
- They are sent as key presses, so every historical screen works as with a keyboard.
- The mode switches automatically: gameplay mode is on while gameplay frames are being
  captured.

**Text input.** While the game reads a typed string (profile name, replay name and comment),
the soft keyboard is shown (`input_text_input`). Printable characters then come from text
events and key events only carry key state, so hardware keys are not doubled.

Found on the way, fixed for every platform: Shift was put into the key buffer. Allegro never
buffers modifier keys, and `get_string` stored the buffered Shift as a NUL character, which
cut off every name typed with a capital letter.

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
2. Done: touch controls (zones, slide, tilt, off) selectable in the game's menus, tap
   latching, the touch remote for other screens, and the soft keyboard for name entry.
   All verified on the emulator, tilt with the emulated accelerometer.
3. Tapping menu items directly, a touch-friendly replay browser, and overlay polish
   (size and position settings in the game).
4. Replay check on a real ARM phone (`-check` of all replays through adb) and polish.
