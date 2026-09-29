package io.github.icytowerport;

import org.libsdl.app.SDLActivity;

/* SDL's activity runs the game's SDL_main from libmain.so (built by the
 * repository's CMakeLists.txt) on its own thread. */
public class IcyTowerActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "main" };
    }
}
