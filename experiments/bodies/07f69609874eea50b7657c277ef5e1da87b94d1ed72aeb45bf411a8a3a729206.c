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


    fo = profile->start_floor * 3 + 0x11;              /* 2498 */
    so = profile->start_floor + 0x65;                  /* 2499 */

    frame_count++;                                     /* 2501 */

    if (ply[player_id]->level > 0xc8) {
        if (ply[player_id]->level > 0x15e) {
            max_bg_id = (ply[player_id]->level >= 0x259) + 4;
        } else {
            max_bg_id = 3;
        }
    } else {
        max_bg_id = 2;
    }

    {
        /* 2510: row = floor(map.offset / 256), rounding toward -inf for negative offsets
         * (the js/lea 0xff/sar sequence is the historical bias-then-shift idiom). */


        if (((map.offset < 0) ? ((map.offset + 0xff) >> 8) : (map.offset >> 8)) > last_stripe_y) {                      /* 2510 */
            last_stripe_y++;                             /* 2511 */

            bg_stripe_ids[4] = bg_stripe_ids[3];         /* 2514 */
            bg_stripe_ids[3] = bg_stripe_ids[2];
            bg_stripe_ids[2] = bg_stripe_ids[1];
            bg_stripe_ids[1] = bg_stripe_ids[0];

            /* 2523: only ONE physical `movl $0x0,bg_stripe_ids` exists (offsets 336..352),
             * reached both by the >0x28 branch falling straight through and by the
             * collision-detected branch's own jump (offset 372/380 both target 4093ec,
             * the same address) -- a genuine control-flow merge, not two compiled copies,
             * so the reset is written once and reached from both predecessors. */
            if (new_rand() % 100 > 0x28)                  /* 2517 */
                goto reset_stripe;
            bg_stripe_ids[0] = new_rand() % max_bg_id;     /* 2521 */
            if (bg_stripe_ids[0] != bg_stripe_ids[1] &&    /* 2522 */
                bg_stripe_ids[0] != bg_stripe_ids[2])
                goto skip_reset;
        reset_stripe:
            bg_stripe_ids[0] = 0;                          /* 2523 */
        skip_reset:
            ;
        }

        for (p_im = 0; p_im != 4; p_im++) {                        /* 2529 */
            BITMAP *stripe = data[bg_stripe_ids[p_im + 1] + 1].dat; /* 2530 */
            blit(stripe, bmp, 0, 0, 0x25,
                 p_im * 0x80 + (map.offset % 0x100) / 2,
                 stripe->w, stripe->h);
        }
    }

    if (hurry_y + 0x63 <= 0x242 && options.flash != 2) {  /* 2541 */
        BITMAP *hspr = data[67].dat;   /* ? historical index 67 (0x4dd23c + 0x430) not named by
                                         * function_data_refs beyond the generic "data" table */
        draw_sprite(bmp, hspr, 320 - hspr->w / 2, hurry_y); /* 2542 */
    }

    {
        /* 2547: real loop, evidenced by the back-edge test `cmpl $0x1e0,-0x178(%ebp)` (cx)
         * at offset 1580 sitting AFTER the sign-text block (main.c:2562..2575) but tagged
         * to line 2547 -- a rotated for-loop whose test/increment sit at the bottom. `esi`
         * (the per-row Tfloor pointer) is decremented by sizeof(Tfloor)==0x18 each pass
         * (offset 1570 `sub $0x18,%esi`) in lockstep with cx += 0x10 (offset 1573), for
         * 0x1e0/0x10 == 30 rows. Field offsets (empty=0, start_tile=4, end_tile=8, level=12,
         * sign=16, tiles=20) match Tfloor exactly for BOTH the floor body below (2548..2560)
         * and the sign body carried into D2 (2562..2575): they are the SAME loop over the
         * SAME `room` pointer, so this loop opens here and its closing brace is in D2.c. */
        Tfloor *room = &map.room[31]; /* ? starting row; map.room[32], matches the historical sign scan bound */

        for (cx = 0; cx != 0x1e0; cx += 0x10) {      /* 2547 */
            if (!room->empty) {                       /* 2548 */
                int f;   /* DWARF: block-scoped int at -0x180(ebp); no separate `rowy`/`tile`
                          * names are declared by the historical DWARF for this block, so the
                          * tile index reuses this one slot and the row-y term is recomputed
                          * inline (cx + (map.offset & 0xf) - 6) at each use, 2552. Each of the
                          * three draws (2552/2555/2558) is ~20 bytes short of its historical
                          * count for the same reason as 2606: the line table charges each with
                          * a `dec/or $0xfffffff0/inc` abs()-style tail duplicated far away from
                          * its local computation (2552's local copy at 566..623 plus a second
                          * copy at 2212..2217; 2555's at 740..780 plus 1964..1969; 2558's at
                          * 826..876 plus 2224..2229) -- one statement, two compiled copies from
                          * -O2 block layout, not a missing branch. */

                ls = fo + room->tiles * 3;   /* 2549 */
                if (ls > 0x2c) {
                    ls = 0x2c;
                }
                if (room->level > 0x1387) {  /* 2550 */
                    ls += 3;
                }

                f = room->start_tile;     /* 2551 */

                /* left edge tile */
                cy = f * 16 - 5;          /* 2552, stored at -0x174(%ebp) */
                x = cy;                      /* ? DWARF tracks a separate `x` over this same span; mirrored here */
                draw_sprite(bmp, data[ls].dat, x, cx + (map.offset & 0xf) - 6);

                f++;                                  /* 2553 */
                for (; f < room->end_tile; f++) {      /* 2554 */
                    cy = f * 16;                        /* 2555 */
                    x = cy;
                    draw_sprite(bmp, data[ls + 1].dat, x, cx + (map.offset & 0xf) - 6); /* 2555 */
                }

                cy = f * 16;                            /* 2558 */
                x = cy;
                draw_sprite(bmp, data[ls + 2].dat, x, cx + (map.offset & 0xf) - 6);     /* 2558 */

                if (debug && !key[KEY_F2]) {  /* 2560 */
                    textprintf_ex(bmp, font, 0x208, cx + (map.offset & 0xf), 15, -1, "%d",
                                  (room->level - 1) / 10);
                }
            }

            /* lines 2562..2575 continue this same per-row loop (sign text) in D2.c;
             * loop closes and `room` is decremented at the end of D2.c's chunk. */
    /* still inside the D1 per-row loop for 2562..2575, then combo text / rewards for 2589..2651 */
            if (room->sign) {                  /* 2562, same `room` row as the floor draw above */
                int s;
                int sy;
                int sw;
                int c1;
                int c2;

                s = so + room->tiles;          /* 2563 */
                if (s > 0x6e) {
                    s = 0x6e;
                }
                if (room->level > 0x1387) {    /* 2564 */
                    s++;
                }
                sy = cx + (map.offset & 0xf) + 10;  /* 2565 */
                sw = ((BITMAP *)data[s].dat)->w;     /* 2566 */
                cy = room->start_tile + (room->end_tile - room->start_tile) / 2; /* 2567 */
                cy *= 16;
                draw_sprite(bmp, data[s].dat, cy, sy);
                c1 = makecol(255, 255, 255);   /* 2569 */
                c2 = makecol(55, 55, 55);      /* 2570 */
                cy += sw / 2;                  /* 2571 */
                /* shadowed stripe-number text: four gray offsets then one white on top */
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 7, c2, -1, "%d", room->sign); /* 2571 */
                textprintf_centre_ex(bmp, data[54].dat, cy + 2, sy + 6, c2, -1, "%d", room->sign); /* 2572 */
                textprintf_centre_ex(bmp, data[54].dat, cy,     sy + 6, c2, -1, "%d", room->sign); /* 2573 */
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 5, c2, -1, "%d", room->sign); /* 2574 */
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 6, c1, -1, "%d", room->sign); /* 2575 */
            }

            room--;                            /* offset 1570 `sub $0x18,%esi`, sizeof(Tfloor) */
        }   /* closes the for (cx = 0; ...) loop opened in D1.c (2547) */
    }       /* closes the `{ Tfloor *room = ...; ... }` scope opened in D1.c */

    for (fo = 0; fo < 512; fo++) {          /* 2580: loop index reuses fo; not otherwise evidenced as the star-loop counter */
        if (stars[fo].intensity) {           /* 2581 */
            draw_sprite(swap_screen, data[stars[fo].color + 0x75].dat, fixtoi(stars[fo].x), fixtoi(stars[fo].y));  /* 2582 */
        }
    }

    /* 2589: status==0 branches to a separate narrow-speed check at original offset 2256.
     * That path reaches the edge sprite directly for -0.02<sx<0.02, or reaches the
     * ordinary p_im=1 speed/frame route outside that band. Other statuses select
     * their own pose bases before the 2594 range test. */
    /* 2590/2591/2595/2597 all show the same shape as 2606's gap: the line table charges each of
     * these comparisons far more than one `fldl/fucompp/fnstsw/test` sequence costs (2590: 42 vs
     * our 9; 2591 has its own separate -3.0/6/7 arm so is not the same statement), because -O2
     * duplicates the fcompp+branch sequence at every predecessor edge that reaches it (status==3's
     * arm at offsets 1819..1852 plus a second copy at 3356..3365; 2595's inner-band test appears
     * at 2561..2608 AND again at 3323..3346; 2597's edge==0/-0.2 test appears at 1869..1924,
     * 2308..2315, 2608..2615 AND 3365..3391) -- one source comparison, several compiled copies from
     * jump-threading. These duplicated instruction fragments do not by themselves prove
     * extra source branches; the remaining function difference still needs investigation. */
    if (ply[player_id]->status) {                       /* 2589 */
        if (ply[player_id]->status == 3) {                /* 2590 */
            if (ply[player_id]->sy > 3.0)              /* 2590: fucompp/fnstsw compare direction inferred, not asserted */
                p_im = 7;
            else
                p_im = 6;                                   /* offset 1852's own `mov $0x6,%esi` */
        }
        else if (ply[player_id]->status == 2) {           /* 2591 */
            if (ply[player_id]->sy > 3.0)               /* 2591 */
                p_im = 7;
            else
                p_im = 6;                                   /* offset 2556's own `mov $0x6,%esi` */
        }
        else if (ply[player_id]->status == 1) {            /* 2592 */
            if (ply[player_id]->sy < -3.0)               /* 2592 */
                p_im = 5;
            else
                p_im = 6;
        }
    } else {
        if (ply[player_id]->sx >= 0.0) {
            if (ply[player_id]->sx < 0.02) {
                ply[player_id]->frame = 0;
                customFrame = custom.frame[0];
                oy = 1 - customFrame->h;
                goto edge_sprite;
            }
        } else if (ply[player_id]->sx > -0.02) {
            ply[player_id]->frame = 0;
            customFrame = custom.frame[0];
            oy = 1 - customFrame->h;
            goto edge_sprite;
        }
        goto p_im_one_path;
    }
    /* 2594: the status-zero route bypasses this pose-range check. */
    if ((unsigned)(p_im - 5) <= 2) {          /* 2594: range test on p_im */
        if (ply[player_id]->sx > -0.01 && ply[player_id]->sx < 0.01)
            p_im = 8;                        /* 2595: strict band includes zero */
    }

    if (p_im != 1) {
        ply[player_id]->frame = 0;
    } else {
p_im_one_path:
        if (ply[player_id]->sx < 0.2 && ply[player_id]->sx > -0.2)
            ply[player_id]->frame = 0;
        if (ply[player_id]->frame > 3)
            ply[player_id]->frame = 0;
        p_im = 1;
    }

    /* 2605: no comparison precedes the custom.frame[0] loads at 2315 and 2615,
     * so the earlier null-guard guess is dropped. */
    /* 2606: original `1 - custom.frame[0]->h` fragments appear at offsets 1924,
     * 2321/2621, 3391, and 6051. The current source emits a different layout;
     * attribution of the remaining deficit to compiler duplication alone is unproved. */
    customFrame = custom.frame[0];                                  /* 2605: default for edge==0 */
    oy = 1 - customFrame->h;                                        /* 2606 */

    /* 2609: offset 2631's `test %esi,%esi; je` enters the edge-sprite path at
     * 2329 when p_im==0. Edge sprite calls jump to 2819, after the ordinary
     * rotate/frame draw. The earlier candidate inverted this condition and
     * drew through both paths. */
    if (!p_im) {                                         /* 2609 */
edge_sprite:
        if (ply[player_id]->edge) {                      /* 2611 */
            customFrame = (logic_count & 8) ? custom.frame[13] : custom.frame[14]; /* 2612/2615 */
            oy = (int)ply[player_id]->y + oy;           /* 2624 */
            if (ply[player_id]->edge == 2) {            /* 2617 */
                ox = (int)ply[player_id]->x - customFrame->w + 0xb;
                draw_sprite_h_flip(bmp, customFrame, ox, oy);
            } else {
                ox = (int)ply[player_id]->x - 0xb;
                draw_sprite(bmp, customFrame, ox, oy);
            }
        }
        /* Only edge==0 enters this block at original offset 3115. Both edge==1
         * and edge==2 sprite calls jump to offset 2819 afterward. */
        else {
            if (map.offset > 0xc8 && ply[player_id]->y > 400.0)
                customFrame = custom.frame[11];         /* 2629 */
            else if (logic_count <= 0xb)
                customFrame = custom.frame[9];          /* 2630 */
            else if (logic_count > 0x18 && logic_count <= 0x24)
                customFrame = custom.frame[10];         /* 2631/2632 */
            ox = -(customFrame->w / 2);                 /* 2636 */
            oy = (int)ply[player_id]->y + oy;
            ox = (int)ply[player_id]->x + ox;
            if (ply[player_id]->sx == 0.0)             /* 2638 */
                draw_sprite(bmp, customFrame, ox, oy);
            else
                draw_sprite_h_flip(bmp, customFrame, ox, oy);
        }
    } else {
        flip = ply[player_id]->rotate;                  /* 2643 */
        if (flip) {
        customFrame = custom.frame[12];                 /* 2644: bypasses the p_im+frame index entirely */
        rotate_sprite(bmp, customFrame, (int)ply[player_id]->x, (int)ply[player_id]->y,
                       ply[player_id]->angle);            /* draw.inl:345, offset 6639..6852;
                                                             x/y args not fully traced -- the 200-byte
                                                             inline body wasn't walked past its w/h loads */
        } else {
        customFrame = custom.frame[p_im + ply[player_id]->frame];   /* 2649/2650: offset 2663 `add 0x3c(%edx),%esi`
                                                            * adds ply->frame onto the still-live p_im (esi,
                                                            * location-list range 2561..2666 covers this add),
                                                            * then offset 2666 indexes custom.frame[] with it --
                                                            * p_im is not dead, it is the pose base index itself,
                                                            * so the 2594 range test that keeps it in [5,8] guards
                                                            * a real array bound, not a provably-redundant compare */
        ox = -(customFrame->w / 2);
        /* 2651: fldl 0x10(%edx)/fldz/fucompp guards the draw, then the fistpl-truncated y and x
         * are added onto the running oy/ox and the draw call issued -- offsets 2686..2786, all one
         * historical source line per function_lines --source-view. */
        /* 2651: sx!=0.0 arm (jne target 0x40b189 = offset 7917) reuses this same
         * ox/oy computation (ecx/edi carry the untruncated oy/ox bases through
         * unchanged, edx stays &ply[player_id] for both the x/y loads and, after
         * the vtable reload, esi stays customFrame) and only swaps the call from
         * *0x44(vtable) (draw_sprite) to *0x50(vtable) (draw_sprite_h_flip,
         * offset 7917..8024), rejoining the same 409d9f continuation either way. */
        oy = (int)ply[player_id]->y + oy;            /* 2651 */
        ox = (int)ply[player_id]->x + ox;            /* 2651 */
        if (ply[player_id]->sx == 0)                  /* 2651 */
            draw_sprite(bmp, customFrame, ox, oy);       /* 2651, draw.inl:238 */
        else
            draw_sprite_h_flip(bmp, customFrame, ox, oy);  /* 2651, draw.inl:280, offset 7917..8024 */
        }
    }


    /* lines 2652..2697: no main.c line-table rows in this offset range (the
     * instructions here -- draw.inl:238/280 inline draw_sprite/draw_sprite_h_flip
     * calls plus reused fragments of main.c:2560/2590-2606/2629-2638 -- are the
     * last unrolled iteration of the scroller/background-layer loop whose source
     * text belongs to D1/D2's declared ranges, not D3's; nothing to add here. */

    /* lines 2698..2699: the scrolling side strips, reconstructed from offsets 2786..3105.
     * The loop base in %esi runs -124, 0, 124, 248, 372 and exits on `cmp $0x1f0` (496, line
     * 2698's `cmp`/`je` test, offsets 2970..2982) with a step of 0x7c (124); the second half's
     * tail jumps back to the first half's entry, so the two physically duplicated halves are one
     * loop body cross-jumped by -O2. Inside it, the `idiv $0x54` REMAINDER of map.offset is
     * multiplied by the double 1.476 at 0x4d6d18 and truncated, giving the scroll offset added to
     * the base (line 2699's own bytes: the divisor setup at 2824..2836 plus the idiv/float-mult/
     * data[100] lookup at 2982..3063 -- the mirror computation for the other half is credited by
     * the line table to the inlined draw.inl:280 body instead, an artifact of the cross-jump, not
     * a separate main.c statement). The sprite is data[100].dat and the two constant arguments in
     * the third slot (x) are 0x235 (565) and 0xffffffc7 (-57), so the strips are vertical, at the
     * right and left screen edges, sharing one y per iteration. Resolved by side-by-side reading
     * of offsets 2786..3110 (aligned_view sbs) against the ORIGINAL's own GFX_VTABLE layout, read
     * from its DWARF rather than guessed: 0x44 draw_sprite, 0x48 draw_256_sprite, 0x4c
     * draw_sprite_v_flip, 0x50 draw_sprite_h_flip, 0x54 draw_sprite_vh_flip. So the call reached
     * with x=0x235 uses 0x44 and 0x48, which is the depth check inside the plain draw_sprite
     * inline (draw.inl:238), and the call reached with x=0xffffffc7 uses 0x50, which is
     * draw_sprite_h_flip: the left strip is the right strip mirrored horizontally. An earlier
     * reading of this block was one vtable slot out and used the vertical flips. */
    for (cy = -124; cy != 496; cy += 124) {                                    /* 2698 */
        cx = cy + (int)((map.offset % 84) * 1.476);                             /* 2699 */
        draw_sprite(bmp, data[100].dat, 565, cx);                                /* 2699 */
        draw_sprite_h_flip(bmp, data[100].dat, -57, cx);                        /* 2699 */
    }

    draw_sprite(bmp, data[16].dat, 22, 100);          /* 2705 */
    /* Machine evidence (offsets 3640..3806 vs 5823..5968, --report comparison.json):
     * the in_combo branch's own tail (after its own blit+draw_sprite) jumps directly
     * into offset 3744, which is INSIDE the reward_time branch's argument setup for
     * textprintf_centre_ex, skipping reward_time's own test (3663) and its own
     * draw_sprite (3677) entirely. A live combo can therefore never also run the
     * reward_time body in the same frame, which is only reachable if the two `if`s
     * are one if/else-if chain, not two independent statements. */
    if (ply[player_id]->in_combo) {                    /* 2706 */
        blit(data[15].dat, bmp, 0, 100 - ply[player_id]->in_combo, 33,   /* 2707 */
             219 - ply[player_id]->in_combo, 16, ply[player_id]->in_combo);
        draw_sprite(bmp, data[14].dat, -8, 210);         /* 2708 */
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d",   /* 2709 */
                              ply[player_id]->acc_level);
    } else if (reward_time) {                            /* 2711 */
        draw_sprite(bmp, data[14].dat, -8, 210);           /* 2712 */
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d",     /* 2713 */
                              ply[player_id]->latest_combo);
    }

    if (hurry_y < 251 || hurry_y > 479) {                 /* 2717 */
        x = 6;
        y = 10;
        cx = 0;
        cy = 0;
    } else {
        x = logic_count % 3 + 5;
        y = (logic_count + 1) % 3 + 8;
        cx = logic_count % 3 - 1;
        cy = (logic_count + 1) % 3 - 1;
    }
    draw_sprite(bmp, data[12].dat, x, y);                  /* 2718 */

    if (hurry_y >= 201 && hurry_y <= 479) {                /* 2719 */
        cx = (logic_count + 2) % 3 - 1;
        cy = (logic_count + 3) % 3 - 1;
    }
    rotate_sprite(bmp, data[13].dat, cx + 34, cy + 28,      /* 2720 */
                  clock_angle ? ftofix((clock_angle % 1500) * 0.1706666) : 0);

    if (reward_time) {                                      /* 2721 */
        draw_reward(swap_screen);                            /* 2722 */
    }

    textprintf_ex(bmp, data[52].dat, 8, 440, -1, -1, "score: %d",   /* 2739 */
                  ply[player_id]->level * 10 + ply[player_id]->score);

    if (!recording) {                                        /* 2742 */
        char myBuf[256];

        if (frame_count & 8) {                                /* 2746 */
            strcpy(myBuf, "REPLAY");                            /* 2747 */
            ls = 630 - text_length(data[53].dat, myBuf);         /* 2748 */
            textprintf_ex(bmp, data[53].dat, ls + 1, 5, makecol(0, 0, 0), -1, "REPLAY");        /* 2749 */
            textprintf_ex(bmp, data[53].dat, ls, 4, makecol(255, 255, 255), -1, "REPLAY");       /* 2750 */
        }

        if (is_playing_custom_game) {                          /* 2754 */
            sprintf(myBuf, "%s Floors", floor_size_selection.caption[demo->floor_size]);  /* 2755 */
            cx = 630 - text_length(data[53].dat, myBuf);          /* 2756 */
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 16, makecol(0, 0, 0), -1);          /* 2757 */
            textout_ex(bmp, data[53].dat, myBuf, cx, 15, makecol(255, 255, 255), -1);         /* 2758 */

            sprintf(myBuf, "%s Speed", scroll_speed_selection.caption[demo->start_speed]);  /* 2760 */
            cx = 630 - text_length(data[53].dat, myBuf);           /* 2761 */
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 26, makecol(0, 0, 0), -1);           /* 2762 */
            textout_ex(bmp, data[53].dat, myBuf, cx, 25, makecol(255, 255, 255), -1);          /* 2763 */

            strcpy(myBuf, gravity_selection.caption[demo->gravity]);   /* 2765 */
            cx = 630 - text_length(data[53].dat, myBuf);              /* 2766 */
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 36, makecol(0, 0, 0), -1);            /* 2767 */
            textout_ex(bmp, data[53].dat, myBuf, cx, 35, makecol(255, 255, 255), -1);           /* 2768 */
        }
        /* DWARF lexical block 124048 (myBuf/myPos/vcr/len/scrollerText) has PC ranges
         * covering both this REPLAY/custom-game text (main.c:2742..2768) and D4's
         * scroller/controller-icon code (main.c up to ~2803, ending right before the
         * unconditional debug F2 overlay): they are one shared `if (!recording) { }`
         * block, not two separate blocks. This region intentionally leaves that
         * block open; D4 declares myPos/len/vcr/scrollerText as siblings of myBuf
         * and closes the brace itself. */

    /* Continues D3's `if (!recording) { ... }` block (DWARF lexical block 124048
     * covers myBuf through here, up to and including the rect() below, as one
     * shared block, not a nested one); the brace is closed just before the
     * unconditional debug F2 overlay. */
        int myPos;
        int len;
        BITMAP *vcr;
        char scrollerText[70];

        vcr = data[127].dat;                            /* 2773 */
        myPos = rec_pos;                                /* 2774 */
        len = demo->size;                               /* 2774 */
        ox = 0x27b - vcr->w;                             /* 2776 */
        y = 0x1db - vcr->h; /* 2777: DWARF: `y`'s slot (reg edi) is live 4279..4341, exactly this
                              * assignment through the dead-check below; the D1-owned `x`
                              * local is NOT live over the matching esi computation here
                              * (its ranges stop at 2234), so that esi temp stays `ox`. */
        draw_sprite(bmp, vcr, ox, y); /* offsets 4282..4315, draw.inl:238 -- the only main.c:2778
                                        * candidate in this range; args are the ox/y just set. */
        if (!ply[player_id]->dead) {                     /* 2779 */
            /* `y` stays live (edi) through 7594..7837 for these three: is_left/is_fire/is_right
             * each build `y + 5` directly in a register (ecx) while the x-argument is spilled
             * through the `cx` stack slot (-0x178) only as a call-argument temporary. */
            if (is_left(&ctrl))                          /* 2780 */
                draw_sprite(bmp, data[128].dat, ox + 0x61, y + 5);
            if (is_fire(&ctrl))                           /* 2781 */
                draw_sprite(bmp, data[130].dat, ox + 0x6b, y + 5);
            if (is_right(&ctrl))                          /* 2782 */
                draw_sprite(bmp, data[129].dat, ox + 0x75, y + 5);
        }
        /* Stack-slot evidence (DW_OP_breg5): cx = -0x178(ebp), cy = -0x174(ebp).
         * offset 4338 "add $0xa,%edi; mov %edi,-0x178(%ebp)" stores y+0xa into cx (edi holds y).
         * offset 4347 "lea 0xa(%esi),%edi; mov %edi,-0x174(%ebp)" stores ox+0xa into cy (esi holds ox),
         * and that same edi is the set_clip_rect x1 argument, i.e. cy, not cx. */
        cx = y + 0xa;                                     /* 2784 */
        cy = ox + 0xa;                                     /* 2785 */
        set_clip_rect(bmp, cy, 0, 0x26f, 0x1df);            /* 2785 */
        /* Machine evidence (offsets 4392..4451 fallthrough vs 6010..6026 jump-in):
         * there is exactly one physical call to sprintf; the empty-comment path only
         * sets up arg3 = "" (esp+0x10) before falling into the shared arg2/arg1/fmt/
         * dest setup and the single call, and the comment path only sets up
         * arg3 = demo->comment before jumping into that same shared setup -- so this
         * is sprintf(scrollerText, "%s%s%s", demo->name, " - ", <arg3>) with a single
         * call site, arg3 selected by the test at 2787, not two separate sprintf
         * statements. */
        sprintf(scrollerText, "%s%s%s", demo->name, " - ",         /* 2787 */
                !demo->comment[0] ? "" : demo->comment);
        /* offset 4479 "mov -0x178(%ebp),%edi; add $0x4,%edi" reloads cx (not cy) for the
         * y-coordinate of every textout_ex below; the x-coordinate keeps using ox. */
        textout_ex(bmp, data[53].dat, demo->name, ox + 0xc - scroll_count / 2,     /* 2788 */
            cx + 4, makecol(150, 150, 160), -1);
        textout_ex(bmp, data[53].dat, demo->name, ox + 0xd - scroll_count / 2,     /* 2789 */
            cx + 4, makecol(200, 200, 210), -1);
        if (demo->comment[0]) {                                                    /* 2790 */
            textout_ex(bmp, data[53].dat, " - ",                                    /* 2791 */
                ox + 0xc - scroll_count / 2 + text_length(data[53].dat, demo->name),
                cx + 4, makecol(200, 200, 210), -1);
            textout_ex(bmp, data[53].dat, demo->comment,                            /* 2792 */
                ox - scroll_count / 2 + 0x1e + text_length(data[53].dat, demo->name),
                cx + 4, makecol(200, 200, 210), -1);
        }
        set_clip_rect(bmp, 0, 0, 0x27f, 0x1df);              /* 2794 */
        if (demo->comment[0]) {                               /* 2796 */
            if (scroll_delay > 0) {                            /* 2797 */
                scroll_delay--;                                 /* 2798 */
            }
            else {
                scroll_count++;                                 /* 2801 */
                if (scroll_count / 2 > text_length(data[53].dat, scrollerText))  /* 2802 */
                    scroll_count = -250;                          /* 2803 */
            }
        }
        /* Original offsets 4808 and 4863 add 0x13 and 0x14 to cx at
         * -0x178(ebp); offsets 4876 and 4853 use cy at -0x174(ebp) for x.
         * The call through GFX_VTABLE +0xbc is rect (draw.inl:112), not rectfill
         * (+0x3c). The old candidate also shifted both y coordinates by 10. */
        rect(bmp, cy, cx + 0x14,                                          /* 2808 */
            cy + (myPos * 117 / len > 0x74 ? 0x74 : myPos * 117 / len),
            cx + 0x13, makecol(50, 200, 50));
    }

    if (debug && key[KEY_F2]) {                                          /* 2812 */
            textprintf_ex(bmp, font, 0, 0, 15, -1, "FPS:%6d / %d", fps, lps);  /* 2813 */
            textprintf_ex(bmp, font, 0, 0xa, 15, -1, "REC:%6d / %d", rec_pos,   /* 2814 */
                demo->size);
            textprintf_ex(bmp, font, 0, 0x14, 15, -1, "    %6d  (%d) ",          /* 2815 */
                demo->data[rec_pos].key_flags, demo->data[rec_pos].cycle_count);
            textprintf_ex(bmp, font, 0xc8, 0, 15, -1, "POS: %d, %d",              /* 2816 */
                (int)ply[player_id]->x, (int)ply[player_id]->y);
            textprintf_ex(bmp, font, 0xc8, 0xa, 15, -1, " dx: %1.2f",              /* 2817 */
                ply[player_id]->sx);
            textprintf_ex(bmp, font, 0xc8, 0x14, 15, -1, "rjp: %d",                 /* 2818 */
                options.jump_hold);
            textprintf_ex(bmp, font, 0x190, 0, 15, -1, "any: %6d %6d %6d",           /* 2819 */
                any11, any12, any13);
            textprintf_ex(bmp, font, 0x190, 0xa, 15, -1, "any: %6d %6d %6d",          /* 2820 */
                any21, any22, any23);
        }
    /* line 2822 tail: fragments at offsets 2246 ("mov $0x2,%edi") and 2556
     * ("mov $0x6,%esi") are also attributed to this line by the line table,
     * but they sit far outside this region's byte range and duplicate
     * register-constant setup that reads as spillover from an earlier
     * region's block layout; no call is associated with them, so nothing
     * is written here beyond the implicit function epilogue, which the
     * closing brace below inherits this annotation for. */
    /* 2822 */
}