# Portable SDL3 build: architecture

This branch (`portable-sdl3`) turns the frozen matching reconstruction of Icy Tower 1.5.1
into a modern source port. Its rules come from the port brief:

- the game's rules, physics, RNG, replay semantics and 50 Hz timing are the frozen
  reconstruction's;
- platform and presentation machinery is replaced.

`docs/port/BASELINE.md` records the provenance. `docs/port/TESTING.md` describes how
behaviour is checked against the frozen build.

```
historical frozen reconstruction (main @ 70ece2a)
        |  linked with the historical GCC 4.4 + Allegro 4.4.1 toolchain
        v
reference oracle  build/oracle-ref/icytower-ref.exe   (-check REPLAY -all)
        |  same replays, same XML, same per-tick state
        v
portable SDL3 implementation (this branch)
```

## Layers

```
src/, include/               the 25 historical game units, adapted (see "Game sources")
compat/allegro4-sdl3/        Allegro 4.4.1 API subset the game uses (software bitmaps,
                             datafiles, fonts, packfiles, mixer, input/timer facades)
                             + display-list recording (a4_dl.*)
port/platform/               SDL3: lifetime, clock, events, file roots, message boxes,
                             audio device
port/input/                  keyboard/mouse/gamepad (hot-plug), logical actions,
                             game control flags, scripted input for tests
port/sim/                    fixed-step scheduler, render snapshots, x87 arithmetic
port/render/                 window/presenter, render pacing, faithful canvas path,
                             modern renderer (camera, world, HUD), display-list replay,
                             texture cache, captures
port/config/                 icytower-port.ini
port/game/                   glue the game units call: entry point, portable file
                             helpers, MSVCRT rand, Ogg loader, snapshot capture,
                             simulation fingerprint, ads stub
```

Nothing below `port/` or `compat/` is game-rule code, and no game unit calls SDL.
Only `port/platform`, `port/input` and `port/render` include SDL headers.

## Three coordinate domains

| domain | owner | unit | notes |
|---|---|---|---|
| game space | simulation (`play()`, `player.c`, `map.c`) | historical frame units | x = 0..639 around the tower axis x = 320, player clamped to 85..555. The 480-unit vertical view is a game rule: the map holds 32 rows of 16 units, and scrolling is triggered by the player's y. Unchanged; see `docs/port/COORDINATES.md`. |
| camera / view | `port/render/modern.c` (`make_view`) | game units -> pixels | Vertical field of view is always 480 units, s = output height / 480. The horizontal view grows with the aspect ratio (640 at 4:3, ~853 at 16:9, ~1147 at 21:9) around x = 320. With `widescreen = false`, or on outputs narrower than 4:3, it keeps 640 x 480 and letter- or pillar-boxes. |
| presentation | `port/render/present.c` | drawable pixels | Real output size (`SDL_GetCurrentRenderOutputSize`) including high-DPI density. Mouse coordinates are mapped back with `SDL_RenderCoordinatesFromWindow`. Game code never sees it. |

Outside the historical walls (x < -57 or x > 697) the original frame showed nothing. Wide
views continue the tower's own wall graphics outward, mirrored and dimmed. The legal play area
does not change.

### UI layout

The HUD is drawn from snapshot data and anchored to the visible area, which is inset by the
platform safe area (`SDL_GetWindowSafeArea`). Positions stay the historical frame coordinates,
interpreted relative to an anchor:

| element | historical position | anchor |
|---|---|---|
| clock, combo meter, combo number | (6,10), (22,100), (42,210) | top-left |
| score | (8,440) | bottom-left |
| REPLAY blink, custom-game captions | right edge 630 | top-right |
| replay VCR, scrolling title, progress | 635-w, 475-h | bottom-right |
| combo reward ("Good!", "Sweet!", ...) | centre (320, 360) | centre |

The hurry-up sign belongs to the world: it sits behind the floors on the tower axis.

`ui_scale` (auto = 1) scales HUD units relative to the world scale. At 4:3 with `ui_scale = 1`
every anchor coincides with its historical position.

