{
    int left, right;
    int fx1, fy1, fx2, fy2;
    int plx1, ply1, plx2, ply2;
    int prx1, pry1, prx2, pry2;
    int ilx, ily, irx, iry;
    int col1, col2;
    int solid1, solid2;
    double px, py;
    px = ply[player_id]->x;
    py = ply[player_id]->y;
    col1 = makecol(255, 0, 0);
    col2 = makecol(255, 255, 0);
    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    any11 = solid1;
    any12 = solid2;
    any23 = 0;
    any22 = 0;
    any21 = 0;
    if (solid1 + solid2 == 0) {
        switch (ply[player_id]->status) {
        case 2:
        case 0:
            ply[player_id]->status = 3;
            break;
        }
        fy1 = -12345678;
        getFloorData(&map, (int)ply[player_id]->y, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) {
            getFloorData(&map, lastY, &fy1, &fx1, &fx2);
            if (fy1 == -12345678) {
                fy1 = 0;
                fx1 = 0;
                fx2 = 0;
            }
        }
        plx1 = (int)px - 11;
        ply1 = (int)py + 1;
        plx2 = lastX - 11;
        ply2 = lastY;
        prx1 = (int)px + 11;
        pry1 = (int)py + 1;
        prx2 = lastX + 11;
        pry2 = lastY;
        if (debug) {
            if (key[KEY_F2]) {
                line(screen, fx1, fy1, fx2, fy1, col1);
                line(screen, plx1, ply1, plx2, ply2, col2);
                line(screen, prx1, pry1, prx2, pry2, col2);
            }
        }
        left = line_intersect(fx1, fy1, fx2, fy1,
            plx1, ply1, plx2, ply2, &ilx, &ily);
        right = line_intersect(fx1, fy1, fx2, fy1,
            prx1, pry1, prx2, pry2, &irx, &iry);
        if (!left && !right) {
            ply[player_id]->edge = 0;
            return;
        }
        ply[player_id]->edge = left == right ? 0 : (left ? 1 : 2);
        if (ply[player_id]->status != 2 && ply[player_id]->status != 3)
            return;

        play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        ply[player_id]->y = fy1 - 1;
        ply[player_id]->x = left ? ilx + 11 : irx - 11;
        ply[player_id]->rotate = 0;
    } else {
        if (ply[player_id]->status == 1 || ply[player_id]->status == 2)
                    return;
                if (ply[player_id]->status)
                    play_sound(sounds[8], 1, 1);
                ply[player_id]->status = 0;
                ply[player_id]->sy = 0;
                if (solid1) {
                    ply[player_id]->y -= solid1 - 9999;
                    ply[player_id]->rotate = 0;
                    ply[player_id]->edge = solid1 == solid2 ? 0 : 1;
                    return;
                }
                ply[player_id]->y -= solid2 - 9999;
                ply[player_id]->rotate = 0;
                ply[player_id]->edge = 2;
                return;
    }
}