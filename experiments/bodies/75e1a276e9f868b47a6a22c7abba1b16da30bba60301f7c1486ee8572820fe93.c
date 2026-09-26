{
    int midX, midY, dX, dY;
    int solid1, solid2;

    dX = lastX - (int)ply[player_id]->x; if (dX < 0) dX = -dX;
    dY = lastY - (int)ply[player_id]->y; if (dY < 0) dY = -dY;
    if ((int)ply[player_id]->x < lastX) midX = lastX - dX / 2;
    else midX = lastX + dX / 2;
    if ((int)ply[player_id]->y < lastY) midY = lastY - dY / 2;
    else midY = lastY + dY / 2;

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
    if (midY > lastY) {
        solid1 = is_solid(&map, midX - 11, midY);
        solid2 = is_solid(&map, midX + 11, midY);
        any21 = solid1;
        any22 = solid2;
        if (solid1 + solid2 == 0) {
            if (ply[player_id]->status == 2 || ply[player_id]->status == 0) ply[player_id]->status = 3;
        }
        else if (ply[player_id]->status != 1) if (ply[player_id]->status != 2) {
            any23 = 1;
            if (ply[player_id]->status) play_sound(sounds[8], 1, 1);
            ply[player_id]->status = 0;
            ply[player_id]->sy = 0;
            if (solid1) ply[player_id]->y -= solid1 - 9999;
            else if (solid2) ply[player_id]->y -= solid2 - 9999;
            ply[player_id]->rotate = 0;
            if (solid1 != solid2) ply[player_id]->edge = solid1 ? 1 : 2;
            else ply[player_id]->edge = 0;
        }
    }
}