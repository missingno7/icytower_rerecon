# Coordinate and screen-size inventory

Analysis report for the SDL3 port. It records every significant use of the historical
640x480 frame geometry in the 25 game units (`src/*.c`; `include/` has no hits) and assigns
each use to one coordinate domain. It is the input for a later, reviewed refactor that
names constants where the meaning is proven. No source file was changed to produce it.

Raw occurrences can be regenerated with the standard-library scanner:

```
python tools/port/coord_scan.py            # file, line, token, count, code (TSV)
python tools/port/coord_scan.py --summary  # hits per file and token
python tools/port/coord_scan.py --tokens 565,464,85,555
```

Default tokens: `640 639 480 479 320 240 SCREEN_W SCREEN_H screen swap_screen gfx_driver`
(whole-word matches, comments and strings included). Related layout constants (565, -57, 37,
464, 85/555, 540, 900, 160..0, 630/635/475/623, ...) were added by hand where they explain
an expression. Line numbers refer to the sources as of commit `70ece2a`, whose `src/` and
`include/` are byte-identical to `portable-sdl3` (`b431415`).

## 1. Purpose and categories

The port separates three domains that coincide in the 2008 build:

1. **Game space**: the historical gameplay coordinates. They must not change.
2. **Camera/view**: the part of the world that is visible and where the world origin lands in
   the frame. In widescreen more world is visible horizontally.
3. **Physical presentation**: the window's real pixels.

| code | category | meaning |
|---|---|---|
| `GAME` | GAME SEMANTIC | affects simulation, collision, physics, scoring or replay |
| `CAMERA` | CAMERA | decides which part of the world is visible, or where the world origin is in the frame |
| `WORLD` | WORLD DRAW | draws world content at world coordinates (floors, walls, stripes, player, particles, signs, positional audio) |
| `UI` | UI LAYOUT | HUD, menu, dialog and text positions |
| `PHYS` | PHYSICAL DISPLAY | assumes the physical screen or back-buffer size or identity: video mode, `screen` access, back-buffer lifetime, presentation, clears, clip reset, screenshots |
| `TEMP` | TEMPORARY / EFFECT | full-frame effects and scratch buffers: fades, dims, stripe overlays, shake, snapshot copies, the head-shadow buffer |
| `UNKNOWN` | UNKNOWN | unclear; the entry says what would resolve it |
| `text` | (not counted) | the token appears only in a log string or comment |

Classification rules used throughout:

- A row is classified by the meaning of the coordinate literal(s) on it. The render target
  (`screen` or `swap_screen`) goes in the *Target* column, so a UI text drawn straight onto
  `screen` is `UI` with target `screen`. Section 5 lists every direct-to-`screen` use.
- A bare buffer reference with no geometry (`draw_frame(swap_screen)`,
  `blit_to_screen(swap_screen)`, `fadeIn(swap_screen,16)`, `take_screenshot(swap_screen)`) is
  `PHYS`: it selects the back buffer or drives presentation.
- A full-frame dim or fade on the back buffer is `TEMP`. The same operation on `screen` is
  `PHYS`. A read-back from `screen` is `PHYS`.
- Target values: `swap` = `swap_screen` (or a `bmp` parameter that is always `swap_screen`),
  `screen` = the physical display bitmap, `tmp` = a scratch bitmap, `-` = no drawing.

## 2. Summary

Raw token hits from `coord_scan.py --summary` (whole-word, including comments and strings):

| file | 640 | 639 | 480 | 479 | 320 | 240 | SCREEN_W | SCREEN_H | screen | swap_screen | gfx_driver | total |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| src/main.c | 23 | 15 | 27 | 12 | 34 | 7 | 11 | 12 | 53 | 151 | 7 | 352 |
| src/profile.c | 5 | 0 | 9 | 0 | 0 | 0 | 5 | 5 | 2 | 25 | 0 | 51 |
| src/hisc.c | 0 | 0 | 1 | 0 | 0 | 0 | 4 | 4 | 1 | 13 | 0 | 23 |
| src/replay.c | 2 | 0 | 2 | 0 | 0 | 0 | 4 | 4 | 1 | 9 | 0 | 22 |
| src/custom.c | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 2 |
| src/loadpng.c | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 |
| **total** | 30 | 15 | 39 | 12 | 34 | 7 | 24 | 25 | 60 | 198 | 7 | **451** |

No other unit and no header under `include/` (excluding the pinned Allegro headers)
contains any of these tokens. `map.c` and `player.c` hold the game-space constants, which
section 6 lists as related rows.

Classified rows in section 6. One row covers one line, or a tight group of identical uses;
`text` rows are excluded:

| file | GAME | CAMERA | WORLD | UI | PHYS | TEMP | UNKNOWN | rows | text (excluded) |
|---|---|---|---|---|---|---|---|---|---|
| src/main.c | 8 | 2 | 16 | 92 | 53 | 21 | 0 | 192 | 3 |
| src/map.c | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 8 | 0 |
| src/player.c | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 |
| src/menu.c | 0 | 0 | 0 | 1 | 3 | 0 | 0 | 4 | 0 |
| src/scroller.c | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 0 |
| src/hisc.c | 0 | 0 | 0 | 7 | 4 | 5 | 0 | 16 | 0 |
| src/profile.c | 0 | 0 | 0 | 14 | 7 | 12 | 0 | 33 | 0 |
| src/replay.c | 0 | 0 | 0 | 6 | 4 | 5 | 0 | 15 | 0 |
| src/custom.c | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 3 | 0 |
| src/loadpng.c | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| **total** | **19** | **2** | **16** | **121** | **74** | **43** | **0** | **275** | 4 |

No occurrence stayed `UNKNOWN`. What remains uncertain is asset sizes (for example the wall
sprite width), not the meaning of any source expression; section 7 lists those items. Both
`CAMERA` rows are debug-only zoom modes: shipping code has no camera other than the scroll.
The `GAME` rows are all *(rel)* constants. No scanner token (640/480/320/..., `SCREEN_*`,
`screen`) has game-semantic meaning anywhere.

## 3. Key findings

### 3.1 The gameplay coordinate system *is* the historical frame

- **x axis.** World x equals frame x. There is no horizontal camera: nothing in the
  game ever offsets world x before drawing. The tower is laid out symmetrically about
  x = 320:
  - Player centre is clamped to **85..555** (`player.c:72-83`). With the feet at
    **±11** (collision rays, `main.c:3565ff`, `is_solid(x∓11)`), the feet span **74..566**.
  - The right wall sprite `data[100]` is drawn at **x = 565**, and a horizontally flipped copy
    at **x = -57** (`main.c:3283`). If the sprite is 132 px wide, the inner faces are at
    **75** and **565** (= 640 - 75), which fits the feet limits 74/566. Only the
    positions are in the source; the width is inferred (see open question Q1).
  - The background stripes `data[1..5]` are blitted at **x = 37** (`main.c:3114`). They are
    symmetric about 320 if 566 px wide (37..603). Unverified.
  - Floors use 16-px tiles, `tile*16` (`main.c:3136-3142`). Random floors occupy tiles
    **5..34** (`map.c:60`: `start = rand()%(30-width)+5`), so drawn x starts at 80-5 = 75, the
    wall face. Full-width floors, every 50th and 500th floor, use tiles **0..40**
    (`map.c:35,41`), drawn from x = -5 to beyond 640. Their collision extent is
    `start*16-2 .. (end+1)*16+1` (`map.c:98-99`).
  - Stereo pan normalises world x by **640** (`main.c:1958`). That is the world width, not a
    display width.
- **y axis.** y grows downward and is **view-relative world y**. Scrolling is simulation
  state: every scroll step adds the same amount to `map.offset` and to `ply->y`
  (`main.c:4288-4291`, `4303-4313`). The vertical camera is therefore part of the game
  state and cannot be separated from it.
  - Map rows: `map.room[r]`, r = 0..31. The floor surface of row r is at
    `y = 464 - 16*r + map.offset%16` (drawing, `main.c:3131`, minus 6 for the sprite lip).
    Collision uses the same geometry (`map.c:77-100`: `row = 29 - ((cy+1)>>4)`,
    `fy = ((cy+1)>>4)*16 + offset%16`). **464 = 29*16**, and 29 = 480/16 - 1. Rows 0..29
    cover the visible 0..479 and rows 30..31 sit above the view at -16 and -32. New floors
    enter at `room[31]` (`map.c:27-28`).
  - **The 480-px view height is part of the game rules.** The map holds exactly the view
    plus two rows. Showing more world vertically would show rows that are not generated,
    and changing that changes the simulation.
  - Scroll triggers on the player's y: below 160 → +1, then additional steps at 140, 120,
    100, 80, 60, 40 (+2), 20 (+2) and 0 (+3) (`main.c:4278-4291`). Auto-scroll begins once
    `map.offset > 100` (`4297`). A new floor is added when `offset%16` wraps or
    `tot_scroll > 15` (`4342-4349`).
  - Death when `y > 540`, i.e. 60 px below the view (`4535`). Game-over voice when `y > 900`
    (`4569`). y is clamped to 1000 (`player.c:70`). The start pose is (200, 431),
    standing on the floor surface at 432 (`main.c:3517-3518`).
