{
    int x, y, dx, dy;
    int solid1, solid2;

    /* 3246 */
    x = (int)ply[player_id]->x;
    dx = lastX - x;
    dx = dx < 0 ? -dx : dx;
    /* 3247 */
    y = (int)ply[player_id]->y;
    dy = lastY - y;
    dy = dy < 0 ? -dy : dy;
    /* 3248 */
    if ((int)ply[player_id]->x < lastX) x = lastX - dx / 2;
    /* 3249 */
    else x = lastX + dx / 2;
    /* 3250 */
    if ((int)ply[player_id]->y < lastY) y = lastY - dy / 2;
    /* 3251 */
    else y = lastY + dy / 2;

    /* 3253 */
    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    /* 3254 */
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    /* 3255 */
    any11 = solid1;
    /* 3256 */
    any12 = solid2;
    /* 3257 */
    any23 = 0;
    any22 = 0;
    any21 = 0;
    /* 3258 */
    if (solid1 + solid2 == 0) {
        /* 3259 */
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0)
            ply[player_id]->status = 3;
        /* 3270 */
        if (y <= lastY)
            return;
        goto sweep;
    }

resolve:
    /* 3260 */
    if (ply[player_id]->status == 1 || ply[player_id]->status == 2)
        return;
    /* 3261 */
    if (ply[player_id]->status)
        play_sound(sounds[8], 1, 1);
    /* 3262 */
    ply[player_id]->status = 0;
    /* 3263 */
    ply[player_id]->sy = 0;
    /* 3264 */
    if (solid1) {
        ply[player_id]->y -= solid1 - 9999;
        /* 3266 */
        ply[player_id]->rotate = 0;
        /* 3267 */
        if (solid1 == solid2)
            ply[player_id]->edge = 0;
        else
            ply[player_id]->edge = 1;
        return;
    }
    if (solid2) {
        ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        ply[player_id]->edge = 2;
        return;
    }
    ply[player_id]->rotate = 0;
    ply[player_id]->edge = 0;
    return;

sweep:
    /* 3271 */
    solid1 = is_solid(&map, x - 11, y);
    /* 3272 */
    solid2 = is_solid(&map, x + 11, y);
    /* 3273 */
    any21 = solid1;
    /* 3274 */
    any22 = solid2;
    /* 3275 */
    if (solid1 + solid2 == 0) {
        /* 3276 */
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0)
            ply[player_id]->status = 3;
        return;
    }
    /* 3277 */
    if (ply[player_id]->status == 1 || ply[player_id]->status == 2)
        return;
    /* 3278 */
    any23 = 1;
    /* 3279 */
    if (ply[player_id]->status)
        play_sound(sounds[8], 1, 1);
    /* 3280 */
    ply[player_id]->status = 0;
    /* 3281 */
    ply[player_id]->sy = 0;
    /* 3282 */
    if (solid1) {
        ply[player_id]->y -= solid1 - 9999;
        /* 3284 */
        ply[player_id]->rotate = 0;
        /* 3285 */
        if (solid1 == solid2)
            ply[player_id]->edge = 0;
        else
            ply[player_id]->edge = 1;
        return;
    }
    /* 3283 */
    if (solid2) {
        ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        ply[player_id]->edge = 2;
        return;
    }
    /* 3286 */
    ply[player_id]->rotate = 0;
    ply[player_id]->edge = 0;
    return;
}