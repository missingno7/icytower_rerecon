# SDL3 backend and Android builds: preparation

Goal: run the reconstructed game on SDL3 (desktop first, then Android) while keeping the 25
historical units as the behavioural reference. The reconstruction repository stays what it is
(the historical build and its proofs); the port lives beside it and links the same 25 units
against a compatibility layer instead of Allegro 4.4.1.

## Measured API surface

`tools/port_inventory.py` scans the 25 units against the pinned Allegro 4.4.1 headers and writes
`docs/sdl3/allegro-api-usage.json`. Result: 127 Allegro functions, 15 globals, 8 types and
53 constants are used (`name` in the list is a struct field, not the Allegro macro). Grouped:

| area | functions (use counts in the JSON) | notes for the layer |
|---|---|---|
| bitmaps and drawing | create_bitmap(_ex), create_sub_bitmap, destroy_bitmap, blit, masked_blit, stretch_blit, draw_sprite(_h_flip/_v_flip), draw_trans_sprite, rotate_sprite, rotate_scaled_sprite, stretch_sprite, rectfill, rect, line, putpixel, getpixel, clear_bitmap, clear_to_color, set_clip_rect, acquire/release_bitmap, acquire/release_screen, vsync, is_memory_bitmap, bitmap_color_depth | software BITMAP in memory (8/16/32 bpp as the data files need), one SDL texture upload of `screen` per frame; `screen` is a 640x480 BITMAP |
| colour and blending | makecol(_depth), getr/getg/getb/geta (15/16/24/32), set_trans_blender, set_alpha_blender, drawing_mode, solid_mode, set_color_conversion, get_color_conversion, set_color_depth, desktop_color_depth, select_palette, set_palette, get_palette, generate_332_palette, _rgb_scale_6, _color_load_depth, _fixup_loaded_bitmap | keep Allegro's colour math exactly (it is the reference); palette handling is used only for 8-bit loading |
| text | textout_ex, textout_centre_ex, textout_right_ex, text_length, text_height, `font` | Allegro FONT from the datafile; glyph blitting in software |
| datafiles and files | load_datafile(_callback), unload_datafile, register_datafile_object, DAT_ID, load_bitmap, save_bitmap, register_bitmap_file_type, pack_fopen/fread/fwrite/fclose, packfile_password, exists, file_exists, file_size_ex, delete_file, get_filename, get_extension, replace_extension, replace_filename, get_executable_name, for_each_file_ex, set_config_file, get_config_string | the datafile reader and the LZSS unpacker must be ported verbatim from Allegro (assets are `.dat`); PACKFILE maps onto SDL_IOStream |
| input | key[], keypressed, readkey, clear_keybuf, simulate_keypress, install_keyboard, install_mouse, mouse_x/y/b, show_mouse, select_mouse_cursor, enable_hardware_cursor, install_joystick, poll_joystick, joy[] | scancode table KEY_* to SDL_Scancode; `key[]` refreshed from SDL events each frame; Android needs on-screen controls mapped into `key[]` |
| timing | install_timer, install_int, rest | the game's `cycle_count` timer callback drives logic at a fixed rate; implement `install_int` with an SDL timer or a monotonic-clock accumulator, `rest` with SDL_Delay |
| sound | install_sound, play_sample, stop_sample, adjust_sample, destroy_sample, load_sample, load_wav, voice_stop, voice_get_position, set_volume, play_midi, stop_midi, load_midi, destroy_midi, logg (ogg decode) | software mixer with SDL3 audio streams; ogg via stb_vorbis or libvorbis; MIDI is used once (menu) and can be stubbed or rendered |
| system | set_gfx_mode, gfx_driver, set_display_switch_mode/callback, set_close_button_callback, allegro_exit, alert, allegro_errno, END_OF_MAIN, SWITCH_* | window/fullscreen through SDL; focus callbacks from SDL window events |
| fixed point | itofix, fixtoi, ftofix, fixtof, fixsin | header inlines, copy verbatim |

Types: BITMAP, SAMPLE, DATAFILE, FONT, PACKFILE, RGB, fixed, GFX_DRIVER. The layer must keep the
struct layouts the game touches (BITMAP `w`, `h`, `line[]`, `dat`; DATAFILE `dat`, `size`).

## Design: `compat/allegro4-sdl3`

1. One C library exposing `allegro.h`-compatible declarations for exactly the surface above, so
   the 25 units compile unchanged. Anything not in the inventory is left out on purpose.
2. Rendering is software into memory BITMAPs, exactly as Allegro did, and `screen` is presented
   with one texture update per frame. This keeps pixel results identical to the reference build,
   which matters for replays and for later equivalence tests.
3. Behavioural verification: the same replay files must play identically on the historical build
   and on the SDL3 build. Add a headless mode (the game already has `itrcheck`, the replay checker)
   and compare its XML output between both builds in CI.
4. Files: profiles, replays, config and screenshots move under `SDL_GetPrefPath`; the data files
   ship read-only (`SDL_GetBasePath`, or Android assets through SDL_IOStream).

## Android

- Build with SDL3's Android project template (Gradle + CMake); the 25 units plus the layer are
  plain C99, no Windows headers once the layer replaces `winalleg.h`/`aintwin.h` includes in
  main.c (those two includes are the only Win32-specific lines in the game sources; the
  QueryPerformanceCounter use in `play` needs a portable clock in the layer).
- 640x480 logical size rendered with letterboxing (`SDL_SetRenderLogicalPresentation`).
- Touch: an overlay that sets `key[KEY_LEFT/RIGHT/SPACE/...]` and feeds `readkey` for menus.
- Audio: SDL3 audio streams; keep the mixer in the layer so sample priorities match.
- Ads module (fld_adspot, httpget) is dead code without a server; keep it compiled, it is
  harmless, or stub the network call.

## Order of work

1. Layer skeleton with bitmaps, datafile loading, text, keyboard, timer, `set_gfx_mode`; get to
   the main menu on desktop.
2. Sound and gameplay; verify replays against the historical build with the replay checker.
3. Android project, touch overlay, file locations.
4. Only then consider touching game sources, and only outside the 25 historical units' proven code.