- **`map.offset`** is the total scrolled distance. Its presentation phases are:
  stripes `offset%256/2` (1/2 speed) and stripe variety `offset/256`; walls
  `(offset%84)*1.476` (about 1.48x speed); floors `offset%16` (1:1). The idle "look down"
  pose applies when `offset > 200 && y > 400` (`main.c:3213`).
- **No simulation code reads the display size.** `SCREEN_W/SCREEN_H/gfx_driver/screen`
  never appear in logic. Every `SCREEN_*` use is a fade, a dim, a snapshot or the menu
  ad mouse test. The replay checker path (`itrcheck`) skips `draw_frame`, particles,
  fades and presentation entirely, which demonstrates the separation.
- **RNG separation.** The simulation RNG is the C library `rand()` in `map.c`, seeded with
  the replay's `random_seed`. The presentation RNG is `new_rand()`: stripes, particles,
  shake, sound pitch, hint text and the menu face. A widescreen renderer may never call
  `rand()`. It should also avoid extra `new_rand()` calls, because they would shift all
  later cosmetic randomness and break golden-image comparisons, although not replays.

### 3.2 Camera

- Horizontal: identity. Vertical: game state (see 3.1).
- The only true "camera" code is debug-only: `blit_to_screen` modes 5/6 (F7/F8, `debug`),
  which zoom 2x/4x on the player (`main.c:2903-2910`).
- For widescreen, the only legal camera change is a horizontal translation applied at
  draw time: `frame_x = world_x + CAMERA_X`, with `CAMERA_X = (FRAME_W - 640)/2` to keep the
  tower centred. The vertical view must stay 480 rows of game space (FRAME_H = 480 at
  logical scale).
- The historical world outside x ∈ [0, 640) contains only wall overhang
  (-57..0 and 640..697 if the wall is 132 px). Wider views need a synthesised side fill.
  That fill is presentation only and a design decision (Q2).

### 3.3 Mixed-domain expressions

- **HUD drawn over the tower.** The combo meter (22,100), combo star (-8,210), clock (6,10)
  and score (8,440) sit on the left wall and floor area. REPLAY and the custom-game captions
  (630-w, 4..36) and the VCR (635-w, 475-h) sit on the right wall. They are UI but were
  designed against the world composition.
- **Particles** live in frame space without scrolling. They are emitted from world
  positions (player trail `4267`, 50-floor fireworks `4589` spanning 20..619 at the view
  bottom) and from a UI anchor (reward burst at the banner, `2966`: (320,360)). One list
  and one draw loop (`3164-3166`) render all of them. With a centred tower the two anchors
  coincide. Otherwise the particles would need tagging (Q4).
- **Draw target inconsistencies inside `draw_frame(bmp)`.** Particles (`3166`) and the
  reward banner (`3306`) are hard-wired to `swap_screen` rather than `bmp`, and so is
  `draw_profile_selector` (`profile.c:721`). This is harmless while `bmp == swap_screen`,
  but a renderer that splits world and HUD into layers must account for it.
- **World geometry drawn to the physical screen.** The collision debug lines
  (`3600-3604`, `3674-3678`, `3774-3778`, F2 + `debug`) draw world coordinates straight onto
  `screen`.
- **Clip reset to frame size.** `set_clip_rect(bmp,0,0,639,479)` (`3378`) restores the clip
  after the VCR ticker. In a buffer wider than 640 it would clip the right side for the rest
  of the frame. `scroller.c` resets with `bmp->w-1/bmp->h-1` and is already size-agnostic.
- **Full-width bands and overlays hard-coded to 640.** The pause/exit grid (`4681`,
  `4751`, `5858`), the ticker bands `0..639` (`5390-5392`, `5756-5758`, `5868-5870`,
  `2170`) and the scroller widths 640 (`5340`, `6296`) all have to follow the frame width.
- **Mouse.** The menu ad hit test (`5716`) compares physical `mouse_x/mouse_y` with
  `SCREEN_H - ad->h`, but the ad is drawn at y = 280. The two agree only for a 200-px-tall
  ad.

### 3.4 Constants that can be named, and those that must not be

| proposed name | exact definition | proven meaning / uses | notes |
|---|---|---|---|
| `GAME_WIDTH` | `640` | width of game space and of the tower composition: pan normalisation `main.c:1958`; fireworks x range `20..619` = `GAME_WIDTH-20` (`4589`); symmetry of 85/555, 75/565, -57/565 | immutable. Never make it the window or frame width |
| `GAME_HEIGHT` | `480` | height of the game view in game space = 30 rows x 16. `map.c` `29` = `GAME_HEIGHT/16-1`; `main.c:3131` `464` = `GAME_HEIGHT-16`; fireworks spawn y `4589` | immutable. The vertical view is a game rule |
| `GAME_CENTER_X` | `GAME_WIDTH/2` (= 320) | tower axis of symmetry (85+555, 75+565, 37+603?) | no simulation line uses `320` literally. It is only the axis. Every literal `320` in the code is a UI centre (see below) |
| `GAME_CENTER_Y` | none | **no proven use.** The `240`s are an x (`1874`), a y that is not a centre (`4687`, `6191-6194`) and a debug half-height (`2904-2905`) | do not introduce it for existing code. Dialogs centre vertically near 200..220, not 240 |
| `FRAME_W`, `FRAME_H` | logical back-buffer size: historically `640`x`480`. In the port `FRAME_H` stays 480 and `FRAME_W` may grow | `swap_screen` creation `2615`, clip reset `3378`, full-width bands `0..639`, grids `i<640`, scroller widths `640`, full-frame backdrops `640,480`, debug blit modes | this is what `SCREEN_W/SCREEN_H` means everywhere in the game: `screen` is always the same size as `swap_screen` |
| `CAMERA_X` | `(FRAME_W - GAME_WIDTH)/2` | translation applied to every `WORLD` draw (and to the world particles) | 0 in the historical build |
| `UI_X_CENTER(x)` | `(x) + (FRAME_W-640)/2` | every literal-320 centre and every centred panel (103, 100, 120, 140, 160, 70, 355, 330, 400, ...) | equal to the tower centre when the tower is centred |
| `UI_X_RIGHT(x)` | `(x) + (FRAME_W-640)` | 630 (REPLAY, captions), 635 and 623 (VCR), 636..638 (welcome), 658 (score-table arrows) | 623 is really `VCR_x + (vcr_w-12)`, a VCR-relative position |
| `UI_X_LEFT(x)` | `(x)` | 4/5 (version), 8 (score), 20 (rank badge), 0 (ad, logo), HUD meter 22/33/-8/42, clock 6/34 | |

Constants that must stay literal, or be named only on their own terms and never derived
from `GAME_*`, `FRAME_*` or the window size:

- `player.c`: 85, 555, 1000 and the ±20 bounce. `map.c`: 32 rooms, 29, 16, 10000, 30, 5, 40
  and the `rand()` expressions. `main.c`: 160..0 scroll thresholds, 100 (auto-scroll start),
  15/16 (floor add), 540, 900, (200, 431), ±11 foot rays, 4 (snap), -12345678.
  Writing `555` as `GAME_WIDTH-85` would tie the simulation to a name that someone
  may later reinterpret as the view width.
- **False friends.** These look like frame constants but are not: `480` as an x coordinate
  (`main.c:3956-3957` qualify badge; `profile.c:875-879` dialog box); `320` as a y
  (`main.c:5349` rank target); `360` as an x (`5376`) and as a y (`2925`, `2932`, `2966`,
  `5596`); `240` as an x (`1874`); `464` in the menu ticker band (`5758`), which is unrelated
  to the floor origin 464; `320` as the left edge of left-aligned text (`6028`).
- Debug-only presentation modes (`blit_to_screen` 3..6) can stay literal and
  640x480-specific. They are reachable only with `debug` set.

**Caveat for the refactor.** `AGENTS.md` pins the source identities of the 25 units in
`recovery.json`. A macro that expands to the same literal keeps code generation identical
but still changes the pinned source, so it needs a reviewed gate extension, or the port
has to keep the names in port-only code (Q6).

## 4. HUD and UI element inventory