Menus and dialogs keep the historical 4:3 design canvas. That canvas is resolution
independent: it is rendered from display lists (see below), centred, and on wide outputs the
screen's own background continues into the sides.

## Two render paths

Both paths consume the same game state. The software 640x480 frame is always produced as
before, so screenshots, read-backs and the faithful path stay exact.

**faithful** (`renderer = faithful`): the historical `screen` bitmap is uploaded once per
change and scaled to the window, aspect-correct, with nearest, linear or pixel-art sampling and
optional integer scaling. This is the regression reference for the presentation.

**modern** (`renderer = modern`, default) never scales a finished frame. It uses:

- **Gameplay world and HUD from snapshots.** `port/game/snapshot_capture.c` brackets every
  `draw_frame()` in `play()`. It copies the state `draw_frame` used and repeats its
  presentation decisions (pose, offsets, HUD shake) without mutating anything. It pushes a
  `game_snapshot`. `port/render/modern.c` draws floors, signs, stripes, walls, particles,
  player and HUD from the original bitmaps at output resolution, through the camera
  transform.
- **Canvas display lists for everything else.** Every compat drawing call that targets a
  canvas bitmap (`screen`, `swap_screen`, canvas-sized temporaries) also records an
  operation (`compat/allegro4-sdl3/src/a4_dl.c`): bitmap/sprite with flip, rotation and
  blend; fill; line; text; nested canvas; gameplay underlay.
  - Whole-canvas copies share lists, and partial copies nest.
  - The screen-shake self-copy shifts the list.
  - Opaque full-cover draws start a new list.
  - Screens that redraw onto an uncleared canvas every frame (the post-game menu's grid)
    would grow the list without bound. An op whose pixels are all overwritten later is
    therefore pruned: covered by an opaque op, redrawn identically, or faded below half a
    colour step by translucent fills. The final image is unchanged.
  - 8-bit sources are converted with the palette selected at draw time, as Allegro does,
    and the converted copy is recorded (the loading screen's logo uses its own palette).
  - Anything not representable (XOR, zooming a canvas, sub-bitmap drawing) invalidates the
    list, and that frame falls back to the canvas pixels, so correctness never depends on
    the list.
  - `port/render/dlrender.c` replays lists natively: sprites and glyphs are textures at
    their original resolution, placed by edge-rounded transforms so tiles do not seam.
- **Composition.** After the world is captured, `swap_screen`'s list is reset to an
  UNDERLAY marker. Overlays drawn afterwards in the same frame are therefore drawn natively
  on top of the modern world: the results panel, "Game Paused", the exit prompt, fades,
  shake and initials entry. Over gameplay on wide outputs, full-width overlay fills (dims,
  bands) span the whole output.

Textures (`port/render/texcache.c`) are uploaded once from the original bitmaps, in four
variants (opaque, mask-colour transparent, per-pixel alpha, silhouette). They are
re-uploaded only when a bitmap's `generation` changes. For linear filtering, transparent
texels take their neighbours' colours ("alpha bleeding"), so magenta never fringes. Sampling
is chosen per draw from `texture_filter`: `nearest`, `linear` or `pixelart` (SDL's
sharp-pixel mode). The filter is a single switch (`render_scale_mode`), so further modes, for
example shaders, plug in without touching game code.

## Time: fixed 50 Hz simulation, free rendering

The historical game ran one logic step per 20 ms tick of `install_int(cycle_counter, 20)` and
busy-waited on `cycle_count`. On this branch the main thread owns time
(`port/sim/sched.c`):

```
SDL events -> monotonic clock -> 20 ms accumulator -> 0..N ticks (exactly 50 Hz)
                                                   -> frames at display rate while waiting
```

- Every historical per-tick wait (18 sites: play, results, menus, dialogs, fades, credits,
  profile, replay and high-score screens) calls `port_wait_tick()`. That pumps events, renders
  frames with `alpha = accumulated time / 20 ms`, sleeps precisely, and returns once one
  20 ms step is available.
- After a stall the accumulator catches up by at most 5 ticks (100 ms). A gap of more than
  250 ms between waits is treated as a pause (a modal screen or loading) and resynchronises
  instead of replaying the gap.
- Nothing mutates game state from another thread. Allegro-style `install_int` callbacks
  (the fps counter, the legacy `cycle_count`) run on the main thread from the service loop.
- Frame pacing: `vsync`, `max_fps` (0 = uncapped, still limited to 1000 fps without vsync).
  Screens that do not animate redraw only when their canvas changes.

A tick computes the same thing regardless of the renderer, frame rate, vsync or
interpolation. `tools/port/render_rate_test.py` proves it: the per-tick simulation fingerprint
is identical headless and in 13 windowed settings.

## Interpolation

This is presentation only (`interpolation = on/off`). The renderer keeps the last two snapshots
and draws `lerp(previous, current, alpha)` for continuous quantities: the scroll offset
(which moves stripes, floors and walls rigidly), player position, particles, rotation angle,
hurry sign, combo meter, clock hand and reward scale.

Discrete state is snapped, not blended: sprite poses, flips, stripe and floor identities,
signs, shake, score, death, new games. Snapshots are marked discontinuous at every new game.
Values are only interpolated when both snapshots contain the same entity and the step is
plausible: a particle aged exactly one tick, an angle advanced by at most two ticks, the player
moved less than 40x64 units. Floors are matched through the scroll offset, including the row
that scrolled out during the tick, and stripe sets through their 256-unit period.

With interpolation off, the modern renderer uses the historical integer truncations, so at
50 Hz it shows the same positions as the faithful path. Snapshots are never read by the
simulation, the RNG, replays or scoring.

## Determinism: exact x87 arithmetic

The original ran with the x87 FPU at 64-bit precision (the MinGW CRT calls `_fpreset` =
`fninit`). Its gameplay code rounds to `double` or `float` only at stores, and compares and
chains unrounded 80-bit values. SSE2 or ARM arithmetic rounds differently. In practice this
rejected 7 of 13 real replays (the replay checksum folds float timing data) and diverged on
synthetic games.

`port/sim/x87.{h,c}` is an exact software 80-bit implementation: add, sub, mul, div, loads,
stores, `fistp` truncation, `fucom`, and the masked-exception results for infinities and NaN.
It uses integer arithmetic only, so it gives the same results on any CPU. Every gameplay
floating-point site follows the original machine code through it:

- `update_player` (integration, wall bounce, gravity);
- `handle_player_input` (0.7/0.3/0.9 acceleration);
- `line_intersect`, including its divide-by-zero NaN path;
- collision snapping, the scroll moves, the floor-width formula in `add_floor`;
- `calc_replay_checksum`;
- the cosmetic RNG `new_rand`.

Map generation uses the MSVCRT `rand()` LCG (`hist_rand`), because replays store only its
seed. See `docs/port/X87.md`.

## Configuration

`<user dir>/icytower-port.ini` is created on first run with documented defaults (below). It
holds port settings only. `tower.cfg`, profiles (`.itp`), high scores and replays (`.itr`)
keep their historical formats and are never reinterpreted.

```
[display] mode = windowed|fullscreen|borderless, width, height, resizable, vsync,
          max_fps, high_dpi
[render]  renderer = modern|faithful, widescreen, texture_filter = nearest|linear|pixelart,
          interpolation, ui_scale = auto|N, integer_scaling
[audio]   frequency, master_volume
```

- The game's own Fullscreen menu option is kept in sync with `[display] mode`.
- Command-line overrides: `--renderer`, `--window WxH`, `--fullscreen`, `--borderless`,
  `--windowed`, `--filter`, `--interpolation`, `--vsync`, `--max-fps`, `--widescreen`,
  `--ui-scale`, `--config`.

## Files and paths

The game keeps its historical relative paths (`data/data.dat`, `profiles/NAME/...`,
`tower.cfg`, `screenshots/...`). The platform layer resolves them against two roots:

- **user root** (`SDL_GetPrefPath`, or `--user-dir` / `ITOWER_USER_DIR`): writable, searched
  first;
- **asset roots**: `--data` / `ITOWER_DATA_DIR`, the executable's directory, then the current
  directory. Read-only.

Writes always go to the user root. Directory listings merge the roots. The game never depends
on the working directory being writable. Game assets are not part of the repository: point
`--data` at an installed Icy Tower 1.5 directory (`data/`, `characters/`).

## Input

`poll_control()` asks `port/input` for the historical `Tcontrol` flag byte, combining three
sources:

- keyboard: the player's stored bindings, in Allegro scancodes, which profiles store;
- SDL gamepads (hot-plug): the `gamepad.txt` mapping, plus Start = pause;
- virtual flags: the hook for touch controls.

Replays record flags once per simulation step, independent of device or refresh rate. Menus
and text entry still use the Allegro facades (`key[]`, `readkey`); those are fed by the same
SDL event stream.

## Audio

The compat mixer is a port of Allegro's own software mixer (voices, priorities, volume/pan
law, interpolation), driven by an SDL3 audio stream. `voice_get_position()` advances with
real playback, including for the silent copy of the music that `play()` uses for its speed
check. Ogg Vorbis samples are decoded by the vendored libvorbis through `port/game/logg_port.c`.
MIDI playback, used only by custom characters with `.mid` music, is not implemented:
`load_midi` returns NULL, which the game already handles.

## Game sources: what changed and why

All changes are platform or presentation machinery unless listed as a behaviour note.

- Includes, Winsock, pthreads, `ShellExecute`, `LoadCursor`, `LoadLibrary`, `chdir`, `getcwd`,
  `QueryPerformanceCounter`: replaced with portable equivalents (`port/game/port_game.h`).
- `fopen` and `mkdir` go through the file roots. `stricmp` is portable.
- 64-bit fixes: menu data pointer, the GFX menu's character bitmap (read as the 32-bit
  word at byte 8), profile `time_t`, layout asserts of pointer-bearing
  in-memory structs (serialised structs keep strict asserts), `key` parameter names.
