{
    int left, right;
    int fx1, fy1, fx2, fy2;
    int plx1, ply1, plx2, ply2;
    int prx1, pry1, prx2, pry2;
    int ilx, ily, irx, iry;
    int col1, col2;

    plx1 = (int)ply[player_id]->x - 11;
    ply1 = (int)ply[player_id]->y + 1;
    plx2 = lastX - 11;
    prx1 = (int)ply[player_id]->x + 11;
    pry1 = ply1;
    prx2 = lastX + 11;
    ply2 = lastY;
    pry2 = lastY;
    col1 = makecol(255, 0, 0);
    col2 = makecol(255, 255, 0);
    fy1 = -12345678;
    fy2 = -12345678;
    getFloorData(&map, (int)ply[player_id]->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) {
            fy2 = 0;
            fx1 = 0;
            fx2 = 0;
        }
    }

    fy2 = fy1;

    if (debug) {
        if (key[KEY_F2]) {
            line(screen, fx1, fy1, fx2, fy2, col1);
            line(screen, plx1, ply1, plx2, ply2, col2);
            line(screen, prx1, pry1, prx2, pry2, col2);
        }
    }
    left = line_intersect(fx1, fy1, fx2, fy2,
        plx1, ply1, plx2, ply2, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy2,
        prx1, pry1, prx2, pry2, &irx, &iry);
    if (!left && !right) {
        left = line_intersect(fx1, fy1 + 4, fx2, fy2 + 4,
            plx1, ply1, plx2, ply2, &ilx, &ily);
        right = line_intersect(fx1, fy1 + 4, fx2, fy2 + 4,
            prx1, pry1, prx2, pry2, &irx, &iry);
    }
    if (left + right == 0) {
        if (ply[player_id]->status == 0)
            ply[player_id]->status = 3;
    }
    ply[player_id]->edge = left == right ? 0 : (left ? 1 : 2);
    if (right && ply[player_id]->status >= 2 && ply[player_id]->status <= 3) {
            play_sound(sounds[8], 1, 1);
            ply[player_id]->status = 0;
            ply[player_id]->sy = 0;
            ply[player_id]->y = fy1 - 1;
            ply[player_id]->x = left ? ilx + 11 : irx - 11;
            ply[player_id]->rotate = 0;
    }
}