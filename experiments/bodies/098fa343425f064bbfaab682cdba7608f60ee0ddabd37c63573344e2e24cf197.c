{
    int left, right;
    int fx1, fy1, fx2, fy2;
    int plx1 = (int)ply[player_id]->x - 11;
    int ply1 = (int)ply[player_id]->y + 1;
    int plx2 = lastX - 11;
    int ply2 = lastY;
    int prx1 = (int)ply[player_id]->x + 11;
    int pry1 = (int)ply[player_id]->y + 1;
    int prx2 = lastX + 11;
    int pry2 = lastY;
    int ilx, ily, irx, iry;
    int col1 = makecol(255, 0, 0);
    int col2 = makecol(255, 255, 0);
    int solid1, solid2;

    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    any11 = solid1;
    any12 = solid2;
    any21 = any22 = any23 = 0;
    if (solid1 + solid2 == 0) {
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0) ply[player_id]->status = 3;
    }
    else if (ply[player_id]->status != 1) if (ply[player_id]->status != 2) {
        if (ply[player_id]->status) play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        if (solid1) ply[player_id]->y -= solid1 - 9999;
        else if (solid2) ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        if (solid1 != solid2) ply[player_id]->edge = solid1 ? 1 : 2;
        else ply[player_id]->edge = 0;
        return;
    }

    fy1 = -12345678;
    getFloorData(&map, (int)ply[player_id]->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) fx1 = fx2 = 0;
    }
    fy2 = fy1;
    if (debug && key[KEY_F2]) {
        line(screen, fx1, fy1, fx2, fy2, col1);
        line(screen, plx1, ply1, plx2, ply2, col2);
        line(screen, prx1, pry1, prx2, pry2, col2);
    }
    left = line_intersect(fx1, fy1, fx2, fy2, plx1, ply1, plx2, ply2, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy2, prx1, pry1, prx2, pry2, &irx, &iry);
    if (left + right == 0) {
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0)
            ply[player_id]->status = 3;
    }
    if (left != right) ply[player_id]->edge = left ? 1 : 2;
    else ply[player_id]->edge = 0;
    if (left + right != 0 && (ply[player_id]->status == 2 || ply[player_id]->status == 3)) {
        play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        ply[player_id]->y = fy1 - 1;
        ply[player_id]->x = !left ? irx - 11 : ilx + 11;
        ply[player_id]->rotate = 0;
    }
}