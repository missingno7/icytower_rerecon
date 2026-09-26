{
    int rp;
    unsigned char flags;
    unsigned char key_flags;

    if (!control)
        return;
    if (recording) {
        poll_control(control,0);
        if (ply[player_id]->dead) {
            demo->data[rec_pos+1].key_flags=0x80;
            demo->data[rec_pos+1].cycle_count=0;
            demo->data[rec_pos+2].key_flags=0;
            demo->data[rec_pos+2].cycle_count=0;
        }
        else {
            key_flags=demo->data[rec_pos].key_flags;
            if (key_flags&0x80) {
                demo->data[rec_pos+1].key_flags=0x80;
                demo->data[rec_pos+1].cycle_count=0;
                demo->data[rec_pos+2].key_flags=0;
                demo->data[rec_pos+2].cycle_count=0;
            }
            else {
                flags=control->flags&0x93;
                if (key_flags==flags)
                    demo->data[rec_pos].cycle_count++;
                else {
                rec_pos++;
                demo->data[rec_pos].key_flags=flags;
                    demo->data[rec_pos].cycle_count=0;
                }
            }
        }
    }
    else {
        rp=rec_pos-1;
        if (rp>=0) {
            if (rp>=demo->size)
                control->flags=0;
            else {
                control->flags=demo->data[rp].key_flags;
                if (demo->data[rp].cycle_count>0)
                    demo->data[rp].cycle_count--;
                else
                    rec_pos++;
            }
        }
        else
            rec_pos++;
    }

    if (is_left(control)) {
        if (ply[player_id]->sx>0)
            ply[player_id]->sx*=0.7;
        ply[player_id]->sx-=0.3;
    }
    else if (is_right(control)) {
        if (ply[player_id]->sx<0)
            ply[player_id]->sx*=0.7;
        ply[player_id]->sx+=0.3;
    }
    else
        ply[player_id]->sx*=0.9;

    if (!rejump) {
        if (is_fire(control)) {
            if (!ply[player_id]->jump_key) {
                if (jump_player(ply[player_id],0)) {
                    ply[player_id]->jump_key=-1;
                    play_jump_sound(ply[player_id]);
                    if (profile)
                        profile->total_jumps++;
                }
            }
        }
        if (!is_fire(control))
            ply[player_id]->jump_key=0;
    }
    else {
        if (is_fire(control)) {
            if (jump_player(ply[player_id],0)) {
                play_jump_sound(ply[player_id]);
                if (profile)
                    profile->total_jumps++;
            }
        }
    }
}