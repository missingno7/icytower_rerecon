{
    Tplayer *p;
    int left, right;
    int fx1, fy1, fx2, fy2;
    int plx1, ply1, plx2, ply2;
    int prx1, pry1, prx2, pry2;
    int ilx, ily, irx, iry;
    int col1, col2;

    p = ply[player_id];
    plx1 = (int)p->x - 11;
    ply1 = (int)p->y + 1;
    ply2 = lastY;
    plx2 = lastX - 11;
    prx1 = (int)p->x + 11;
    pry1 = ply1;
    prx2 = lastX + 11;
    pry2 = lastY;
    col1 = makecol(255, 0, 0);
    col2 = makecol(255, 255, 0);
    fy1 = -12345678;
    fx1 = 0;
    fx2 = 0;
    getFloorData(&map, (int)p->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) {
            fy1 = 0;
            fx1 = 0;
            fx2 = 0;
        }
    }

    if (debug) {
        if (key[KEY_F2]) {
            line(screen, fx1, fy1, fx2, fy1, col1);
            line(screen, plx1, ply1, plx2, ply2, col2);
            line(screen, prx1, ply1, prx2, pry2, col2);
        }
    }
    left = line_intersect(fx1, fy1, fx2, fy1,
        plx1, ply1, plx2, ply2, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy1,
        prx1, ply1, prx2, pry2, &irx, &iry);
    if (!left && !right) {
        left = line_intersect(fx1, fy1 + 4, fx2, fy1 + 4,
            plx1, ply1, plx2, ply2, &ilx, &ily);
        right = line_intersect(fx1, fy1 + 4, fx2, fy1 + 4,
            prx1, ply1, prx2, pry2, &irx, &iry);
    }
    if (!left && !right) {
        if (p->status == 2 || p->status == 0)
            p->status = 3;
        p->edge = 0;
        return;
    }
    p->edge = left == right ? 0 : (left ? 1 : 2);
    if (p->status != 2 && p->status != 3)
        return;

    play_sound(sounds[8], 1, 1);
    p->status = 0;
    p->sy = 0;
    p->y = fy1 - 1;
    p->x = left ? ilx + 11 : irx - 11;
    p->rotate = 0;
}