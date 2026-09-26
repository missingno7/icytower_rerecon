{
    int solid1;
    int solid2;
    int hit_status;
    Tplayer *status_player;

    solid1=is_solid(&map,(int)ply[player_id]->x-11,(int)ply[player_id]->y);
    solid2=is_solid(&map,(int)ply[player_id]->x+11,(int)ply[player_id]->y);
    any11=solid1;
    any12=solid2;
    any23=0;
    any22=0;
    any21=0;
    if (solid1+solid2==0) {
        if (ply[player_id]->status==2 || ply[player_id]->status==0)
            ply[player_id]->status=3;
        return;
    }
    status_player = ply[player_id];
    hit_status = status_player->status;
    if (hit_status==1) return;
    if (hit_status==2) return;
    if (hit_status)
        play_sound(sounds[8],1,1);
    ply[player_id]->status=0;
    ply[player_id]->sy=0;
    if (!solid1) {
        if (solid2) {
            ply[player_id]->y-=solid2-9999;
            ply[player_id]->rotate=0;
            ply[player_id]->edge=2;
            return;
        }
        ply[player_id]->rotate=0;
        ply[player_id]->edge=0;
        return;
    }
    ply[player_id]->y-=solid1-9999;
    ply[player_id]->rotate=0;
    if (solid1==solid2) {
        ply[player_id]->edge=0;
        return;
    }
    ply[player_id]->edge=1;
    return;
}