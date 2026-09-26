{
    static int blit_mode;
    if (debug) {
        if (key[KEY_F2]) blit_mode = 0;                      /* 2271 */
        if (key[KEY_F3]) blit_mode = 1;                       /* 2272 */
        if (key[KEY_F4]) blit_mode = 2;                       /* 2273 */
        if (key[KEY_F5]) blit_mode = 3;                       /* 2274 */
        if (key[KEY_F6]) blit_mode = 4;                       /* 2275 */
        if (key[KEY_F7]) blit_mode = 5;                       /* 2276 */
        if (key[KEY_F8]) blit_mode = 6;                       /* 2277 */
    }
    acquire_screen();
    if (!blit_mode) {                                          /* 2282 */
        blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);
    }
    else if (blit_mode == 1) {                                 /* 2285 */
        draw_sprite_h_flip(screen, bmp, 0, 0);                 /* 2286 */
    }
    else if (blit_mode == 2) {                                 /* 2288 */
        draw_sprite_v_flip(screen, bmp, 0, 0);                 /* 2289 */
    }
    else if (blit_mode == 3) {                                 /* 2291 */
        int y;
        for (y = 0; y < 480; y++) {                            /* 2293 */
            int x = fixtoi(fixsin(itofix(y + logic_count * 5)) *
                            ply[player_id]->level);              /* 2294 */
            blit(bmp, screen, 0, y, x, y, 640, 1);
        }
    }
    else if (blit_mode == 4) {                                 /* 2297 */
        int y = ply[player_id]->level % 480;                    /* 2298 */
        line(bmp, 0, 479, 639, 479, 0);
        line(bmp, 0, 0, 639, 0, 0);
        blit(bmp, screen, 0, 0, 0, y, bmp->w, bmp->h);           /* 2301 */
        blit(bmp, screen, 0, 0, 0, y - 480, bmp->w, bmp->h);     /* 2302 */
    }
    else if (blit_mode == 5) {                                 /* 2304 */
        int x, y;
        double delta;
        delta = ply[player_id]->x - 160.0;
        if (delta <= 0.0) x = 0;
        else if (delta > 320.0) x = 320;
        else x = (int)delta;
        delta = ply[player_id]->y - 160.0;
        if (delta <= 0.0) y = 0;
        else if (delta > 240.0) y = 240;
        else y = (int)delta;
        stretch_blit(bmp, screen, x, y, 320, 240, 0, 0, 640, 480); /* 2307 */
    }
    else if (blit_mode == 6) {                                 /* 2309 */
        int x, y;
        double delta;
        delta = ply[player_id]->x - 80.0;
        if (delta <= 0.0) x = 0;
        else if (delta > 520.0) x = 520;
        else x = (int)delta;
        delta = ply[player_id]->y - 80.0;
        if (delta <= 0.0) y = 0;
        else if (delta > 360.0) y = 360;
        else y = (int)delta;
        stretch_blit(bmp, screen, x, y, 160, 120, 0, 0, 640, 480); /* 2312 */
    }
    release_screen();
}