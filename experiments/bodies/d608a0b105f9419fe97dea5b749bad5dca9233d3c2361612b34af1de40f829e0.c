{
    int left, right;
    int fx1, fy1, fx2, fy2;
    int plx1, ply1, plx2, ply2;
    int prx1, pry1, prx2, pry2;
    int ilx, ily, irx, iry;

    fx1 = 0;
    fx2 = 0;
    prx1 = (int)ply[player_id]->x;
    pry1 = (int)ply[player_id]->y;
    fy1 = -12345678;
    getFloorData(&map, pry1, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
            if (fy1 == -12345678) {
            if (ply[player_id]->status == 2 || ply[player_id]->status == 0)
                ply[player_id]->status = 3;
            return;
        }
    }
    fy2 = fy1;
    plx1 = prx1 - 11;
    ply1 = pry1 + 1;
    plx2 = lastX - 11;
    prx1 = prx1 + 11;
    prx2 = lastX + 11;

    if (debug) {
        if (key[KEY_F2]) {
            int col1 = makecol(255, 0, 0);
            int col2 = makecol(255, 255, 0);
            line(screen, fx1, fy1, fx2, fy2, col1);
            line(screen, plx1, ply1, plx2, lastY, col2);
            line(screen, prx1, ply1, prx2, lastY, col2);
        }
    }
    left = line_intersect(fx1, fy1, fx2, fy2,
        plx1, ply1, plx2, lastY, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy2,
        prx1, ply1, prx2, lastY, &irx, &iry);
    if (left + right == 0) {
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0) {
            ply[player_id]->status = 3;
            if (left == right) {
                ply[player_id]->edge = 0;
                return;
            }
        }
    }
    if (left == right)
        ply[player_id]->edge = 0;
    else
        ply[player_id]->edge = (left == 1) ? 1 : 2;
    if (left + right == 0 ||
        (ply[player_id]->status != 2 && ply[player_id]->status != 3))
        return;
    if (left && right &&
        (ilx < -10000 || irx < -10000 || ilx > 10000 || irx > 10000))
        return;

    play_sound(sounds[8], 1, 1);
    ply[player_id]->status = 0;
    ply[player_id]->sy = 0;
    ply[player_id]->y = fy1 - 1;
    ply[player_id]->x = left ? ilx + 11 : irx - 11;
    ply[player_id]->rotate = 0;
}