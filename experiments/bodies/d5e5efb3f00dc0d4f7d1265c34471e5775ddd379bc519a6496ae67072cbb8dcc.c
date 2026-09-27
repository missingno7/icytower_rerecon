{
    int x;
    int y;
    int p_im;
    int flip;
    int cx;
    int cy;
    int ls;
    int fo;
    int so;
    int max_bg_id;
    BITMAP *customFrame;
    int oy;
    int ox;

    fo = profile->start_floor * 3 + 0x11;
    so = profile->start_floor + 0x65;

    frame_count++;

    max_bg_id = 2;
    if (ply[player_id]->level > 200) max_bg_id = 3;
    if (ply[player_id]->level > 350) max_bg_id = 4;
    if (ply[player_id]->level > 600) max_bg_id = 5;

    {

        while ((map.offset / 256) > last_stripe_y) {
            last_stripe_y++;

            bg_stripe_ids[4] = bg_stripe_ids[3];
            bg_stripe_ids[3] = bg_stripe_ids[2];
            bg_stripe_ids[2] = bg_stripe_ids[1];
            bg_stripe_ids[1] = bg_stripe_ids[0];

            if (new_rand() % 100 > 0x28)
                goto reset_stripe;
            bg_stripe_ids[0] = new_rand() % max_bg_id;
            if (bg_stripe_ids[0] != bg_stripe_ids[1] &&
                bg_stripe_ids[0] != bg_stripe_ids[2])
                goto skip_reset;
        reset_stripe:
            bg_stripe_ids[0] = 0;
        skip_reset:
            ;
        }

        for (p_im = -1; p_im != 4; p_im++) {
            BITMAP *stripe = data[bg_stripe_ids[p_im + 1] + 1].dat;
            blit(stripe, bmp, 0, 0, 0x25,
                 p_im * 0x80 + (map.offset % 0x100) / 2,
                 stripe->w, stripe->h);
        }
    }

    if ((unsigned)(hurry_y + 0x63) <= 0x242 && options.flash != 2) {
        BITMAP *hspr = data[67].dat;

        draw_sprite(bmp, hspr, 320 - hspr->w / 2, hurry_y);
    }

    {


        for (y = 31; y >= 0; y--) {
            Tfloor *room = &map.room[y];
            cx = 464 - y * 16;
            if (!room->empty) {
                int f;

                ls = fo + room->tiles * 3;
                if (ls > 0x2c) {
                    ls = 0x2c;
                }
                if (room->level > 0x1387) {
                    ls += 3;
                }

                f = room->start_tile;

                draw_sprite(bmp, data[ls].dat, f * 16 - 5, cx + (map.offset % 16) - 6);

                f++;
                for (; f < room->end_tile; f++) {
                    draw_sprite(bmp, data[ls + 1].dat, f * 16, cx + (map.offset % 16) - 6);
                }

                draw_sprite(bmp, data[ls + 2].dat, f * 16, cx + (map.offset % 16) - 6);

                if (debug && key[KEY_F2]) {
                    textprintf_ex(bmp, font, 0x208, cx + (map.offset % 16), 15, -1, "%d",
                                  (room->level - 1) / 5);
                }
            }

            if (room->sign) {
                int s;
                int sy;
                int sw;
                int c1;
                int c2;

                s = so + room->tiles;
                if (s > 0x6e) {
                    s = 0x6e;
                }
                if (room->level > 0x1387) {
                    s++;
                }
                sy = cx + (map.offset % 16) + 10;
                sw = ((BITMAP *)data[s].dat)->w;
                cy = room->start_tile + (room->end_tile - room->start_tile) / 2;
                cy *= 16;
                draw_sprite(bmp, data[s].dat, cy, sy);
                c1 = makecol(255, 255, 255);
                c2 = makecol(55, 55, 55);
                cy += sw / 2;

                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 7, c2, -1, "%d", room->sign);
                textprintf_centre_ex(bmp, data[54].dat, cy + 2, sy + 6, c2, -1, "%d", room->sign);
                textprintf_centre_ex(bmp, data[54].dat, cy,     sy + 6, c2, -1, "%d", room->sign);
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 5, c2, -1, "%d", room->sign);
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 6, c1, -1, "%d", room->sign);
            }

        }
    }

    yr (y = 0; y < 512; y++) {
        if (stars[y].intensity) {
            draw_sprite(swap_screen, data[stars[y].color + 0x75].dat, fixtoi(stars[y].x), fixtoi(stars[y].y));
        }
    }

    p_im = 6;
    if (ply[player_id]->status) {
        if (ply[player_id]->status == 3) {
            if (ply[player_id]->sy > 3.0) p_im = 7;
        }
        if (ply[player_id]->status == 2) {
            if (ply[player_id]->sy > 3.0) p_im = 7;
        } else if (ply[player_id]->status == 1) {
            if (ply[player_id]->sy < -3.0) p_im = 5;
        }
    } else {
        p_im = ABS(ply[player_id]->sx) < 0.02 ? 0 : 1;
    }
    if (p_im >= 5 && p_im <= 7) {
        if (ABS(ply[player_id]->sx) < 0.01) p_im = 8;
    }
    if (p_im != 1) {
        ply[player_id]->frame = 0;
    } else {
        if (ABS(ply[player_id]->sx) < 0.2) ply[player_id]->frame = 0;
        if (ply[player_id]->frame > 3) ply[player_id]->frame = 0;
    }

    customFrame = custom.frame[0];
    oy = 1 - customFrame->h;

    if (!p_im) {
        if (ply[player_id]->edge) {
            customFrame = custom.frame[14 - ((logic_count & 8) != 0)];
            if (ply[player_id]->edge == 2) {
                draw_sprite_h_flip(bmp, customFrame,
                    (int)ply[player_id]->x - customFrame->w + 11,
                    (int)ply[player_id]->y + oy);
            } else {
                draw_sprite(bmp, customFrame, (int)ply[player_id]->x - 11,
                    (int)ply[player_id]->y + oy);
            }
        }

        else {
            if (map.offset > 0xc8 && ply[player_id]->y > 400.0)
                customFrame = custom.frame[11];
            else if (logic_count <= 0xb)
                customFrame = custom.frame[9];
            else if (logic_count > 0x18 && logic_count <= 0x24)
                customFrame = custom.frame[10];
            ox = -(customFrame->w / 2);
            if (ply[player_id]->sx > 0.0)
                draw_sprite(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
            else
                draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        }
    } else {
        flip = ply[player_id]->rotate;
        if (flip) {
        customFrame = custom.frame[12];
        rotate_sprite(bmp, customFrame, (int)ply[player_id]->x, (int)ply[player_id]->y,
                       ply[player_id]->angle);

        } else {
        customFrame = custom.frame[p_im + ply[player_id]->frame];

        ox = -(customFrame->w / 2);

        if (ply[player_id]->sx > 0)
            draw_sprite(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        else
            draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        }
    }

    for (cy = -124; cy != 496; cy += 124) {
        draw_sprite(bmp, data[100].dat, 565, cy + (int)((map.offset % 84) * 1.476));
        draw_sprite_h_flip(bmp, data[100].dat, -57, cy + (int)((map.offset % 84) * 1.476));
    }

    draw_sprite(bmp, data[16].dat, 22, 100);

    if (ply[player_id]->in_combo) {
        blit(data[15].dat, bmp, 0, 100 - ply[player_id]->in_combo, 33,
             219 - ply[player_id]->in_combo, 16, ply[player_id]->in_combo);
        draw_sprite(bmp, data[14].dat, -8, 210);
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d",
                              ply[player_id]->acc_level);
    } else if (reward_time) {
        draw_sprite(bmp, data[14].dat, -8, 210);
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d",
                              ply[player_id]->latest_combo);
    }

    if (hurry_y < 251 || hurry_y > 479) {
        x = 6;
        y = 10;
        cx = 0;
        cy = 0;
    } else {
        cx = logic_count % 3 - 1;
        cy = (logic_count + 1) % 3 - 1;
        y = cy + 10;
        x = cx + 6;
    }
    draw_sprite(bmp, data[12].dat, x, y);

    if (hurry_y >= 201 && hurry_y <= 479) {
        cx = (logic_count + 2) % 3 - 1;
        cy = (logic_count + 3) % 3 - 1;
    }
    rotate_sprite(bmp, data[13].dat, cx + 34, cy + 28,
                  clock_angle ? ftofix((clock_angle % 1500) * 0.1706666) : 0);

    if (reward_time) {
        draw_reward(swap_screen);
    }

    textprintf_ex(bmp, data[52].dat, 8, 440, -1, -1, "score: %d",
                  ply[player_id]->level * 10 + ply[player_id]->score);

    if (!recording) {
        char myBuf[256];

        if (frame_count & 8) {
            strcpy(myBuf, "REPLAY");
            ls = 630 - text_length(data[53].dat, myBuf);
            textprintf_ex(bmp, data[53].dat, ls + 1, 5, makecol(0, 0, 0), -1, "REPLAY");
            textprintf_ex(bmp, data[53].dat, ls, 4, makecol(255, 255, 255), -1, "REPLAY");
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
            cx = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 36, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, cx, 35, makecol(255, 255, 255), -1);
        }

        int myPos;
        int len;
        BITMAP *vcr;
        char scrollerText[70];

        vcr = data[127].dat;
        myPos = rec_pos;
        len = demo->size;
        ox = 0x27b - vcr->w;
        y = 0x1db - vcr->h;

        draw_sprite(bmp, vcr, ox, y);

        if (!ply[player_id]->dead) {

            if (is_left(&ctrl))
                draw_sprite(bmp, data[128].dat, ox + 0x61, y + 5);
            if (is_fire(&ctrl))
                draw_sprite(bmp, data[130].dat, ox + 0x6b, y + 5);
            if (is_right(&ctrl))
                draw_sprite(bmp, data[129].dat, ox + 0x75, y + 5);
        }

        cx = y + 0xa;
        cy = ox + 0xa;
        set_clip_rect(bmp, cy, 0, 0x26f, 0x1df);

        sprintf(scrollerText, "%s%s%s", demo->name, demo->comment[0] ? " - " : "",
                !demo->comment[0] ? "" : demo->comment);

        textout_ex(bmp, data[53].dat, demo->name, ox + 0xc - scroll_count / 2,
            cx + 4, makecol(150, 150, 160), -1);
        textout_ex(bmp, data[53].dat, demo->name, ox + 0xd - scroll_count / 2,
            cx + 4, makecol(200, 200, 210), -1);
        if (demo->comment[0]) {
            textout_ex(bmp, data[53].dat, " - ",
                ox + 0xc - scroll_count / 2 + text_length(data[53].dat, demo->name),
                cx + 4, makecol(200, 200, 210), -1);
            textout_ex(bmp, data[53].dat, demo->comment,
                ox - scroll_count / 2 + 0x1e + text_length(data[53].dat, demo->name),
                cx + 4, makecol(200, 200, 210), -1);
        }
        set_clip_rect(bmp, 0, 0, 0x27f, 0x1df);
        if (demo->comment[0]) {
            if (scroll_delay > 0) {
                scroll_delay--;
            }
            else {
                scroll_count++;
                if (scroll_count / 2 > text_length(data[53].dat, scrollerText))
                    scroll_count = -250;
            }
        }

        rect(bmp, cy, cx + 0x14,
            cy + (myPos * 117 / len > 0x74 ? 0x74 : myPos * 117 / len),
            cx + 0x13, makecol(50, 200, 50));
    }

    if (debug && key[KEY_F2]) {
            textprintf_ex(bmp, font, 0, 0, 15, -1, "FPS:%6d / %d", fps, lps);
            textprintf_ex(bmp, font, 0, 0xa, 15, -1, "REC:%6d / %d", rec_pos,
                demo->size);
            textprintf_ex(bmp, font, 0, 0x14, 15, -1, "    %6d  (%d) ",
                demo->data[rec_pos].key_flags, demo->data[rec_pos].cycle_count);
            textprintf_ex(bmp, font, 0xc8, 0, 15, -1, "POS: %d, %d",
                (int)ply[player_id]->x, (int)ply[player_id]->y);
            textprintf_ex(bmp, font, 0xc8, 0xa, 15, -1, " dx: %1.2f",
                ply[player_id]->sx);
            textprintf_ex(bmp, font, 0xc8, 0x14, 15, -1, "rjp: %d",
                options.jump_hold);
            textprintf_ex(bmp, font, 0x190, 0, 15, -1, "any: %6d %6d %6d",
                any11, any12, any13);
            textprintf_ex(bmp, font, 0x190, 0xa, 15, -1, "any: %6d %6d %6d",
                any21, any22, any23);
        }

}