Anchor suggestions assume a view `FRAME_W >= 640`, `FRAME_H = 480`, with the tower centred
(`CAMERA_X = (FRAME_W-640)/2`). "Tower" means the element is attached to the tower, i.e.
offset by `CAMERA_X`, which reproduces the historical look exactly. The alternative anchor
moves it to the view edge.

### 4.1 In-game (`draw_frame`, `play`)

| element | where | historical position | suggested anchor |
|---|---|---|---|
| combo meter frame `data[16]` | main.c:3289 | (22,100) | top-left (alt: tower) |
| combo fill `data[15]` | 3291 | (33, 219-n), 16 wide, n ≤ 100 | top-left, with meter |
| combo star `data[14]` + number | 3292-3297 | star (-8,210), partly off-frame; number centred (42,210) | top-left, with meter |
| clock face `data[12]` / hand `data[13]` | 3301-3304 | (6,10) / rotate at (34,28); ±1 shake while `hurry_y` is in 251..479 / 201..479 | top-left |
| score text | 3323 | (8,440) | bottom-left |
| "REPLAY" blink | 3330-3334 | right edge 630, y 4/5 | top-right |
| custom-game captions (3 lines) | 3338-3352 | right edge 630, y 15/25/35 (+1 shadow) | top-right |
| VCR panel `data[127]` | 3357-3362 | (635-w, 475-h) | bottom-right |
| VCR buttons, ticker, progress | 3364-3392 | +97/107/117,+5; ticker clip x+10..623, y..479; progress 117 px at y+20 | bottom-right, relative to VCR (convert 623 to VCR-relative) |
| "Hurry up!" banner `data[67]` | 3125-3126, 4324-4331 | x = 320-w/2; y animates 479 → -100, step -2 | centre (horizontal), full view height |
| combo reward banner `data[90+r]` | 2918-2935 | centred at (320,360) (scaled/rotated) | centre (= tower centre) |
| reward particle burst | 2966 | emitted at (320,360) | same as banner (see Q4) |
| debug readouts | 3396-3404 | x 0/200/400, y 0/10/20 | top-left |
| debug floor numbers | 3144 | x 520, per floor row | tower (world-attached) |
| exit-confirm overlay | 4681-4687 | full-frame grid; text centred x 320, y 160/210/240 | grid: full view; text: centre |
| pause overlay | 4751-4756 | same grid; text 320 at 160/210 | same |

### 4.2 Post-game (`play` after death)

| element | where | historical position | suggested anchor |
|---|---|---|---|
| results panel (`draw_results`) | 3931-3963, 5252-5263 | logo x 320-w/2; labels x 200, values right x 440, PB mark 476, qualify badge 480+dist/+7; y = hy, sliding 480 → ~140 (lerp to 130) | centre |
| "Enter your initials" + letters | 5265, 5371-5376 | x 320 (letters 300/320/340/360), y hy*2+80 / +120 | centre |
| rank-up badge + text | 5379-5382 | x 20, y 580 → 320 | bottom-left (x left, y slide) |
| summary ticker band | 5340-5343, 5386-5397 | band 0..639, y -20 → 0; scroller width 640, x 0/1 | top, full view width |
| new start floor unlocked | 5530-5546 | backdrop `data[126]` 640x480, dim, head `data[58]` (320-w/2, 20), text x 320 at y 300/350/440 | backdrop: centre + side fill; text: centre |

### 4.3 Menus and dialogs

| element | where | historical position | suggested anchor |
|---|---|---|---|
| main-menu backdrop `data[126]` | main.c:5731 | full 640x480 | centre (the 640 composition), side fill |
| menu picture `data[71]` | 5732 | (330,280) | centre (composition) |
| logo `data[125]` | 5733 | (0,0) | centre (composition). Drawn centred `320-w/2` in the credits (`5593`) |
| ad banner | 5734-5736, 5716 | (0,280); hit-test `mouse_y > SCREEN_H - h` | bottom-left (ad code is dead without a server) |
| animated head + shadow | 5739-5751 | shadow buffer 640x320, shadow at (402,y), head at (400,y), y ≈ 6..28 | centre (composition) |
| main menu list | 6324, 6337, 6342, 6387 | (355,285) | centre (composition) |
| greeting ticker | 5753-5763, 6296 | band 0..639, y 462..479; scroller width 640 at y 462 | bottom, full view width |
| version text | 5765-5768 | (4,2)/(5,3) | top-left |
| welcome text | 5769-5795 | right edge 636..638, y 2/3 | top-right |
| replay menu | 5858-5874, 5995 | grid full-frame; panel `data[87]` (120,140); menu (180,160); summary ticker band 0..639, y 0..20 | panel/menu: centre; ticker: top, full width |
| save-replay dialog | 6026-6100 | `data[86]` stretched to (120,140,380x200); title (140,150); slots x 140, y 210/250/290, width 340; hint left edge (320,312) | centre |
| first-run profile dialog | 6184-6194 | panel `data[87]` (100,120); text x 130; box 129..430 x 240..258 | centre |
| `my_alert` | 1825-1887 | panel `data[88]` (103,130); title/text centred x 320, y 135/180; hint right edge 520,200; buttons (240,220)/(365,220) | centre |
| `line_alert` | 1791-1810 | panel (103,200); text centred (320,220) | centre |
| loading progress bar | 2153-2174 | frame 108..532 x 400..410; fill 320±size (≤212); log band 0..639 x 420..430; text (320,420) | centre; band full width |
| startup "please wait" / FLD logo | 2536, 2554 | (320,220); logo centred at (320,200) | centre |
| credits | 5589-5600 | backdrop 640x480; logo (320-w/2,10); text x 320 at 280/360/390 | centre + side fill |
| instructions | 5622-5623 | backdrop + masked full-frame page art 640x480 | centre + side fill |
| high-score viewer | hisc.c:217-279 | table at x 160, y 500 → 0 (scroll limit `470-h`); arrows `data[9]`/`data[6]` at x 658-dark (→500), y 380/40 | table: centre; arrows: right |
| profile viewer | profile.c:657-691 | panel (70, 500 → 15) | centre |
| profile selector + create dialog | profile.c:791-940 | selector (140, 500 → 50, exit at 510); dialog `data[88]` (100,140), text 140,149, box 139..480 x 191..210 | centre |
| replay selector | replay.c:837-1001 | selector (120, 500 → 25, exit at 510); Allegro `file_select_ex` 400x400 | centre |

## 5. Presentation boundary inventory

### 5.1 The normal presenter: `blit_to_screen` (main.c:2866-2913)

Mode 0 is `blit(bmp, screen, 0,0,0,0,bmp->w,bmp->h)` and is size-agnostic. Modes 1..6 are
debug-only (F2..F8 with `debug`): horizontal flip, vertical flip, sine wobble (640x480),
vertical wrap (`level % 480`), 2x and 4x player zoom. Callers:

- main.c: `fadeIn` 3902 (scratch `mybmp`), `fadeOut` 3922, play 4688, 4757, 4908 (after
  shake self-blit), 4912, 5281, 5419; `get_string` 5912 (its `bmp` argument is always
  `swap_screen`); save-replay dialog 6036, 6079, 6100; first-run profile 6193; main loop
  6341.
- menu.c: `handle_menu` 241. hisc.c: 243, 279. profile.c: 671, 691, 916, 940. replay.c:
  983, 998.

### 5.2 `swap_screen` → `screen` blits that bypass `blit_to_screen`

| where | code | note |
|---|---|---|
| main.c:1885 | `blit(swap_screen, screen, 0,0,0,0,639,479)` | `my_alert` restores the frame saved at 1844; 639x479 leaves the last column and row stale |
| main.c:5277 | `blit(swap_screen, screen, 0, new_rand()%8, 0,0,w,h)` | death shake while the results panel slides in |
| main.c:5414 | same | death shake in the results loop |

In-game shake (4907) is instead a self-blit `swap_screen → swap_screen` followed by
`blit_to_screen`.

### 5.3 Direct drawing on `screen`

| where | what |
|---|---|
| main.c:1802-1809 | `line_alert`: translucent dim over `gfx_driver->w/h`, panel and text |
| main.c:1841-1875 | `my_alert`: dim over `gfx_driver->w/h`, panel, text, yes/no buttons redrawn each tick |
| main.c:2166-2172 | `draw_progress_bar` during startup and datafile loading |
| main.c:2536 | "please wait" |
| main.c:2553-2554 | white clear + FLD logo |
| main.c:2773 | `clear_bitmap(screen)` after the intro fade |
| main.c:3600-3604, 3674-3678, 3774-3778 | collision debug lines (F2 + debug) |
| main.c:3926 | `fadeOut` final black `rectfill(screen,0,0,SCREEN_W,SCREEN_H)` |
| main.c:5577 | `clear(screen)` at the end of `play` |
| custom.c:250, 253 | `clear_bitmap(screen)` around a custom-character `alert` |
| main.c:2529, 5683 | `show_mouse(screen)` (windowed mode; cursor on the physical surface) |
| Allegro GUI, implicit | `alert()` main.c:2150 and custom.c:16; `file_select_ex(...,400,400)` replay.c:904. The GUI draws on `screen` and centres itself with `SCREEN_W/SCREEN_H` |

