# Icy Tower 1.5.1: portable SDL3 build

Branch `portable-sdl3` is a modern source port of the frozen matching reconstruction
(`main` @ `70ece2a`). The game's rules, physics, RNG, replays and 50 Hz timing are the
reconstruction's. Replays are verified identical to the frozen build.

Platform, rendering, input, audio and files are new:

- SDL3 with no Allegro runtime;
- resizable, high-DPI, true widescreen output;
- rendering decoupled from the fixed 50 Hz simulation, with optional interpolation;
- gamepads;
- per-user writable paths.

| document | contents |
|---|---|
| [docs/port/ARCHITECTURE.md](docs/port/ARCHITECTURE.md) | layers, coordinate domains, renderers, scheduler, interpolation, x87 determinism, config, what changed in the game sources, remaining legacy limitations |
| [docs/port/TESTING.md](docs/port/TESTING.md) | how equivalence is established: Allegro golden tests, replay regression against the frozen build, render-rate independence, Linux |
| [docs/port/COORDINATES.md](docs/port/COORDINATES.md) | classification of every 640/480/320/240/SCREEN_W/H/screen/swap_screen use |
| [docs/port/X87.md](docs/port/X87.md) | the software 80-bit arithmetic that makes gameplay bit-exact on every CPU |
| [docs/port/BASELINE.md](docs/port/BASELINE.md) | provenance and the reference build |

This branch makes no matching claims. `recovery.json`, `FREEZE.md` and `tools/verify.py`
describe the frozen commit, not these sources.

## Build

Requirements:

- CMake 3.24+ and Ninja;
- a C compiler (MinGW-w64 gcc 12 tested on Windows, gcc 15 on Linux);
- Python 3.10+ for the helper scripts.

SDL3 3.4.16 is downloaded once, pinned by sha256; the vendored libogg/libvorbis are in
`port/xiph`.

```
python tools/port/build.py                  # -> build/port/release/icytower(.exe)
```

Or use CMake directly (`-DITOWER_SDL3_MODE=system` uses an installed SDL3):

```
cmake -S . -B build/port/release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/port/release
```

A windowed Linux build needs SDL3's usual X11/Wayland development packages. With
`-DSDL_UNIX_CONSOLE_BUILD=ON` it builds without them, for the headless replay checker.

## Run

Game data is not tracked. Point the port at an installed Icy Tower 1.5 directory (the one
containing `data/` and `characters/`). In a working copy of this repository the installed
game lives in the ignored `assets/` folder:

```
icytower --data assets
```

`ITOWER_DATA_DIR` does the same. Alternatively, put the executable next to the data.
Profiles, replays, high scores, screenshots, `tower.cfg` and the port settings go to the
per-user directory: `%APPDATA%/IcyTowerPort/IcyTower` on Windows,
`~/.local/share/IcyTowerPort/IcyTower` on Linux; `--user-dir` overrides it.

### Settings

`icytower-port.ini` in the user directory is created on first run and documents itself:

```
[display]  mode = windowed | fullscreen | borderless
           width, height, resizable, vsync, max_fps (0 = uncapped), high_dpi
[render]   renderer = modern | faithful      modern: native resolution-independent
                                             faithful: the historical 640x480 frame, scaled
           widescreen = on | off             off: 4:3 view with bars
           texture_filter = nearest | linear | pixelart
           interpolation = on | off          smooth motion above 50 fps (presentation only)
           ui_scale = auto | N, integer_scaling (faithful)
[audio]    frequency, master_volume
```

Each setting has a command-line override: `--renderer`, `--window 1920x1080`,
`--fullscreen`, `--borderless`, `--windowed`, `--vsync`, `--max-fps`, `--filter`,
`--interpolation`, `--widescreen`, `--ui-scale`, `--config FILE`.

The historical options still work: `-check REPLAY [-all]` runs the headless replay checker,
and the in-game options menu is available (its Fullscreen entry follows `[display] mode`).

Controls are the game's own configurable keys. SDL gamepads also work and can be connected
at any time; Start pauses.
