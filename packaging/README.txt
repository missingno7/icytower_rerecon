Icy Tower 1.5.1 - portable SDL3 port, version 1.0
===================================================

A modern port of Icy Tower 1.5.1, rebuilt from a reconstruction of the original
source code. It plays exactly like the original (same physics, same replays)
and adds:

- resizable windows, fullscreen, true widescreen and high-DPI rendering
- smooth motion on high-refresh displays (the game still runs at its fixed 50 Hz)
- gamepad support
- an Android version with touch controls

The original game files are NOT included. You need your own copy of Icy Tower
1.5 (freeware by Free Lunch Design).


WINDOWS
-------
1. Copy icytower-port.exe into your Icy Tower 1.5 folder, the one that
   contains the "data" and "characters" folders. It sits next to the original
   icytower.exe and does not replace it.
2. Start icytower-port.exe.

Your existing profiles, high scores and replays in the game folder are picked
up. Everything the port saves (profiles, replays, high scores, screenshots,
settings) goes to:

    %APPDATA%\IcyTowerPort\IcyTower

so the original installation is never modified.

Settings are in icytower-port.ini in that folder (created on first start,
every option is explained inside). Examples: renderer (modern / faithful),
widescreen, texture filter, interpolation, vsync, frame cap, window size.
Command-line overrides: --fullscreen, --window 1920x1080, --renderer faithful,
--filter linear, --interpolation off, --data "<Icy Tower folder>".


LINUX (x86_64)
--------------
1. Extract the archive and copy icytower-port into your Icy Tower 1.5 folder,
   the one that contains "data" and "characters" (for example the folder of
   a Wine installation), or start it with --data "<your Icy Tower folder>".
2. Run ./icytower-port (mark it executable first if needed:
   chmod +x icytower-port).

Needs a 64-bit distribution from about 2022 on (glibc 2.35, e.g. Ubuntu
22.04), X11 or Wayland, and ALSA, PulseAudio or PipeWire for sound. Saved
data and icytower-port.ini go to ~/.local/share/IcyTowerPort/IcyTower.
File names are matched without regard to case, as on Windows.


ANDROID
-------
1. Install IcyTower-port-v1.0-android.apk (allow installing apps from that
   source when asked).
2. Start it once. It shows the folder where the game data has to go.
3. Connect the phone to a PC over USB and copy the folders "data" and
   "characters" from your Icy Tower 1.5 folder into:

       Internal storage > Android > data > io.github.icytowerport > files

4. Start the game again.

Touch controls: choose Zones, Slide, Tilt or Off in the game under
Options > Controls > Touch. Menus have an on-screen remote (arrows, OK, Back);
the phone's back button works as Esc. Bluetooth gamepads work too.


LICENSES
--------
The port uses SDL 3 (zlib license), libogg and libvorbis (BSD-style), and
code derived from Allegro 4 (giftware) and logg. See the "licenses" folder.
Icy Tower is (c) Free Lunch Design.