### 5.4 Read-backs from `screen`

These snapshot the last presented frame and use it as a dialog backdrop, so the compat
`screen` must hold exactly the last presented *logical* frame, not the scaled window.

| where | code |
|---|---|
| main.c:1844 | `blit(screen, swap_screen, 0,0,0,0,639,479)` (`my_alert`, after dimming `screen`) |
| main.c:3914 | `blit(screen, bmp, 0,0,0,0,SCREEN_W,SCREEN_H)` (`fadeOut`) |
| main.c:6171 | `blit(screen, bg, ...SCREEN_W,SCREEN_H)` (`force_create_profile`) |
| hisc.c:172 | `blit(screen, bg, ...SCREEN_W,SCREEN_H)` (`view_scores`) |
| profile.c:622 | `draw_sprite(bg, screen, 0, 0)` (`view_profile`; a *masked* copy, so magenta pixels are skipped) |
| profile.c:784 | `blit(screen, bgbmp, 0,0,0,0,640,480)` (`select_profile`) |
| replay.c:842 | `blit(screen, bg, ...SCREEN_W,SCREEN_H)` (`replay_selector`) |

### 5.5 Video mode, sizes, input and screenshots

- `set_gfx_mode(..., 640, 480, 0, 0)`: main.c:2503 (windowed), 2511 (fullscreen), 5661 and 5677
  (runtime toggle in `testWindowResolution`).
- `gfx_driver->w/h`: main.c:1802-1804, 1841-1842 (alert dims). In the port they must report
  the logical frame size.
- `if (!screen)` check 2519; `log2file(... screen)` 2530.
- `swap_screen = create_bitmap(640,480)` 2615; destroyed 2834.
- Mouse in physical coordinates: main.c:5716 (ad hit test). The port must map window
  coordinates to logical frame coordinates.
- Screenshots of the logical frame: `take_screenshot(swap_screen)` main.c:4623, 4836, 5285,
  5362, 5710 (`create_sub_bitmap(bmp,0,0,bmp->w,bmp->h)`, size-agnostic).

### 5.6 Fades and full-frame effects

- `fadeIn(bmp,speed)` (3889-3906): scratch `create_bitmap(SCREEN_W,SCREEN_H)`, black
  translucent rect over `SCREEN_W,SCREEN_H`, presented through `blit_to_screen`.
  Call sites: 4056, 5544, 5600, 5626, 6325, 6339, 6388.
- `fadeOut(speed)` (3908-3927): snapshot of `screen`, fade drawn on `swap_screen`, final black
  on `screen`. Call sites: 2772, 5527, 5615, 5638, 6005, 6346, 6357, 6372, 6382, 6385, 6397.
- Dimmed modal backdrops over `SCREEN_W/SCREEN_H`: main.c:5535, 6182; hisc.c:231, 267;
  profile.c:666, 686, 865, 910, 934; replay.c:980, 995.
- Stripe grid overlays: main.c:4681, 4751, 5858.

## 6. Full occurrence tables

Columns: line(s), code (trimmed), target, category, justification. Rows marked *(rel)* hold
related constants that the scanner tokens do not match.

