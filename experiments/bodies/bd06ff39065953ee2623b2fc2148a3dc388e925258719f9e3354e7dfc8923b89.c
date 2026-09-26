{
    Tplayer *p;
    int floor_y, floor_x1, floor_x2;
    int left_x, left_y, right_x, right_y;
    int left, right;
    int current_x, current_y;
    int plx1, plx2, prx1, prx2;
    int col1, col2;

    p = ply[player_id];
    plx1 = (int)p->x - 11;
    current_x = (int)p->x;
    current_y = (int)p->y + 1;
    plx2 = lastX - 11;
    prx1 = (int)p->x + 11;
    prx2 = lastX + 11;
    col1 = makecol(255, 0, 0);
    col2 = makecol(255, 255, 0);
    floor_y = -12345678;
    floor_x1 = 0;
    floor_x2 = 0;
    getFloorData(&map, (int)p->y, &floor_y, &floor_x1, &floor_x2);
    if (floor_y == -12345678) {
        getFloorData(&map, lastY, &floor_y, &floor_x1, &floor_x2);
        if (floor_y == -12345678) {
            floor_y = 0;
            floor_x1 = 0;
            floor_x2 = 0;
        }
    }

    if (debug) {
        if (key[KEY_F2]) {
            line(screen, floor_x1, floor_y, floor_x2, floor_y, col1);
            line(screen, plx1, current_y, plx2, lastY, col2);
            line(screen, prx1, current_y, prx2, lastY, col2);
        }
    }
    left = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        plx1, current_y, plx2, lastY, &left_x, &left_y);
    right = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        prx1, current_y, prx2, lastY, &right_x, &right_y);
    if (!left && !right) {
        left = line_intersect(floor_x1, floor_y + 4, floor_x2, floor_y + 4,
            plx1, current_y, plx2, lastY, &left_x, &left_y);
        right = line_intersect(floor_x1, floor_y + 4, floor_x2, floor_y + 4,
            prx1, current_y, prx2, lastY, &right_x, &right_y);
    }
    if (!left && !right) {
        if (p->status == 2 || p->status == 0)
            p->status = 3;
        p->edge = 0;
        return;
    }
    p->edge = left == right ? 0 : (left ? 1 : 2);
    if (!right) return;
    if (p->status != 2 && p->status != 3)
        return;

    play_sound(sounds[8], 1, 1);
    p->status = 0;
    p->sy = 0;
    p->y = floor_y - 1;
    p->x = left ? left_x + 11 : right_x - 11;
    p->rotate = 0;
}