- `game_data.c`: overlapping `sprintf` (undefined behaviour) replaced by appends, and buffers
  sized for the 5000-entry tables.
- The advertising module (`fld_adspot`, `httpget`, `csv`, `strptime`, `timecompat`) is
  replaced by `port/game/ads_stub.c`, because the servers are gone and a port should not
  phone home.
- `loadpng.c` and `savepng.c` use SDL3's PNG codec. libpng gamma correction is not reproduced
  (the game's images carry none). 32-bit screenshots are written opaque; the original
  saved a zero alpha channel.
- Timing: waits go through the scheduler, and the per-tick `rest(2)` of the play loop is gone.
- Presentation hooks: snapshot capture around `draw_frame`, and the simulation fingerprint.
- Behaviour notes (all outside the simulation, or matching the replay checker):
  - replay playback starts from "no input" on its first step, as `-check` does; the original
    used whatever keys were held;
  - the player struct is zero-initialised (its `angle` was heap garbage, drawing only);
  - the first-run profile dialog, the pause-key pause screen and the key-redefinition prompt
    end when the window is closed. The original ignored the close button there until a
    key was pressed.

## Remaining legacy limitations

- The historical code structure remains: `main.c` is still one large unit of blocking loops,
  and the scheduler and display lists adapt it rather than restructure it. Further extraction
  (menus as a state machine, `play()` split into update and draw) is possible, and the tests
  make it safe to do step by step.
- Some menu content is rendered into intermediate bitmaps that are not canvas-sized
  (high-score tables, the profile viewer). In modern mode these are drawn as scaled pixels,
  not natively.
- The faithful path reproduces the historical frame. The modern path renders sprites with GPU
  rotation and scaling, so rotated sprites (clock hand, reward, rotating jumps) are smooth
  rather than Allegro's nearest-neighbour rotation.
- Android: the platform, input (virtual flags), file roots (SDL asset I/O would add an asset
  root) and renderer are ready, but no Gradle project or touch overlay exists yet.
- A windowed Linux build needs SDL3's X11/Wayland development packages. The code builds and
  passes the replay tests on Linux with SDL's console backend (see TESTING.md).