### src/main.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 117 | `BITMAP *swap_screen = 0;` | - | PHYS | back-buffer global; the single composition surface |
| 1802-1804 | `if (gfx_driver) { height=gfx_driver->h; width=gfx_driver->w;` | - | PHYS | `line_alert` sizes its dim to the video mode |
| 1806 | `rectfill(screen,0,0,width,height,color);` | screen | PHYS | translucent dim of the physical screen |
| 1808 | `draw_sprite(screen,data[88].dat,103,200);` | screen | UI | alert panel, centred by symmetric margin (width unverified, Q1) |
| 1809 | `textprintf_centre_ex(screen,data[51].dat,320,220,...)` | screen | UI | alert text centred on x 320 |
| 1841-1842 | `rectfill(screen, 0, 0, gfx_driver ? gfx_driver->w : 0, gfx_driver ? gfx_driver->h : 0, ...)` | screen | PHYS | `my_alert` dims the physical screen |
| 1844 | `blit(screen, swap_screen, 0, 0, 0, 0, 639, 479);` | screen→swap | PHYS | read-back of the dimmed screen; 639x479 is one short (historical) |
| 1845 | `acquire_bitmap(screen);` | screen | PHYS | locks the physical surface |
| 1846 | `draw_sprite(screen, data[88].dat, 103, 130);` | screen | UI | dialog panel |
| 1847 | `textprintf_centre_ex(screen, data[51].dat, 320, 135, ..., func);` | screen | UI | centred title |
| 1849 | `textout_centre_ex(screen, data[54].dat, txt, 320, 180, ...);` | screen | UI | centred message |
| 1851 | `textout_right_ex(screen, ..., "(enter to continue)", 520, 200, ...)` | screen | UI | right edge of the panel text (panel-relative) |
| 1874 | `draw_sprite(screen, data[status ? 11 : 10].dat, 240, 220);` | screen | UI | yes button; 240 is an x |
| 1875 | `draw_sprite(screen, data[status ? 7 : 8].dat, 365, 220);` | screen | UI | no button |
| 1885 | `blit(swap_screen, screen, 0, 0, 0, 0, 639, 479);` | swap→screen | PHYS | restores the frame directly, bypassing `blit_to_screen` |
| 1958 | `pan=(int)((float)(ply[player_id]->x/640.0f)*192.0f+32.0f);` | - | WORLD | positional audio: world x over the 640-wide tower; must stay `GAME_WIDTH` |
| 2168 | `rect(screen,108,ypos,532,ypos+10,...)` (ypos=400) | screen | UI | loading-bar frame, symmetric about 320 |
| 2169 | `rectfill(screen,320-size,ypos,320+size,ypos+10,...)` | screen | UI | fill grows from the centre (≤ ±212) |
| 2170 | `rectfill(screen,0,420,639,430,makecol(255,255,255));` | screen | UI | full-width band for the log line |
| 2171 | `textout_centre_ex(screen,font,last_log,320,420,...)` | screen | UI | centred log text |
| 2503 | `set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0)` | - | PHYS | windowed video mode |
| 2511 | `set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0)` | - | PHYS | fullscreen video mode |
| 2519-2520 | `if (!screen) { log2file("ERROR: screen was not set");` | - | PHYS | video-mode success check |
| 2529 | `show_mouse(screen);` | screen | PHYS | software/hardware cursor on the physical surface |
| 2530 | `log2file("Graphics mode set. (screen = %d)",screen);` | - | PHYS | logs the surface pointer |
| 2536 | `textprintf_centre_ex(screen,font,320,220,...,"please wait");` | screen | UI | centred startup text |
| 2549 | `log2file("Putting FLD Logo on screen");` | - | text | log string |
| 2553 | `clear_to_color(screen,whiteColor);` | screen | PHYS | clears the physical screen |
| 2554 | `draw_sprite(screen,fldLogo,320-fldLogo->w/2,200-fldLogo->h/2);` | screen | UI | logo centred at (320,200) |
| 2615-2616 | `swap_screen=create_bitmap(640,480); if (!swap_screen)` | - | PHYS | back buffer = historical frame size |
| 2619 | `allegro_message("Failed reserve memory screen buffers.");` | - | text | message string |
| 2773 | `clear_bitmap(screen);` | screen | PHYS | clear after the intro fade |
| 2834 | `if (swap_screen) destroy_bitmap(swap_screen);` | - | PHYS | back-buffer lifetime |
| 2880 | `blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);` | swap→screen | PHYS | **the presentation boundary** (mode 0), size-agnostic |
| 2883 | `draw_sprite_h_flip(screen, bmp, 0, 0);` | swap→screen | PHYS | debug mode 1 |
| 2886 | `draw_sprite_v_flip(screen, bmp, 0, 0);` | swap→screen | PHYS | debug mode 2 |
| 2890-2892 | `for (y = 0; y < 480; y++) { ... blit(bmp, screen, 0, y, x, y, 640, 1);` | swap→screen | TEMP | debug wobble, 640x480-specific |
| 2896-2898 | `int y = level % 480; line(bmp, 0, 479, 639, 479, 0); line(bmp, 0, 0, 639, 0, 0);` | swap | TEMP | debug vertical wrap |
| 2899-2900 | `blit(bmp, screen, 0,0,0,y,...); blit(bmp, screen, 0,0,0,y-480,...)` | swap→screen | TEMP | debug vertical wrap |
| 2903-2905 | `MID(0, x-160.0, 320)`, `MID(0, y-160.0, 240)`, `stretch_blit(bmp, screen, x, y, 320, 240, 0, 0, 640, 480)` | swap→screen | CAMERA | debug 2x zoom that follows the player; 320x240 source window; dest = frame |
| 2908-2910 | `MID(0, x-80.0, 520)`, `MID(0, y-80.0, 360)`, `stretch_blit(bmp, screen, x, y, 160, 120, 0, 0, 640, 480)` | swap→screen | CAMERA | debug 4x zoom; clamps 520/360 exceed 640-160/480-120 (historical quirk) |
| 2923-2927 | `stretch_sprite(bmp,reward_bmp, 320-..., 360-...-..., ...)` | swap | UI | reward banner centred at (320,360) |
| 2930-2933 | `rotate_scaled_sprite(bmp,reward_bmp, 320-..., 360-..., ...)` | swap | UI | same banner, rotating variant |
| 2966 | `p = create_particle(stars, 320, 360);` | - | UI | reward burst emitted at the banner anchor (Q4) |
| 3094-3108 *(rel)* | `while (map.offset / 256 > last_stripe_y) { ... new_rand() ... }` | - | WORLD | stripe variety per 256 px of scroll; presentation RNG |
| 3113-3114 *(rel)* | `blit(stripe, bmp, 0, 0, 37, ls * 128 + (map.offset % 256) / 2, w, h)` | swap | WORLD | tower back wall at x 37, half-speed parallax |
| 3125-3126 | `if (hurry_y > -100 && hurry_y < 480 && ...) draw_sprite(bmp, data[67].dat, 320 - w / 2, hurry_y);` | swap | UI | "Hurry up" banner rising from the frame bottom |
| 3131 *(rel)* | `cx = 464 - y * 16;` | - | WORLD | floor row → frame y; 464 = 29*16 is the draw twin of the `map.c` row origin |
| 3136-3142 *(rel)* | `draw_sprite(bmp, data[f].dat, x * 16 - 5, cx + (map.offset % 16) - 6);` (+ middle, right cap) | swap | WORLD | floor tiles at tile*16 |
| 3144 *(rel)* | `textprintf_ex(bmp, font, 520, cx + (map.offset % 16), 15, -1, ...)` | swap | WORLD | debug floor number, world row at fixed x 520 |
| 3149-3159 *(rel)* | `sy = cx + (map.offset % 16) + 10; cy = (start + (end-start)/2) * 16; draw_sprite(bmp, sign, cy, sy)` | swap | WORLD | floor sign at the floor midpoint |
| 3164-3166 | `draw_sprite(swap_screen, data[color + 117].dat, fixtoi(x), fixtoi(y));` | swap | WORLD | particles; target hard-wired to `swap_screen`; frame space, no scroll |
| 3202-3235 *(rel)* | `draw_sprite(bmp, customFrame, (int)x + ox, (int)y + oy)` (all poses) | swap | WORLD | player sprite at game coordinates |
| 3213 *(rel)* | `if (map.offset > 200 && ply[player_id]->y > 400.0) customFrame = custom.frame[11];` | - | WORLD | idle "look down" pose chosen from game y |
| 3283 *(rel)* | `draw_sprite(bmp, data[100].dat, 565, ls*124 + (int)((map.offset % 84) * 1.476)); draw_sprite_h_flip(bmp, data[100].dat, -57, ...)` | swap | WORLD | tower walls, mirrored about 320, ~1.48x parallax |
| 3289 *(rel)* | `draw_sprite(bmp, data[16].dat, 22, 100);` | swap | UI | combo meter frame (over the left wall) |
| 3291 *(rel)* | `blit(data[15].dat, bmp, 0, 100 - n, 33, 219 - n, 16, n);` | swap | UI | combo meter fill |
| 3292, 3296 *(rel)* | `draw_sprite(bmp, data[14].dat, -8, 210);` | swap | UI | combo star, partly off the left frame edge |
| 3293, 3297 *(rel)* | `textprintf_centre_ex(bmp, data[50].dat, 42, 210, ...)` | swap | UI | combo number |
| 3301 | `if (hurry_y < 251 \|\| hurry_y > 479) { cx = 0; cy = 0; } else {...}` | - | UI | clock shakes while the hurry banner crosses y 251..479 |
| 3302 | `draw_sprite(bmp, data[12].dat, cx + 6, cy + 10);` | swap | UI | clock face, top-left |
| 3303 | `if (hurry_y >= 201 && hurry_y <= 479) {...}` | - | UI | clock-hand shake window |
| 3304 *(rel)* | `rotate_sprite(bmp, data[13].dat, cx + 34, cy + 28, ...)` | swap | UI | clock hand |
| 3305-3306 | `if (reward_time) draw_reward(swap_screen);` | swap | UI | reward banner; target hard-wired |
| 3323 *(rel)* | `textprintf_ex(bmp, data[52].dat, 8, 440, ..., "score: %d", ...)` | swap | UI | score, bottom-left |
| 3330-3334 *(rel)* | `myPos = 630 - text_length(...); textprintf_ex(bmp, ..., myPos + 1, 5, ...)` | swap | UI | "REPLAY", right edge 630 |
| 3338-3352 *(rel)* | `cx = 630 - text_length(...); textout_ex(bmp, ..., cx, 15/25/35, ...)` | swap | UI | custom-game captions, right edge 630 |
| 3357-3362 *(rel)* | `x = 635 - vcr->w; y = 475 - vcr->h; draw_sprite(bmp, vcr, x, y);` | swap | UI | VCR panel, bottom-right, 5 px margins |
| 3364-3366 *(rel)* | `draw_sprite(bmp, data[128].dat, x + 97, y + 5);` (+107, +117) | swap | UI | VCR input lights, panel-relative |
| 3368-3369 | `cx = y + 10; cy = x + 10; set_clip_rect(bmp, cy, 0, 623, 479);` | swap | UI | ticker clip; 623 is VCR-relative in disguise; 479 = frame bottom |
| 3372-3376 *(rel)* | `textout_ex(bmp, ..., demo->name, x + 12 - scroll_count / 2, cx + 4, ...)` | swap | UI | VCR ticker text |
| 3378 | `set_clip_rect(bmp, 0, 0, 639, 479);` | swap | PHYS | resets the clip to the full back buffer; must follow `FRAME_W` |
| 3392 *(rel)* | `rect(bmp, cy, cx + 20, cy + (myPos * 117 / len ...), cx + 19, ...)` | swap | UI | VCR progress bar |
| 3396-3404 *(rel)* | `textprintf_ex(bmp, font, 0/200/400, 0/10/20, ...)` | swap | UI | debug readouts, top-left |
| 3517-3518 *(rel)* | `ply->x = 200.0; ply->y = 431.0;` | - | GAME | start pose on the floor surface at y 432 |
| 3522 | `hurry_y = 480;` | - | UI | banner parked at the frame bottom (hidden) |
| 3565-3574, 3644-3653, 3719-3728 *(rel)* | `plx1 = (int)x - 11; ... prx1 = (int)x + 11; ... fy1 = -12345678;` | - | GAME | foot rays ±11 for the floor-segment collision |
| 3600-3604 | `line(screen, fx1, fy1, fx2, fy2, col1); line(screen, plx1, ...); line(screen, prx1, ...);` | screen | WORLD | debug collision lines in world coordinates, drawn on `screen` |
| 3674-3678 | same, `handle_player_collision_vector_2` | screen | WORLD | same |
| 3774-3778 | same, `handle_player_collision_combo` | screen | WORLD | same |
| 3894 | `mybmp=create_bitmap(SCREEN_W,SCREEN_H);` | tmp | TEMP | `fadeIn` scratch |
| 3900 | `rectfill(mybmp,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));` | tmp | TEMP | fade veil |
| 3913 | `bmp=create_bitmap(SCREEN_W,SCREEN_H);` | tmp | TEMP | `fadeOut` snapshot buffer |
| 3914 | `blit(screen,bmp,0,0,0,0,SCREEN_W,SCREEN_H);` | screen→tmp | PHYS | read-back of the last presented frame |
| 3917 | `draw_sprite(swap_screen,bmp,0,0);` | swap | TEMP | fade source |
| 3920 | `rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));` | swap | TEMP | fade veil |
| 3922 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | presents the fade step |
| 3926 | `rectfill(screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));` | screen | PHYS | final black directly on screen |
| 3941 | `draw_sprite(bmp,logo,320-logo->w/2,y);` | swap | UI | results logo centred |
| 3943-3946 *(rel)* | `textprintf_ex(bmp,...,200,...)`, `textprintf_right_ex(bmp,...,440,...)` | swap | UI | results columns |
| 3950 *(rel)* | `draw_sprite(bmp,data[69].dat,476,...)` | swap | UI | personal-best marker |
| 3956-3957 | `draw_sprite(bmp,data[68].dat,480+dist,...); textprintf_ex(bmp,...,480+dist+7,...)` | swap | UI | qualify badge; **480 is an x** |
| 4054 | `draw_frame(swap_screen);` | swap | PHYS | first frame into the back buffer |
| 4056 | `fadeIn(swap_screen, 16);` | swap | PHYS | presentation (fade in) |
| 4267 *(rel)* | `create_particle(stars, (int)ply->x, (int)ply->y - 16);` | - | WORLD | combo trail emitted at the player |
| 4278-4291 *(rel)* | `if (y < 160.0) { scroll_acc = 1; ... < 140 ... < 0.0 ...; map.offset = old + scroll_acc; y += scroll_acc;` | - | GAME | player-driven scroll = vertical camera as game state |
| 4297-4315 *(rel)* | `if (map.offset > 100 && !dead) { ... map.offset += scroll; ply->y += scroll; ...}` | - | GAME | auto-scroll |
| 4324 | `if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;` | - | UI | banner animation |
| 4331 | `hurry_y = 479;` | - | UI | banner starts at the frame bottom |
| 4342-4349 *(rel)* | `if (old_map_pos % 16 > map.offset % 16) add_floor(&map); else if (tot_scroll > 15) add_floor(&map);` | - | GAME | row insertion on each 16-px scroll |
| 4423 *(rel)* | `level = (get_level(&map, (int)ply->y) - 1) / 5;` | - | GAME | floor counting from game y |
| 4535 *(rel)* | `if (ply->y > 540.0 && !ply->dead)` | - | GAME | death line, 60 px below the view |
| 4569 *(rel)* | `if (ply->y > 900.0 && !game_over)` | - | GAME | logic-side game-over trigger |
| 4589-4590 | `p = create_particle(stars, (new_rand() % 600) + 20, 480);` | - | WORLD | 50-floor fireworks across the tower width (20..619) from the view bottom |
| 4623 | `take_screenshot(swap_screen);` | swap | PHYS | screenshot of the logical frame |
| 4656 *(rel)* | `shake = new_rand() % 8;` | - | TEMP | shake amplitude |
| 4681-4684 | `for (i = 0; i < 640; i += 2) { vline(swap_screen, i, 0, 480, 0); hline(swap_screen, 0, i, 640, 0); }` | swap | TEMP | exit overlay grid (hline rows ≥ 480 clipped) |
| 4685-4687 | `textout_centre_ex(swap_screen, ..., 320, 160/210/240, ...)` | swap | UI | exit prompt centred; 240 is only a text y |
| 4688 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 4751-4754 | same grid loop | swap | TEMP | pause overlay grid |
| 4755-4756 | `textout_centre_ex(swap_screen, ..., "Game Paused", 320, 160, ...)` (+210) | swap | UI | pause text centred |
| 4757 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 4836 | `take_screenshot(swap_screen);` | swap | PHYS | screenshot (replay pause) |
| 4897 | `draw_frame(swap_screen);` | swap | PHYS | per-frame composition target |
| 4907 | `blit(swap_screen, swap_screen, 0, shake, 0, 0, swap_screen->w, swap_screen->h);` | swap | TEMP | in-place shake (frame moved up 0..7 px) |
| 4908 | `blit_to_screen(swap_screen); release_screen();` | swap→screen | PHYS | present |
| 4912 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 5252-5258 *(rel)* | `hy = 480.0f; hyTarget = 140.0f; hy = hy + (130.0f - hy) * 0.1;` | - | UI | results panel slides in from the frame bottom |
| 5261 | `if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;` | - | UI | banner animation |
| 5262 | `draw_frame(swap_screen);` | swap | PHYS | composition target |
| 5263 | `draw_results(swap_screen, ..., (int)hy, ...)` | swap | UI | results panel at y = hy |
| 5265 | `textout_centre_ex(swap_screen, ..., "Enter your initials", 320, (int)(hy * 2 + 80), ...)` | swap | UI | centred prompt |
| 5277 | `blit(swap_screen, screen, 0, new_rand() % 8, 0, 0, swap_screen->w, swap_screen->h);` | swap→screen | TEMP | shake presented directly (bypass) |
| 5281 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 5285 | `take_screenshot(swap_screen);` | swap | PHYS | screenshot |
| 5340 | `init_scroller(&summary_scroller, data[54].dat, msg, 640, 30, -1);` | - | UI | ticker width = frame width |
| 5348-5349 | `rank_y = 0x244; int rankTargetY = 320;` | - | UI | rank badge slides 580 → 320; **320 is a y** |
| 5362 | `take_screenshot(swap_screen);` | swap | PHYS | screenshot |
| 5367 | `if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;` | - | UI | banner animation |
| 5368 | `draw_frame(swap_screen);` | swap | PHYS | composition target |
| 5369 | `draw_results(swap_screen, ..., (int)hy, ...)` | swap | UI | results panel |
| 5371 | `textout_centre_ex(swap_screen, ..., "Enter your initials", 320, (int)(hy * 2 + 80), ...)` | swap | UI | centred prompt |
| 5373-5376 | `textout_centre_ex(swap_screen, ..., &buf[0..4]/"%", 300/320/340/360, (int)(hy * 2 + 120), ...)` | swap | UI | initials letters; 360 is an x |
| 5380-5381 | `draw_sprite(swap_screen, data[rank_bmp_id].dat, 20, rank_y); textout_ex(..., "rank up!", 20, rank_y + 0x46, ...)` | swap | UI | rank-up badge, left |
| 5390-5392 | `rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 20/18/16, ...)` | swap | UI | full-width translucent ticker band (top) |
| 5394-5395 | `draw_scroller(&summary_scroller, swap_screen, 1/0, scrollerY, ...)` | swap | UI | ticker text |
| 5414 | `blit(swap_screen, screen, 0, new_rand() % 8, 0, 0, swap_screen->w, swap_screen->h);` | swap→screen | TEMP | shake presented directly (bypass) |
| 5419 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 5530 | `blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | UI | full-frame backdrop art (unlock screen) |
| 5535 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));` | swap | TEMP | translucent dim |
| 5539 | `draw_sprite(swap_screen, data[58].dat, 320 - w / 2, 20);` | swap | UI | centred head |
| 5540-5542 | `textout_centre_ex(swap_screen, ..., 320, 0x12c/0x15e/0x1b8, ...)` | swap | UI | centred text at y 300/350/440 |
| 5544 | `fadeIn(swap_screen, 16);` | swap | PHYS | presentation (fade) |
| 5577 | `clear(screen);` | screen | PHYS | clears the physical screen |
| 5589 | `clear(swap_screen);` | swap | PHYS | back-buffer clear |
| 5592 | `blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | UI | credits backdrop art |
| 5593 | `draw_sprite(swap_screen, logoBMP, 320 - logoBMP->w / 2, 10);` | swap | UI | centred logo |
| 5595-5598 | `textout_centre_ex(swap_screen, ..., 320, 280/360/390, ...)` | swap | UI | credits text; 360 is a y |
| 5600 | `fadeIn(swap_screen, 16);` | swap | PHYS | presentation (fade) |
| 5622 | `blit(data[126].dat,swap_screen,0,0,0,0,640,480);` | swap | UI | instructions backdrop |
| 5623 | `masked_blit(data[70].dat,swap_screen,0,0,0,0,640,480);` | swap | UI | full-frame instructions page art |
| 5626 | `fadeIn(swap_screen,16);` | swap | PHYS | presentation (fade) |
| 5661 | `set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0);` | - | PHYS | runtime mode switch |
| 5677 | `set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0);` | - | PHYS | runtime mode switch |
| 5683 | `show_mouse(screen);` | screen | PHYS | cursor on the physical surface |
| 5710 | `take_screenshot(swap_screen);` | swap | PHYS | screenshot (menu) |
| 5716 | `mouseInAd = mouse_x < pFLDAdBitmap->w && mouse_y > SCREEN_H - pFLDAdBitmap->h;` | - | PHYS | physical mouse vs screen height; ad is drawn at y 280 (agrees only for a 200-px ad) |
| 5731 | `blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | UI | main-menu backdrop art |
| 5732 | `draw_sprite(swap_screen, data[71].dat, 330, 280);` | swap | UI | menu picture |
| 5733 | `draw_sprite(swap_screen, data[125].dat, 0, 0);` | swap | UI | logo |
| 5736 | `draw_trans_sprite(swap_screen, pFLDAdBitmap, 0, 280);` | swap | UI | ad banner |
| 5741 | `head_bmp = create_bitmap(640, 320);` | tmp | TEMP | head-shadow scratch: full width, top 320 rows |
| 5745 *(rel)* | `rotate_sprite(head_bmp, head_shadow, 402, headY, headX * 5);` | tmp | UI | shadow position (head + 2 px) |
| 5747 | `draw_trans_sprite(swap_screen, head_bmp, 0, 0);` | swap | TEMP | composites the shadow buffer |
| 5751 | `rotate_sprite(swap_screen, head, 400, headY, headX * 5);` | swap | UI | animated head |
| 5756-5758 | `rectfill(swap_screen, 0, 462/463/464, 639, 479, makecol(0, 0, 0));` | swap | UI | full-width bottom ticker band; 464 unrelated to floors |
| 5760-5762 | `draw_scroller(&greeting_scroller, swap_screen, 1/0, 462, ...)` | swap | UI | greeting ticker text |
| 5765-5768 | `textprintf_ex(swap_screen, data[54].dat, 5, 3 / 4, 2, ..., "v%s %s", ...)` | swap | UI | version, top-left |
| 5772-5793 | `textprintf_right_ex(swap_screen, data[54].dat, 638/637/636, 3/2, ...)` (8 calls) | swap | UI | welcome text, right-aligned top-right |
| 5858-5860 | `for (i=0; i<640; i+=2) { vline(swap_screen,i,0,480,0); hline(swap_screen,0,i,640,0); }` | swap | TEMP | replay-menu grid overlay |
| 5862 | `draw_sprite(swap_screen,data[87].dat,120,140);` | swap | UI | replay-menu panel |
| 5868-5870 | `rectfill(swap_screen,0,0,639,20/18/16,makecol(0,0,0));` | swap | UI | full-width top ticker band |
| 5872-5873 | `draw_scroller(&summary_scroller,swap_screen,1/0,0,...)` | swap | UI | ticker text |
| 5878 | `/* ... callers retain the supplied string and screen. */` | - | text | comment |
| 5995-5996 | `handle_menu(replay_menu,&menu_params,&ctrl,swap_screen, replay_menu_callback,180,160,0);` | swap | UI | replay menu anchor |
| 6026 | `stretch_sprite(swap_screen,data[86].dat,120,140,380,200);` | swap | UI | save-replay panel |
| 6027 | `textout_ex(swap_screen,data[51].dat,"SAVE REPLAY",140,150,-1,-1);` | swap | UI | title |
| 6028 | `textout_ex(swap_screen,data[54].dat,"(enter to advance)",320,312, ...)` | swap | UI | hint; 320 is a left edge, not a centre |
| 6030-6034, 6047-6053, 6070-6076, 6091-6097 | `drawSlot(swap_screen,140,210/250/290,...)` | swap | UI | input slots (340 wide) |
| 6036, 6079, 6100 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 6038, 6062, 6082 | `get_string(swap_screen,...,340,...,140,210/250/290,...)` | swap | UI | text editor position |
| 6170 | `bg=create_bitmap(SCREEN_W,SCREEN_H);` | tmp | TEMP | snapshot buffer |
| 6171 | `blit(screen,bg,0,0,0,0,SCREEN_W,SCREEN_H);` | screen→tmp | PHYS | read-back |
| 6179 | `blit(bg,swap_screen,0,0,0,0,SCREEN_W,SCREEN_H);` | swap | TEMP | backdrop copy |
| 6182 | `rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));` | swap | TEMP | translucent dim |
| 6184 | `draw_sprite(swap_screen,data[87].dat,100,120);` | swap | UI | first-run panel |
| 6185-6190 | `textout_ex(swap_screen,...,130,127..220,...); textout_right_ex(...,430,260,...)` | swap | UI | first-run text |
| 6191-6192 | `rectfill/rect(swap_screen,129,240,430,258,...)` | swap | UI | input box; 240 is a y |
| 6193 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 6194 | `get_string(swap_screen,new_name,300,32,data[54].dat,130,240,...)` | swap | UI | editor position |
| 6296 | `init_scroller(&greeting_scroller, data[54].dat, scroller_greetings, 640, 30, -1);` | - | UI | ticker width = frame width |
| 6324, 6337, 6387 | `draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);` | swap | UI | main menu anchor |
| 6325, 6339, 6388 | `fadeIn(swap_screen, 16/32);` | swap | PHYS | presentation (fade) |
| 6341 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 6342 | `handle_menu(main_menu, &menu_params, &ctrl, swap_screen, main_menu_callback, 355, 285, 0);` | swap | UI | main menu anchor |

