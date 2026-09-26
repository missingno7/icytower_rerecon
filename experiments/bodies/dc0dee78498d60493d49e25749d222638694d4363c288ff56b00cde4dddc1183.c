{
    Tplayer *p;
    int floor_y;
    int floor_x1, floor_x2;
    int left_x, left_y, right_x, right_y;
    int left, right;
    int current_x, current_y;

    p = ply[player_id];
    current_x = (int)p->x;
    current_y = (int)p->y;
    floor_y = -12345678;
    floor_x1 = 0;
    floor_x2 = 0;
    getFloorData(&map, current_y, &floor_y, &floor_x1, &floor_x2);
    if (floor_y == -12345678) {
        getFloorData(&map, lastY, &floor_y, &floor_x1, &floor_x2);
        if (floor_y == -12345678) {
            if (p->status == 2 || p->status == 0)
                p->status = 3;
            return;
        }
    }

    if (debug) {
        if (key[KEY_F2]) {
            int col1 = makecol(255, 0, 0);
            int col2 = makecol(255, 255, 0);
            line(screen, floor_x1, floor_y, floor_x2, floor_y, col1);
            line(screen, current_x - 11, current_y + 1, lastX - 11, lastY, col2);
            line(screen, current_x + 11, current_y + 1, lastX + 11, lastY, col2);
        }
    }
    left = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        current_x - 11, current_y + 1, lastX - 11, lastY, &left_x, &left_y);
    right = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        current_x + 11, current_y + 1, lastX + 11, lastY, &right_x, &right_y);
    if (!left && !right) {
        if (p->status == 2 || p->status == 0)
            p->status = 3;
        return;
    }
    p->edge = left == right ? 0 : (left ? 1 : 2);
    if (p->status != 2 && p->status != 3)
        return;
    if (left && right &&
        (left_x < -10000 || right_x < -10000 || left_x > 10000 || right_x > 10000))
        return;

    play_sound(sounds[8], 1, 1);
    p->status = 0;
    p->sy = 0;
    p->y = floor_y - 1;
    p->x = left ? left_x + 11 : right_x - 11;
    p->rotate = 0;
}