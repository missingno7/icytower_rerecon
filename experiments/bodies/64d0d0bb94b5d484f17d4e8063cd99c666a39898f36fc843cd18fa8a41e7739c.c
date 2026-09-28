{ int x, y;
    int p_im;
    int flip;

    int cx, cy;
    int ls;

    int fo = profile->start_floor * 3 + 17;
    int so = profile->start_floor + 101;

    frame_count++;


    int max_bg_id = 2;
    if (ply[player_id]->level > 200) max_bg_id = 3;
    if (ply[player_id]->level > 350) max_bg_id = 4;
    if (ply[player_id]->level > 600) max_bg_id = 5;


    while (map.offset / 256 > last_stripe_y) {
        last_stripe_y++;


        bg_stripe_ids[4] = bg_stripe_ids[3]; bg_stripe_ids[3] = bg_stripe_ids[2]; bg_stripe_ids[2] = bg_stripe_ids[1]; bg_stripe_ids[1] = bg_stripe_ids[0];


        if (new_rand() % 100 > 40)
            bg_stripe_ids[0] = 0;
        else {

            bg_stripe_ids[0] = new_rand() % max_bg_id;
            if (bg_stripe_ids[0] == bg_stripe_ids[1] || bg_stripe_ids[0] == bg_stripe_ids[2])
                bg_stripe_ids[0] = 0;
        }
    }



    for (ls = -1; ls < 4; ls++)
        blit(data[bg_stripe_ids[ls + 1] + 1].dat, bmp, 0, 0, 37, ls * 128 + (map.offset % 256) / 2, ((BITMAP *)data[bg_stripe_ids[ls + 1] + 1].dat)->w, ((BITMAP *)data[bg_stripe_ids[ls + 1] + 1].dat)->h);










    if (hurry_y > -100 && hurry_y < 480 && options.flash != 2)
        draw_sprite(bmp, data[67].dat, 320 - ((BITMAP *)data[67].dat)->w / 2, hurry_y);




    for (y = 31; y >= 0; y--) { cx = 464 - y * 16;
        if (!map.room[y].empty) {
            int f = fo + map.room[y].tiles * 3; if (f > 44) f = 44;
            if (map.room[y].level > 4999) f += 3;
            x = map.room[y].start_tile;
            draw_sprite(bmp, data[f].dat, x * 16 - 5, cx + (map.offset % 16) - 6);
            x++;
            while (x < map.room[y].end_tile) {
                draw_sprite(bmp, data[f + 1].dat, x * 16, cx + (map.offset % 16) - 6);
                x++;
            }
            draw_sprite(bmp, data[f + 2].dat, x * 16, cx + (map.offset % 16) - 6);

            if (debug && key[KEY_F2]) textprintf_ex(bmp, font, 520, cx + (map.offset % 16), 15, -1, "%d", (map.room[y].level - 1) / 5);
        }
        if (map.room[y].sign) {
            int s = so + map.room[y].tiles; if (s > 110) s = 110;
            if (map.room[y].level > 4999) s++;
            int sy = cx + (map.offset % 16) + 10;
            int sw = ((BITMAP *)data[s].dat)->w;
            cy = (map.room[y].start_tile + (map.room[y].end_tile - map.room[y].start_tile) / 2) * 16;
            draw_sprite(bmp, data[s].dat, cy, sy);
            int c1 = makecol(255, 255, 255);
            int c2 = makecol(55, 55, 55);
            cy += sw / 2; textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 7, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy + 2, sy + 6, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy, sy + 6, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 5, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 6, c1, -1, "%d", map.room[y].sign);
        }
    }


    for (fo = 0; fo < 512; fo++)
        if (stars[fo].intensity)
            draw_sprite(swap_screen, data[stars[fo].color + 117].dat, fixtoi(stars[fo].x), fixtoi(stars[fo].y));






    p_im = 6; if (!ply[player_id]->status) p_im = 1;
    if (ply[player_id]->status == 3 && ply[player_id]->sy > 3.0) p_im = 7;
    if (ply[player_id]->status == 2 && ply[player_id]->sy > 3.0) p_im = 7;
    if (ply[player_id]->status == 1 && ply[player_id]->sy < -3.0) p_im = 5;

    if (!ply[player_id]->status) { if (ABS(ply[player_id]->sx) < 0.02) p_im = 0; }
    if (p_im >= 5 && p_im <= 7 && ABS(ply[player_id]->sx) < 0.01) p_im = 8;

    if (p_im != 1 || ABS(ply[player_id]->sx) < 0.2) ply[player_id]->frame = 0;
    else
    if (ply[player_id]->frame > 3) ply[player_id]->frame = 0;





    BITMAP *customFrame = custom.frame[0];
    int oy = 1 - customFrame->h;
    int ox = 0;

    if (!p_im) {

        if (ply[player_id]->edge) {
            if (logic_count & 8) p_im = 13; else p_im = 14;


            customFrame = custom.frame[p_im];

            if (ply[player_id]->edge == 2)
                draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x - customFrame->w + 11, (int)ply[player_id]->y + oy);




            else
                draw_sprite(bmp, customFrame, (int)ply[player_id]->x - 11, (int)ply[player_id]->y + oy);
        }
        else {


            if (map.offset > 200 && ply[player_id]->y > 400.0) customFrame = custom.frame[11];
            else if (logic_count <= 11) customFrame = custom.frame[9];
            else if (logic_count > 24)
                if (logic_count <= 36) customFrame = custom.frame[10];



            ox = -(customFrame->w / 2);

            if (ply[player_id]->sx > 0.0) draw_sprite(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy); else draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        }
    }
    else {

        if (ply[player_id]->rotate) {
            customFrame = custom.frame[12]; cx = customFrame->w / 2;
            rotate_sprite(bmp, customFrame, (int)ply[player_id]->x - cx, (int)ply[player_id]->y - 8 - custom.frame[0]->h, ply[player_id]->angle);


        } else {
            customFrame = custom.frame[p_im + ply[player_id]->frame];
            ox = -(customFrame->w / 2);
            if (ply[player_id]->sx > 0.0) draw_sprite(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy); else draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        }
    }












































    for (ls = -1; ls < 4; ls++) {
        draw_sprite(bmp, data[100].dat, 565, ls * 124 + (int)((map.offset % 84) * 1.476)); draw_sprite_h_flip(bmp, data[100].dat, -57, ls * 124 + (int)((map.offset % 84) * 1.476));
    }




    draw_sprite(bmp, data[16].dat, 22, 100);
    if (ply[player_id]->in_combo) {
        blit(data[15].dat, bmp, 0, 100 - ply[player_id]->in_combo, 33, 219 - ply[player_id]->in_combo, 16, ply[player_id]->in_combo);
        draw_sprite(bmp, data[14].dat, -8, 210);
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d", ply[player_id]->acc_level);
    }
    else if (reward_time) {
        draw_sprite(bmp, data[14].dat, -8, 210);
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d", ply[player_id]->latest_combo);
    }


    if (hurry_y < 251 || hurry_y > 479) { cx = 0; cy = 0; } else { cx = logic_count % 3 - 1; cy = (logic_count + 1) % 3 - 1; }
    draw_sprite(bmp, data[12].dat, cx + 6, cy + 10);
    if (hurry_y >= 201 && hurry_y <= 479) { cx = (logic_count + 2) % 3 - 1; cy = (logic_count + 3) % 3 - 1; }
    rotate_sprite(bmp, data[13].dat, cx + 34, cy + 28, clock_angle ? ftofix((clock_angle % 1500) * 0.1706666) : 0);
    if (reward_time)
        draw_reward(swap_screen);
















    textprintf_ex(bmp, data[52].dat, 8, 440, -1, -1, "score: %d", ply[player_id]->level * 10 + ply[player_id]->score);


    if (!recording) {
        char myBuf[256];
        int myPos = 0;

        if (frame_count & 8) {
            strcpy(myBuf, "REPLAY");
            myPos = 630 - text_length(data[53].dat, myBuf);
            textprintf_ex(bmp, data[53].dat, myPos + 1, 5, makecol(0, 0, 0), -1, "REPLAY");
            textprintf_ex(bmp, data[53].dat, myPos, 4, makecol(255, 255, 255), -1, "REPLAY");
        }


        if (is_playing_custom_game) {
            sprintf(myBuf, "%s Floors", floor_size_selection.caption[demo->floor_size]);
            cx = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 16, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, cx, 15, makecol(255, 255, 255), -1);

            sprintf(myBuf, "%s Speed", scroll_speed_selection.caption[demo->start_speed]);
            cx = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 26, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, cx, 25, makecol(255, 255, 255), -1);

            strcpy(myBuf, gravity_selection.caption[demo->gravity]);
            myPos = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, myPos + 1, 36, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, myPos, 35, makecol(255, 255, 255), -1);
        }



        BITMAP *vcr = data[127].dat;
        myPos = rec_pos; int len = demo->size;

        x = 635 - vcr->w;
        y = 475 - vcr->h;
        draw_sprite(bmp, vcr, x, y);
        if (!ply[player_id]->dead) {
            if (is_left(&ctrl)) draw_sprite(bmp, data[128].dat, x + 97, y + 5);
            if (is_fire(&ctrl)) draw_sprite(bmp, data[130].dat, x + 107, y + 5);
            if (is_right(&ctrl)) draw_sprite(bmp, data[129].dat, x + 117, y + 5);
        }
        cx = y + 10;
        cy = x + 10; set_clip_rect(bmp, cy, 0, 623, 479);
        char scrollerText[70];
        sprintf(scrollerText, "%s%s%s", demo->name, demo->comment[0] ? " - " : "", !demo->comment[0] ? "" : demo->comment);
        textout_ex(bmp, data[53].dat, demo->name, x + 12 - scroll_count / 2, cx + 4, makecol(150, 150, 160), -1);
        textout_ex(bmp, data[53].dat, demo->name, x + 13 - scroll_count / 2, cx + 4, makecol(200, 200, 210), -1);
        if (demo->comment[0]) {
            textout_ex(bmp, data[53].dat, " - ", x + 12 - scroll_count / 2 + text_length(data[53].dat, demo->name), cx + 4, makecol(200, 200, 210), -1);
            textout_ex(bmp, data[53].dat, demo->comment, x + 30 - scroll_count / 2 + text_length(data[53].dat, demo->name), cx + 4, makecol(200, 200, 210), -1);
        }
        set_clip_rect(bmp, 0, 0, 639, 479);

        if (demo->comment[0]) {
            if (scroll_delay > 0)
                scroll_delay--;
            else {

                scroll_count++;
                if (scroll_count / 2 > text_length(data[53].dat, scrollerText))
                    scroll_count = -250;
            }
        }


        rect(bmp, cy, cx + 20, cy + (myPos * 117 / len > 116 ? 116 : myPos * 117 / len), cx + 19, makecol(50, 200, 50));
    }


    if (debug && key[KEY_F2]) {
        textprintf_ex(bmp, font, 0, 0, 15, -1, "FPS:%6d / %d", fps, lps);
        textprintf_ex(bmp, font, 0, 10, 15, -1, "REC:%6d / %d", rec_pos, demo->size);
        textprintf_ex(bmp, font, 0, 20, 15, -1, "    %6d  (%d) ", demo->data[rec_pos].key_flags, demo->data[rec_pos].cycle_count);
        textprintf_ex(bmp, font, 200, 0, 15, -1, "POS: %d, %d", (int)ply[player_id]->x, (int)ply[player_id]->y);
        textprintf_ex(bmp, font, 200, 10, 15, -1, " dx: %1.2f", ply[player_id]->sx);
        textprintf_ex(bmp, font, 200, 20, 15, -1, "rjp: %d", options.jump_hold);
        textprintf_ex(bmp, font, 400, 0, 15, -1, "any: %6d %6d %6d", any11, any12, any13);
        textprintf_ex(bmp, font, 400, 10, 15, -1, "any: %6d %6d %6d", any21, any22, any23);
    }
}