### src/map.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 14-19 *(rel)* | `for (i = 0; i < 32; i++) {...} m->offset = 0;` | - | GAME | 32 rows = 30 visible + 2 above |
| 27-28 *(rel)* | `for (i=0; i<31; i++) m->room[i]=m->room[i+1];` | - | GAME | rows shift down; new row enters at room[31] (y -32) |
| 35, 41 *(rel)* | `m->room[31].start_tile=0; m->room[31].end_tile=40;` | - | GAME | full-width floors (tiles 0..40) |
| 59-61 *(rel)* | `m->room[31].start_tile=rand()%(30-width)+5; end_tile=start_tile+width;` | - | GAME | random floors within tiles 5..34 (x 80..559) |
| 77-78 *(rel)* | `y = (cy + 1) >> 4; if (29 - y < 0 \|\| 29 - y > 31) return 0;` | - | GAME | row of a game y; 29 = 480/16 - 1 |
| 80-83 *(rel)* | `x = cx >> 4; ... return cy - (m->offset % 16) + 10000 - (y << 4);` | - | GAME | tile column and surface penetration |
| 88-89 *(rel)* | `int y = 29 - ((cy + 1) >> 4); if (y < 0 \|\| y > 31) return 0;` | - | GAME | floor level under a game y |
| 95-100 *(rel)* | `*fx1 = (start << 4) - 2; *fx2 = ((end + 1) << 4) + 1; *fy = (y << 4) + (m->offset % 16);` | - | GAME | floor segment for the collision vector |

