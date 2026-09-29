# Portable SDL3 build: testing

Screenshots alone never prove equivalence here. Equivalence is established at three levels:

- the compat layer against real Allegro;
- gameplay against the frozen reference build, compared through the historical replay checker;
- the simulation against every presentation setting.

`docs/port/BASELINE.md` explains the reference build `build/oracle-ref/icytower-ref.exe`.
It is an ordinary link of the frozen sources, never the original game. Game assets are not in
the repository; `build/oracle-ref` holds the reference build next to a copy of an installed
game's `data/` and `characters/`.

## Build

```
python tools/port/build.py                 # Release: build/port/release (MinGW + Ninja)
python tools/port/build.py --type Debug
```

SDL3 3.4.16 is fetched once, pinned by sha256, into `ITOWER_DEPS_DIR` (default `build/deps`).
`-DITOWER_SDL3_MODE=system` uses an installed SDL3 instead.

## 1. Compat layer vs Allegro 4.4.1 (`golden.py`)

```
python tools/port/golden.py --data build/oracle-ref
```

The same scenario programs are built twice: against real Allegro 4.4.1 with the historical
GCC 4.4 toolchain, and against `compat/allegro4-sdl3`. The two record streams are then
compared byte for byte. The scenarios cover:

- graphics: every blit, sprite, rotation, blender, primitive and clip case the game uses, over
  its colour depths;
- text;
- datafile, packfile and PNG loading;
- the software mixer's output samples.

Current result: `8555/8555 records identical`.

## 2. Unit tests

| test | checks |
|---|---|
| `test_x87` | the software x87 emulator against the real FPU. By default 57.6M cases with 0 mismatches; hosts without x87 check the recorded vectors instead (see `X87.md`) |
| `test_sound` | mixer, voices, `voice_get_position`, master gain (133 checks) |
| `test_logg` | Ogg Vorbis loading through the game's loader (23 checks) |
| `test_files` | user and asset roots, write redirection, directory listings |

Run each from `build/port/release/`. Each exits with status 0 on success.

## 3. Gameplay vs the frozen build (`replay_check.py`)

The historical game contains its own replay checker, `-check REPLAY -all`. It re-simulates
a replay headlessly and prints XML covering:

- claimed and actual score, floor and combo;
- every combo and jump sequence;
- key counts;
- the anti-cheat timing table.

The checker runs on both builds and the complete outputs are compared:

```
python tools/port/replay_check.py build/oracle-ref/replays build/port/synth-a build/port/synth-b \
    --jobs 12 --timeout 60 [--save DIR]
```

The corpus:

- **real replays** (13): recorded games covering best score/floor/combo, lost combos, no-combo
  runs, jump-sequence records, and a custom game;
- **synthetic replays**, from `mkreplay.py`, valid ITR140 files with generated input:
  ```
  python tools/port/mkreplay.py build/port/synth-a --count 150 --seed 11
  python tools/port/mkreplay.py build/port/synth-b --count 150 --seed 12 --custom
  ```
  The styles climb, run into walls, make large combos and die in many ways. `--custom`
  also randomises floor size, speed, gravity and rejump.

Current result: **310 identical, 0 different**. Three synthetic replays never end on either
build: the player keeps bouncing and the checker has no time limit. These are reported as
"endless on both", not as failures.

This check needs exact floating point. Plain SSE arithmetic failed 7 of the 13 real replays,
because the checksum folds in floating-point timings, and diverged on synthetic ones. The x87
emulation (`X87.md`) reproduces them all.

### Other platforms

The replay checker needs no window, so a console build can run it anywhere. On Linux (WSL
Ubuntu, gcc 15; SDL's console backend avoids needing X11/Wayland development packages):

```
cmake -S . -B build/port/linux -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DSDL_UNIX_CONSOLE_BUILD=ON
make -C build/port/linux -j icytower
./build/port/linux/icytower -check REPLAY -all > out.xml
```

Save the reference outputs with `replay_check.py --save DIR` on Windows and compare each file
from the `<itrcheck_results` element onwards. Line endings differ.

Result: **310/310 identical** on Linux x86-64. That build compiles without x87 code, so
this also checks the emulator end to end.

## 4. Render-rate independence (`render_rate_test.py`)

```
python tools/port/render_rate_test.py --data build/oracle-ref
```

This plays a real replay in the windowed build (replay playback from the menu, not the
checker), with `--sim-trace`. That option writes the tick count and an FNV-1a hash of the
gameplay state that feeds later steps, folded in after every 50 Hz step:

- player kinematics as exact bit patterns;
- collision state, rotation and edge;
- level, score and combo bookkeeping;
- the whole map (rows, floors, scroll offset);
- replay position.

Input and RNG divergence show up through these within a step.

The windowed run is compared with the same fingerprint from the headless checker, under
13 settings:

- modern and faithful renderer;
- vsync on and off;
- frame caps of 50, 60, 120, 144 and 240, and uncapped;
- interpolation on and off;
- windows of 640x480 (linear filter), 1280x720, 3440x1440 and 3840x2160.

Every setting must produce the same ticks and hash. Current result: PASS for all 13, with frames
per tick ranging from 1.0 (50 fps cap) to about 25 (uncapped).

`--sim-speed N` shortens runs by advancing virtual time N times faster. It changes how many
frames fall between ticks, which is exactly what the test varies. `ITOWER_TRACE_VERBOSE=1`
prints per-tick hashes to find the first divergent step.

## 5. End-to-end and visual checks

Scripted input and captures let a test drive the real game with no person present:

```
icytower.exe --data build/oracle-ref --user-dir build/port/ud --window 2560x1440 \
    --keys "3000+67,3100-67,..."  --capture build/port/cap --capture-ms 4000,9000 \
    --exit-after-ms 10000
```

- `--keys "MS+CODE,MS-CODE"` presses or releases an Allegro scancode at a wall-clock time.
- `--capture` saves the presented frame at the given times or ticks.
- `--exit-after-*` ends the run.
- `ITOWER_DEBUG_DL=1` logs display-list fallbacks, i.e. frames the modern renderer drew from
  pixels.

Resolutions checked this way:

- 640x480, 1280x720, 1920x1080, 2560x1440, 3440x1440 and 3840x2160;
- 1280x1024, 2560x1080, and a square 1080x1080;
- a window resized while running.

At each one, gameplay, menus, pause, game-over and results were checked for: HUD anchoring,
wall continuation, full-width overlays, the menu side fill, and seams.

## Before committing

1. `python tools/port/build.py`, which must build without new warnings in `port/`/`compat/`.
2. Unit tests and `golden.py`.
3. `replay_check.py` over the full corpus.
4. For scheduler or renderer changes: `render_rate_test.py`.
