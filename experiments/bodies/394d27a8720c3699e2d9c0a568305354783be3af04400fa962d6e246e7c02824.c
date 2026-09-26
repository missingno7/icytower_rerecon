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
    switch (blit_mode) {
    case 0:
        blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);
        break;
    case 1:
        draw_sprite_h_flip(screen, bmp, 0, 0);
        break;
    case 2:
        draw_sprite_v_flip(screen, bmp, 0, 0);
        break;
    case 3: {
        int y;
        for (y = 0; y < 480; y++) {
            int x = fixtoi(fixsin(itofix(y + logic_count * 5)) * ply[player_id]->level);
            blit(bmp, screen, 0, y, x, y, 640, 1);
        }
        break;
    }
    case 4: {
        int y = ply[player_id]->level % 480;
        blit(bmp, screen, 0, 0, 0, y, bmp->w, bmp->h);
        blit(bmp, screen, 0, 0, 0, y - 480, bmp->w, bmp->h);
        break;
    }
    case 5: {
        double dx = ply[player_id]->x - 160.0;
        double dy = ply[player_id]->y - 160.0;
        int x, y;
        if (dx <= 0.0) x = 0;
        else if (dx > 320.0) x = 320;
        else x = (int)dx;
        if (dy <= 0.0) y = 0;
        else if (dy > 240.0) y = 240;
        else y = (int)dy;
        stretch_blit(bmp, screen, x, y, 320, 240, 0, 0, 640, 480);
        break;
    }
    case 6: {
        double dx = ply[player_id]->x - 80.0;
        double dy = ply[player_id]->y - 80.0;
        int x, y;
        if (dx <= 0.0) x = 0;
        else if (dx > 520.0) x = 520;
        else x = (int)dx;
        if (dy <= 0.0) y = 0;
        else if (dy > 360.0) y = 360;
        else y = (int)dy;
        stretch_blit(bmp, screen, x, y, 160, 120, 0, 0, 640, 480);
        break;
    }
    }
    release_screen();
}