### src/player.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 70-71 *(rel)* | `if (p->y > 1000) p->y = 1000;` | - | GAME | fall clamp |
| 72-77 *(rel)* | `if (p->x > 555) { p->x = 555; p->sx *= -0.9; ... p->bounce = -20; }` | - | GAME | right wall bounce (feet at 566) |
| 78-83 *(rel)* | `if (p->x < 85) { p->x = 85; ... p->bounce = 20; }` | - | GAME | left wall bounce (feet at 74) |

### src/menu.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 97-145 *(rel)* | `draw_menu(BITMAP *bmp, ..., int cx, int y, int dx)`: all positions relative to `(cx, y)` | swap | UI | caller-supplied anchor (355,285 / 180,160); no absolute screen literals |
| 236-237 *(rel)* | `if (callback) callback(); else clear(bmp);` | swap | PHYS | back-buffer clear when there is no callback |
| 241 *(rel)* | `blit_to_screen(bmp);` | swap→screen | PHYS | present |
| 375, 395 *(rel)* | `line_alert(txt);`, `my_alert("handle_menu", buf, 0, 0);` | screen | PHYS | route to the direct-to-screen dialogs |

### src/scroller.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 40-42, 50, 61 *(rel)* | `set_clip_rect(bmp, x, y, x + sc->width, y + sc->height); ... set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);` | swap | UI | ticker clip; the reset is size-agnostic (a good pattern) |

