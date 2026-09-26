{
    static int blit_mode;`n    int x, y, stretch_w, stretch_h;
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
    if (!blit_mode) {
        blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);
        goto finish;
    }
    else if (blit_mode == 1) {
        draw_sprite_h_flip(screen, bmp, 0, 0);
        goto finish;
    }
    else if (blit_mode == 2) {
        draw_sprite_v_flip(screen, bmp, 0, 0);
        goto finish;
    }
    else if (blit_mode == 3) {
        int y;
        for (y = 0; y < 480; y++) {
            int x = fixtoi(fixsin(itofix(y + logic_count * 5)) * ply[player_id]->level);
            blit(bmp, screen, 0, y, x, y, 640, 1);
        }
        goto finish;
    }
    else if (blit_mode == 4) {
        int y = ply[player_id]->level % 480;
        blit(bmp, screen, 0, 0, 0, y, bmp->w, bmp->h);
        blit(bmp, screen, 0, 0, 0, y - 480, bmp->w, bmp->h);
        goto finish;
    }
    else if (blit_mode == 5) {
        double dx = ply[player_id]->x - 160.0;
        double dy = ply[player_id]->y - 160.0;
        if (dx <= 0.0) x = 0;
        else if (dx > 320.0) x = 320;
        else x = (int)dx;
        if (dy <= 0.0) y = 0;
        else if (dy > 240.0) y = 240;
        else y = (int)dy;
        stretch_w = 320;
        stretch_h = 240;
        goto zoom_blit;
    }
    else if (blit_mode == 6) {
        double dx = ply[player_id]->x - 80.0;
        double dy = ply[player_id]->y - 80.0;
        if (dx <= 0.0) x = 0;
        else if (dx > 520.0) x = 520;
        else x = (int)dx;
        if (dy <= 0.0) y = 0;
        else if (dy > 360.0) y = 360;
        else y = (int)dy;
        stretch_w = 160;
        stretch_h = 120;
    }
    else goto finish;
zoom_blit:
    stretch_blit(bmp, screen, x, y, stretch_w, stretch_h, 0, 0, 640, 480);
finish:    release_screen();
}