### src/hisc.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 22 | `extern BITMAP *swap_screen;` | - | PHYS | back-buffer declaration |
| 171 | `BITMAP *bg = create_bitmap(SCREEN_W, SCREEN_H);` | tmp | TEMP | snapshot buffer |
| 172 | `blit(screen, bg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);` | screen→tmp | PHYS | read-back |
| 217-218, 257 *(rel)* | `pageY = 500; targetY = 0;` / `targetY = 500;` | - | UI | table slides in from below the frame |
| 227 | `blit(bg, swap_screen, 0, 0, 0, 0, bg->w, bg->h);` | swap | TEMP | backdrop copy |
| 231 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));` | swap | TEMP | translucent dim |
| 234 | `draw_sprite(swap_screen, bmp, 160, pageY);` | swap | UI | score table, centred panel |
| 239-240 | `draw_sprite(swap_screen, data[9]/data[6], 658 - dark, 380/40);` | swap | UI | scroll arrows sliding in from the right (→ x 500) |
| 243 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 248 *(rel)* | `if (is_down(...)) targetY = MAX(470 - bmp->h, targetY - 16);` | - | UI | scroll limit = view height - 10 |
| 259 | `while (pageY <= 480) {` | - | UI | exit animation until below the frame |
| 263 | `draw_sprite(swap_screen, bg, 0, 0);` | swap | TEMP | backdrop copy |
| 267 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));` | swap | TEMP | dim |
| 270 | `draw_sprite(swap_screen, bmp, 160, pageY);` | swap | UI | score table |
| 275-276 | `draw_sprite(swap_screen, data[9]/data[6], 658 - dark, 380/40);` | swap | UI | arrows |
| 279 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |

### src/profile.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 95 | `extern void *swap_screen;` | - | PHYS | back-buffer declaration |
| 621 | `bg = create_bitmap(640, 480);` | tmp | TEMP | snapshot buffer (literal frame size) |
| 622 | `draw_sprite(bg, screen, 0, 0);` | screen→tmp | PHYS | masked read-back of the screen |
| 657-658 *(rel)* | `pageY = 500; targetY = 15;` | - | UI | panel slides in from below |
| 663 | `draw_sprite(swap_screen, bg, 0, 0);` | swap | TEMP | backdrop copy |
| 664, 684 *(rel)* | `set_trans_blender(0, 0, 0, (500 - pageY) / 3);` | - | TEMP | dim strength follows the panel position |
| 666-667 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, ...)` | swap | TEMP | dim |
| 669 | `draw_sprite(swap_screen, bmp, 70, pageY);` | swap | UI | profile panel |
| 671 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 680 | `while (pageY <= 480) {` | - | UI | exit animation |
| 683 | `draw_sprite(swap_screen, bg, 0, 0);` | swap | TEMP | backdrop copy |
| 686-687 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, ...)` | swap | TEMP | dim |
| 689 | `draw_sprite(swap_screen, bmp, 70, pageY);` | swap | UI | panel |
| 691 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 721 | `draw_sprite(swap_screen, (BITMAP *)data[86].dat, x - 15, y - 15);` | swap | UI | selector frame; target hard-wired inside a `bmp`-parameterised function |
| 783 | `bgbmp = create_bitmap(640, 480);` | tmp | TEMP | snapshot buffer |
| 784 | `blit(screen, bgbmp, 0, 0, 0, 0, 640, 480);` | screen→tmp | PHYS | read-back |
| 791-792 *(rel)* | `pageY = 500; targetY = 50;` | - | UI | selector slides in |
| 865-868 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, ...)` | swap | TEMP | dim for the create dialog |
| 872 | `draw_sprite(swap_screen, data[88].dat, 100, 140);` | swap | UI | dialog panel |
| 873-874 | `textout_ex(swap_screen, data[51].dat, "Enter profile name:", 140, 149, -1, -1);` | swap | UI | dialog title |
| 875-876 | `textout_right_ex(swap_screen, ..., "...and press enter.", 480, 210, 0, -1);` | swap | UI | hint; **480 is an x** |
| 877-880 | `rectfill/rect(swap_screen, 139, 191, 480, 210, ...)` | swap | UI | input box; 480 is an x |
| 881-882 | `get_string(swap_screen, new_name, 340, 32, ..., 140, 191, ...)` | swap | UI | editor position |
| 907 | `blit(bgbmp, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | TEMP | backdrop copy |
| 910-911 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, ...)` | swap | TEMP | dim |
| 913-915 | `draw_profile_selector(swap_screen, ..., 140, pageY);` | swap | UI | selector position |
| 916 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 927-928 *(rel)* | `targetY = 510; while (pageY <= 499) {` | - | UI | exit animation |
| 931 | `blit(bgbmp, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | TEMP | backdrop copy |
| 934-935 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, ...)` | swap | TEMP | dim |
| 937-939 | `draw_profile_selector(swap_screen, ..., 140, pageY);` | swap | UI | selector position |
| 940 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |

### src/replay.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 27 | `extern BITMAP *swap_screen;` | - | PHYS | back-buffer declaration |
| 837 *(rel)* | `int pageY = 500;` | - | UI | selector starts below the frame |
| 841 | `bg = create_bitmap(SCREEN_W, SCREEN_H);` | tmp | TEMP | snapshot buffer |
| 842 | `blit(screen, bg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);` | screen→tmp | PHYS | read-back |
| 904 *(rel)* | `file_select_ex("Select a new folder and press OK.", path, "itr", 1024, 400, 400);` | screen | UI | Allegro GUI dialog, 400x400, drawn on `screen` and centred by Allegro |
| 976 *(rel)* | `pageY = (int)(0.2 * (25 - pageY) + pageY);` | - | UI | slide to y 25 |
| 977 | `blit(bg, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | TEMP | backdrop copy |
| 980 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));` | swap | TEMP | dim |
| 982 | `draw_replay_selector(swap_screen, rep, ..., 120, pageY);` | swap | UI | selector position |
| 983 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |
| 989-991 *(rel)* | `while (pageY <= 499) { pageY = (int)(0.2 * (510 - pageY) + pageY);` | - | UI | exit animation |
| 992 | `blit(bg, swap_screen, 0, 0, 0, 0, 640, 480);` | swap | TEMP | backdrop copy |
| 995 | `rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));` | swap | TEMP | dim |
| 997 | `draw_replay_selector(swap_screen, NULL, ..., 120, pageY);` | swap | UI | selector position |
| 998 | `blit_to_screen(swap_screen);` | swap→screen | PHYS | present |

### src/custom.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 16 *(rel)* | `alert("CUSTOM CHARACTER", txt1, txt2, "OK", NULL, 0, 0);` | screen | PHYS | Allegro GUI alert drawn on `screen`, centred with `SCREEN_W/SCREEN_H` |
| 250 | `clear_bitmap(screen);` | screen | PHYS | clears before the alert |
| 253 | `clear_bitmap(screen);` | screen | PHYS | clears after the alert |

### src/loadpng.c

| line(s) | code | target | category | justification |
|---|---|---|---|---|
| 27 | `*  Get screen gamma value one of three ways.` | - | text | comment (PNG gamma) |

### Units with no occurrences

`beta.c, control.c, csv.c, directories.c, fld_adspot.c, game_data.c, httpget.c, options.c,
particle.c, regpng.c, savepng.c, stars.c, strptime.c, timecompat.c, timer.c`.
`particle.c` holds 16.16 fixed-point positions in frame space with no size constants.
`stars.c` (`init_star_field`, parameterised w/h) is never called by the game.

## 7. Open questions

1. **Asset widths.** The source gives positions only. The symmetry claims (wall inner faces
   75/565, stripes 37..603, centred panels at x 103/100/120/160/70) depend on the widths of
   `data[100]` (wall), `data[1..5]` (stripes), `data[86/87/88]` (panels), `data[66]` (score
   table), `data[125]` (logo) and `data[126]` (backdrop). *Resolve* by loading `data/data.dat`
   with the compat datafile reader and dumping each object's `w/h`. Nothing needs to run
   for this, only the reader.
2. **Side fill.** What a wider view shows outside the historical world x ∈ [-57, 697): repeated
   wall texture, mirrored stripes, or a neutral backdrop. This is a design decision, and it
   must not consume `new_rand()` or `rand()`.
3. **HUD policy.** Should in-game HUD elements stay tower-attached (historical look,
   `CAMERA_X` offset) or anchor to view edges (section 4)? Decide per element or globally
   with an option.
4. **Particle anchors.** World-emitted and UI-emitted particles share one list. If the tower
   is always centred, a single `CAMERA_X` offset is correct for both. If not, particles need
   an origin tag, which is a source change.
5. **Dialog read-backs.** Seven dialogs snapshot `screen`. The compat `screen` must be a
   readable memory bitmap that holds the last presented logical frame, with `SCREEN_W/H`
   and `gfx_driver->w/h` equal to the logical frame size. If `FRAME_W > 640`, the 639x479
   copies in `my_alert` (1844/1885) and the literal 640x480 snapshots in `profile.c:783-784`
   and `replay.c:977/992` would leave the extra width unsaved or stale.
6. **Pinned sources.** `AGENTS.md` pins the 25 units' source identities in `recovery.json`.
   Should the naming refactor edit `src/` behind a reviewed gate extension (with
   codegen-identical macros), or keep names in port-only headers and wrappers? Needs a
   project decision before any rename.
7. **Debug zoom clamps.** `blit_to_screen` mode 6 clamps x ≤ 520, y ≤ 360 with a 160x120
   source, so it can read past the right and bottom edges. Replicating Allegro's
   `stretch_blit` source clipping only matters for debug parity (low priority).
8. **Ad hit test.** `SCREEN_H - ad->h` against an ad drawn at y 280 agrees only for a
   200-px-tall ad. The ad path is dead without a server, so keep it or stub it.
9. **Simulation RNG.** Unrelated to coordinates but required for "the simulation must not
   change": the map generator uses the C library `rand()` seeded by the replay. Replays
   match only if the port reproduces the historical CRT `rand()` sequence.
