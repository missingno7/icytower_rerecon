{
    int playing;
    int old_map_pos;
    int level;
    int diff;
    int i;
    int quit;
    int scroll_acc;
    int scroll;
    int max_scroll;
    int speeds[9] = { 1500, 3000, 4500, 6000, 7500, 9000, 10500, 1800000, 9000000 };
    int next_speed;
    int next_aight;
    int allow_smpl;
    int game_over;
    int falling;
    int shake;
    int flash;
    int step_count;
    int next_floor;
    int play_again;
    int tot_scroll;
    int lastX;
    int lastY;
    int midX;
    int midY;
    int numComboJumps;
    int totComboFloors;
    int startTime;
    int endTime;
    int lastJumpLength;
    int oldUnlockedFloors;
    int current_rank_id;
    Tcontrol rec_ctrl;
    int time_cheat_count;
    clock_t clockTimeStart;
    clock_t clockTimeEnd;
    double clockElapsed;
    double totClockTimes;
    int qpc_start;
    int qpc_end;
    double qpc_elapsed;
    double totQPCTimes;
    int timeTimeStart;
    int timeTimeEnd;
    int timeElapsed;
    double totTimeTimes;
    int musicCounter;
    int lastMusicPos;
    float accMusics;
    int totMusics;
    LARGE_INTEGER li;
    int qpc_freq;
    /* The -0x928 stack slot holds falling in the game loop; original offset 11240
     * reloads it into esi for the results animation, where DWARF names it falling. */

    rec_ctrl = ctrl;                                   /* line 3439 */
    if (!itrcheck) {                                   /* line 3441 (else-branch placed inline by -O2) */
        oldUnlockedFloors = profile->best_floor / 100; /* line 3442 */
        current_rank_id = get_rank_id(profile);        /* line 3443; get_rank_id has no visible prototype
                                                          * in main.c (declared in profile.c taking
                                                          * Tprofile_rank *; that type isn't visible here,
                                                          * so the cast used by other callers is omitted) */
    } else {
        current_rank_id = 0;
        oldUnlockedFloors = 0;
    }
    if (recording) {                                   /* line 3473 */
        demo->tc_posts = 0;                             /* line 3474 */
        for (i = 0; i < 100; i++) {                     /* line 3475 */
            demo->tc_c_data[i] = 0.0f;                  /* line 3476 */
            demo->tc_q_data[i] = 0.0f;                  /* line 3477 */
            demo->tc_t_data[i] = 0.0f;                  /* line 3478 */
            demo->tc_s_data[i] = 0.0f;                  /* line 3479 */
            demo->tc_f_data[i] = 0.0f;                  /* line 3480 */
        }
    }
    log2file(" setting up play data");                 /* line 3485 */
    fall_count = 0;                                     /* line 3486 */
    clock_angle = 0;                                    /* line 3487 */
    map.offset = 0;                                     /* line 3488 */
    fast_forward = 0;                                   /* line 3490 */
    fast_fast_forward = 0;                               /* line 3491 */
    update_frame();                                      /* line 3493 */
    if (!itrcheck) {                                     /* line 3494 */
        draw_frame(swap_screen);                         /* line 3495 */
        fadeIn(swap_screen, 16);                         /* line 3497 */
        play_sound(custom.yo, 0, 0);                     /* line 3498 */
        startGameMusic();                                /* line 3499 */
    }
    if (!itrcheck && bg_beat) {                          /* line 3502-3503 (compiled as its own re-test
                                                            * of itrcheck, redundant with the block above) */
        checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1); /* line 3504 */
    }
    cycle_count = 0;                                     /* line 3510 */
    log2file(" play started");                           /* line 3512 */
    startTime = time(NULL);                              /* line 3513 */
    QueryPerformanceCounter(&li);                        /* line 3519 */
    qpc_start = li.LowPart;                              /* line 3520 */
    totMusics = 0;
    accMusics = 0.0f;
    lastMusicPos = 0;
    musicCounter = 0;
    time_cheat_count = 0;
    lastJumpLength = 0;
    endTime = 0;
    totComboFloors = 0;
    numComboJumps = 0;
    next_floor = -1;
    step_count = 0;
    shake = 0;
    falling = 0;
    game_over = 0;
    allow_smpl = 1;
    next_aight = 50;
    next_speed = 0;
    scroll = -1;
    quit = 0;
    QueryPerformanceFrequency(&li);                      /* line 3522; result unused here (its DWARF
                                                            * live range only starts at the second QPF
                                                            * call inside the loop, line 3610) */
    clockTimeStart = clock();                            /* line 3528 */
    timeTimeStart = time(NULL);                          /* line 3530 */
    playing = TRUE;                                       /* line 3533: the original enters the loop
                                                            * without testing playing (offset 351 jumps
                                                            * straight to the closeButtonClicked test),
                                                            * so it is known non-zero here; the value is
                                                            * constant-folded away and emits no code */

    while (playing) {                          /* 3534 */
        if (closeButtonClicked)                                 /* 3536 */
            return 0;
        {   /* 3536 (3534 tests playing; both tests and both loop
                                                 * entry/back-edge copies compile to this one physical
                                                 * line, so its bytes are credited to the larger 3536) */
        cycle_count = 0;                                 /* line 3540 */
        logic_count++;                                   /* line 3542 */
        step_count++;                                    /* line 3543 */
        fall_count++;                                     /* line 3544 */
        time_cheat_count++;                               /* line 3545 */
        musicCounter++;                                   /* line 3546 */
        if (!itrcheck) {                                  /* line 3549 */
            if (lastFocus != hasFocus) {                  /* 3550 */
                if (hasFocus) {                            /* 3551 */
                    /* line 3552-3559: gaining focus, restart the background track */
                    if (bg_beat) {                          /* 3552 */
                        checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1); /* 3553 */
                    }
                    startGameMusic();                       /* 3559 */
                    totMusics = 0;                          /* 3559 */
                    accMusics = 0.0f;                       /* 3559 */
                    musicCounter = 0;                       /* 3559 */
                } else {
                    /* line 3562-3567: losing focus, stop the background track */
                    if (checkMusicVoiceID >= 0) {           /* 3562 */
                        voice_stop(checkMusicVoiceID);      /* 3563 */
                    }
                    checkMusicVoiceID = -1;                 /* 3565 */
                    stopGameMusic();                        /* 3567 */
                }
                lastFocus = hasFocus;                     /* line 3569 */
            }
        }
        if (!itrcheck) {                                  /* line 3574 (compiled as its own re-test of
                                                             * itrcheck, redundant with the block above) */
            if (checkMusicVoiceID >= 0) {                 /* line 3574 */
                int vgp;   /* DWARF block 132257 [760..886]: vgp (int), a (float), b (float) */
                float a, b;

                vgp = voice_get_position(checkMusicVoiceID); /* line 3575 */
                if (vgp < lastMusicPos) {                 /* line 3576 */
                    musicCounter = 0;
                }
                /* lines 3580-3585: running average of a 44000/vgp "speed" ratio into accMusics,
                 * gated on that ratio being > 0.01 (x87 fucompp/fnstsw/test $0x45 idiom); see report
                 * for the derivation of the comparison direction. The two named DWARF temps a/b hold
                 * the ratio and the scaled increment across lines 3580-3584. */
                a = vgp / 44000.0;                        /* 3580 */
                if (a > 0.01) {                           /* line 3583: original fldl */
                    b = musicCounter / 50.0;              /* line 3584: the original computes the intermediate
                                                            * musicCounter/50.0 first (fidivrl -0x938, a REVERSE
                                                            * divide of the int by the ST0-resident 50.0) and then
                                                            * divides 'a' by that -- algebraically a*50.0f/musicCounter,
                                                            * but this operand order is what reproduces fidivrl */
                    accMusics += b / a;
                    totMusics++;                          /* line 3585 */
                }
                lastMusicPos = vgp;                       /* line 3585 (tail) */
            }
        }
        if (recording && map.offset > 100 && !ply[player_id]->dead) { /* line 3595 */
            if (time_cheat_count == 1000) {                          /* 3597 */
                /* lines 3604-3661: periodic clock()/QueryPerformanceCounter()/time() cross-check,
                 * recorded into the demo replay's time-cheat-detection arrays. DWARF lexical block
                 * 134356 (3604..3661) declares three doubles our source previously never named:
                 * clockSpeed, qpcSpeed, timeSpeed -- the per-method rate used to gate/derive each
                 * tot*Times value below. The exact x87 formulas are a best-effort reconstruction
                 * from the instruction stream (see report); the calls and field targets are
                 * evidenced directly. */
                double clockSpeed;  /* DWARF block 134356 [3604..3661] */
                double qpcSpeed;
                double timeSpeed;

                clockTimeEnd = clock();                             /* line 3604 */
                clockElapsed = clockTimeEnd - clockTimeStart;       /* line 3605 */
                clockSpeed = (50.0 * clockElapsed) / 1000.0;        /* line 3605 (tail); confirmed against
                                                                      * the real toolchain: this direction
                                                                      * (not 1000.0/(50*clockElapsed)) is what
                                                                      * actually emits fdivr %st,%st(1) -- the
                                                                      * reciprocal form silently emits the same
                                                                      * byte count via plain fdiv, which is why
                                                                      * the swap wasn't caught by bytes alone. */
                if (clockSpeed > 0.0) {                             /* 3606 */
                    totClockTimes = (1000.0 * clockSpeed) / clockSpeed / 20.0; /* 3606: reuses the same
                                                                      * 1000.0 already resident from 3605
                                                                      * (GCC folds this to a single flds,
                                                                      * confirmed via the real toolchain) --
                                                                      * this is 1.0/clockSpeed/20.0, a genuine
                                                                      * rate (1/clockElapsed), not a constant. */
                } else {
                    totClockTimes = -0.05;                          /* 3606 (== -1.0/20.0, guard-fail sentinel) */
                }
                QueryPerformanceFrequency(&li);                     /* line 3610 */
                qpc_freq = li.LowPart;                               /* line 3611 */
                QueryPerformanceCounter(&li);                       /* line 3612 */
                qpc_end = li.LowPart;                               /* line 3612 tail */
                qpc_elapsed = qpc_end - qpc_start;                  /* line 3615 */
                qpcSpeed = (50.0 * qpc_elapsed / qpc_freq) / 20.0;  /* line 3615 (tail) */
                totQPCTimes = qpcSpeed;                             /* line 3615 (tail) */
                timeTimeEnd = time(NULL);                           /* line 3623 */
                timeElapsed = timeTimeEnd - timeTimeStart;          /* line 3638 */
                timeSpeed = (50.0 * timeElapsed) / 20.0;            /* line 3638 (tail) */
                totTimeTimes = timeSpeed;                           /* line 3638 (tail) */
                demo->tc_c_data[demo->tc_posts] = 0.0 + totClockTimes;    /* line 3636 */
                demo->tc_q_data[demo->tc_posts] = 0.0 + totQPCTimes;      /* line 3637 */
                demo->tc_t_data[demo->tc_posts] = 0.0 + totTimeTimes;     /* line 3638 (tail) */
                demo->tc_f_data[demo->tc_posts] = ply[player_id]->level; /* line 3639 */
                if (totMusics != 0) {                               /* line 3640 */
                    demo->tc_s_data[demo->tc_posts] = 50.0 * accMusics / totMusics; /* line 3641 */
                }
                demo->tc_posts = demo->tc_posts < 98 ? demo->tc_posts + 1 : 99;
                clockTimeStart = clock();                           /* line 3654 */
                QueryPerformanceCounter(&li);                       /* line 3656 */
                qpc_start = li.LowPart;                             /* line 3657 */
                timeTimeStart = time(NULL);                         /* line 3661 */
                totMusics = 0;
                accMusics = 0.0f;
                time_cheat_count = 0;
            }
        }
        if (debug) {                                      /* line 3681 */
            /* lines 3682-3691: ten combo-length reward tiers, keyed to the number-row keys */
            if (key[KEY_1]) { if (allow_smpl) start_reward(5); }      /* 3682 */
            if (key[KEY_2]) { if (allow_smpl) start_reward(7); }      /* 3683 */
            if (key[KEY_3]) { if (allow_smpl) start_reward(15); }     /* 3684 */
            if (key[KEY_4]) { if (allow_smpl) start_reward(25); }     /* 3685 */
            if (key[KEY_5]) { if (allow_smpl) start_reward(35); }     /* 3686 */
            if (key[KEY_6]) { if (allow_smpl) start_reward(50); }     /* 3687 */
            if (key[KEY_7]) { if (allow_smpl) start_reward(70); }     /* 3688 */
            if (key[KEY_8]) { if (allow_smpl) start_reward(100); }    /* 3689 */
            if (key[KEY_9]) { if (allow_smpl) start_reward(140); }    /* 3690 */
            if (key[KEY_0]) { if (allow_smpl) start_reward(200); }    /* 3691 */
            /* line 3692 */
            allow_smpl = !(key[KEY_1] || key[KEY_2] || key[KEY_3] ||
                           key[KEY_4] || key[KEY_5] || key[KEY_6] ||
                           key[KEY_7] || key[KEY_8] || key[KEY_9] ||
                           key[KEY_0]);
        }
        midX = (int)ply[player_id]->x;                                  /* 3698 */
        midY = (int)ply[player_id]->y;                                  /* 3699 */

        handle_player_input(&ctrl);                                     /* 3702 */
        update_player(ply[player_id]);                                  /* 3703 */
        if (!itrcheck) {                                                /* 3706 */
            if (ply[player_id]->rotate && ply[player_id]->in_combo && !options.flash)  /* 3707 */
                create_particle(stars, (int)ply[player_id]->x, (int)ply[player_id]->y - 16);  /* 3708 */
            for (i = 0; i < 512; i++)                                   /* 3711 */
                if (stars[i].intensity)
                    update_particle(&stars[i]);
        }

        lastY = midY;
        old_map_pos = map.offset;                                       /* 3717 */
        scroll_acc = 0;
        if (ply[player_id]->y < 160.0) {                                /* 3719 */
            scroll_acc = 1;
            if (ply[player_id]->y < 140.0) scroll_acc++;           /* 3721 */
            if (ply[player_id]->y < 120.0) scroll_acc++;                /* 3722 */
            if (ply[player_id]->y < 100.0) scroll_acc++;                /* 3723 */
            if (ply[player_id]->y < 80.0) scroll_acc++;                 /* 3724 */
            if (ply[player_id]->y < 60.0) scroll_acc++;                 /* 3725 */
            if (ply[player_id]->y < 40.0) scroll_acc += 2;              /* 3726 */
            if (ply[player_id]->y < 20.0) scroll_acc += 2;              /* 3727 */
            if (ply[player_id]->y < 0.0) scroll_acc += 3;               /* 3728 */
            map.offset = old_map_pos + scroll_acc;                      /* 3729 */
            ply[player_id]->y += scroll_acc;                            /* 3731 */
            lastY = midY + scroll_acc;                                  /* 3732 */
        }
        tot_scroll = scroll_acc;
        if (!ply[player_id]->dead)                                      /* 3736 */
            clock_angle++;
        if (map.offset > 100 && !ply[player_id]->dead) {                /* 3738 */
            if (scroll == -1)                                           /* 3739 */
                scroll = start_speeds[demo->start_speed];               /* 3740 */
            if (!scroll) {
                if (step_count & 1) {
                map.offset++;                                           /* 3744 */
                tot_scroll++;                                           /* 3745 */
                ply[player_id]->y += 1.0;                               /* 3746 */
                lastY++;                                                /* 3747 */
                }
            }
            else {
                map.offset += scroll;                                   /* 3751 */
                tot_scroll += scroll;                                   /* 3752 */
                ply[player_id]->y += scroll;                            /* 3753 */
                lastY += scroll;                                        /* 3754 */
            }
        }
        else if (!ply[player_id]->dead) {                               /* 3757 */
            clock_angle = 0;                                            /* 3758 */
            fall_count = 0;
        }
        any13 = tot_scroll;                                             /* 3763 */
        if (hurry_y > -100 && hurry_y < 480)                            /* 3765 */
            hurry_y -= 2;
        if (demo->speed_increase)                                       /* 3766 */
            if (!ply[player_id]->dead && speeds[next_speed] < fall_count && scroll < 5) {  /* 3767 */
                ply[player_id]->ccc[next_speed] = ply[player_id]->level;  /* 3768 */
                next_speed++;                                           /* 3770 */
                scroll++;                                               /* 3771 */
                hurry_y = 479;                                          /* 3772 */
                play_sound(speaker[0], 0, 0);                           /* 3773 */
                play_sound(sounds[4], 0, 0);                            /* 3774 */
            }
        if (scroll == 5) {                                              /* 3778 */
            fall_count -= 45;                                           /* 3779 */
            if (!ply[player_id]->dead)                                  /* 3780 */
                clock_angle -= 45;
        }
        if (old_map_pos % 16 > map.offset % 16                          /* 3783 */
            || tot_scroll > 15)                                         /* 3787 */
            add_floor(&map);                                            /* 3789 */

        switch (collision_type) {                                       /* 3814 */
        case 0:
            handle_player_collision_original(midX, lastY);              /* 3815 */
            break;
        case 1:
            handle_player_collision_old(midX, lastY);                   /* 3817 */
            break;
        case 2:
            handle_player_collision_vector(midX, lastY);                /* 3819 */
            break;
        case 3:
            handle_player_collision_vector_2(midX, lastY);              /* 3821 */
            break;
        case 4:
            handle_player_collision_combo(midX, lastY);                 /* 3823 */
            break;
        default:
            allegro_message("unknown collision type");                  /* 3826 */
            break;
        }

        if (ply[player_id]->rotate)                                     /* 3833 */
            ply[player_id]->angle += itofix(8);
        if (ply[player_id]->in_combo) {                                 /* 3837 */
            ply[player_id]->in_combo--;                                 /* 3838 */
            if (!ply[player_id]->in_combo && ply[player_id]->acc_jumps > 1) {  /* 3839 */
                int rewResult;
                Tgd_combo c;

                ply[player_id]->score += ply[player_id]->acc_level * ply[player_id]->acc_level;  /* 3841 */
                rewResult = start_reward(ply[player_id]->acc_level);    /* 3842 */
                if (recording && !is_playing_custom_game)               /* 3843 */
                    profile->rewards[rewResult]++;
                totComboFloors += ply[player_id]->acc_level;            /* 3844 */
                numComboJumps++;                                        /* 3845 */
                c.length = ply[player_id]->acc_level;                   /* 3848 */
                c.start = gdComboStart;                                 /* 3849 */
                c.end = c.start + c.length;                             /* 3850 */
                add_combo(gameData, &c);                                /* 3851 */
                ply[player_id]->latest_combo = ply[player_id]->acc_level;  /* 3853 */
                if (ply[player_id]->acc_level > ply[player_id]->best_combo)  /* 3854 */
                    ply[player_id]->best_combo = ply[player_id]->acc_level;  /* 3855 */
            }
        }

        if (!ply[player_id]->status) {                                  /* 3862 */
            level = (get_level(&map, (int)ply[player_id]->y) - 1) / 5;  /* 3864 */
            diff = level - ply[player_id]->level;                       /* 3869 */
            if (diff != 0) {                                            /* 3870 */
                if (diff != gdLastJumpDiff) {
                    jumpSequence.dist = gdLastJumpDiff;
                    add_jump_sequence(gameData, &jumpSequence);
                    jumpSequence.num = 1;
                    jumpSequence.start = level - diff;
                } else jumpSequence.num++;

                gdLastJumpDiff = diff;                                  /* 3885 */
            }
            if (level >= ply[player_id]->level) {                       /* 3891 */
                diff = level - ply[player_id]->level;                   /* 3893 */
                if (diff != lastJumpLength && diff != 0) {              /* 3896 */
                    for (i = 0; i < 5; i++) {                           /* 3897 */
                        if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])  /* 3900 */
                            ply[player_id]->jcTop[i] = ply[player_id]->jc[i];  /* 3901 */
                        ply[player_id]->jc[i] = 0;                      /* 3904 */
                    }
                    lastJumpLength = 0;
                }
                if (diff > 0) {                                         /* 3911 */
                    if (diff <= 5)                                      /* 3912 */
                        ply[player_id]->jc[diff - 1]++;                 /* 3913 */
                    if (diff != 1) {                                    /* 3919 */
                        if (ply[player_id]->in_combo) {                 /* 3920 */
                            ply[player_id]->acc_level += diff;          /* 3921 */
                            ply[player_id]->acc_jumps++;                /* 3922 */
                            ply[player_id]->in_combo = 100;             /* 3923 */
                        }
                        else {
                            ply[player_id]->acc_level = diff;           /* 3926 */
                            ply[player_id]->acc_jumps = 1;              /* 3927 */
                            ply[player_id]->in_combo = 100;             /* 3928 */
                        }
                    }
                    lastJumpLength = diff;
                }
                if (diff == 1 && ply[player_id]->in_combo)              /* 3932 */
                    ply[player_id]->in_combo = 1;                       /* 3933 */
                if (!ply[player_id]->in_combo)                          /* 3936 */
                    gdComboStart = level;                               /* 3937 */
            }
            else {
                if (ply[player_id]->in_combo)                           /* 3943 */
                    ply[player_id]->in_combo = 1;
                for (i = 0; i < 5; i++) {                               /* 3945 */
                    if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])  /* 3948 */
                        ply[player_id]->jcTop[i] = ply[player_id]->jc[i];  /* 3949 */
                    ply[player_id]->jc[i] = 0;                          /* 3952 */
                }
                lastJumpLength = 0;
            }
            ply[player_id]->level = level;                              /* 3962 */
            if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level)  /* 3967 */
                ply[player_id]->no_combo_top_floor = gdComboStart;      /* 3969 */
        }

        if (ply[player_id]->y > 540.0 && !ply[player_id]->dead) {       /* 3976 */
            if (itrcheck)                                               /* 3977 */
                playing = FALSE;
            if (ply[player_id]->in_combo && ply[player_id]->acc_jumps > 1)  /* 3978 */
                ply[player_id]->biggest_lost_combo = ply[player_id]->acc_level;  /* 3979 */
            ply[player_id]->in_combo = 0;                               /* 3981 */
            ply[player_id]->dead = 1;                                   /* 3982 */
            play_sound(custom.falling, 0, 1);                           /* 3983 */
            endTime = time(0);                                          /* 3985 */
            for (i = 0; i < 5; i++) {                                   /* 3988 */
                if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])   /* 3991 */
                    ply[player_id]->jcTop[i] = ply[player_id]->jc[i];   /* 3992 */
                ply[player_id]->jc[i] = 0;                              /* 3995 */
            }
            jumpSequence.dist = gdLastJumpDiff;                         /* 3999 */
            add_jump_sequence(gameData, &jumpSequence);                 /* 4000 */
            if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level)  /* 4003 */
                ply[player_id]->no_combo_top_floor = ply[player_id]->level;  /* 4004 */
            lastJumpLength = 0;
            falling = 1;
        }
        if (ply[player_id]->y > 900.0 && !game_over) {                  /* 4010 */
            play_sound(speaker[1], 0, 0);                               /* 4012 */
            game_over = 2;
        }
        if (falling)                                                    /* 4015 */
            falling++;
        if (falling > ply[player_id]->level * 5 || falling > 250) {     /* 4016 */
            play_sound(sounds[6], 0, 1);                                /* 4017 */
            if (custom.falling)                                         /* 4018 */
                stop_sample(custom.falling);                            /* 4019 */
            ply[player_id]->shake = 24;                                 /* 4022 */
            falling = 0;
        }
        if (ply[player_id]->level >= next_aight) {                      /* 4027 */
            play_sound(sounds[2], 0, 0);                                /* 4028 */
            if (!options.flash)                                         /* 4029 */
                for (i = 0; i < next_aight / 2; i++) {
                    int p;
                    p = create_particle(stars, (new_rand() % 600) + 20, 480);  /* 4030 */
                    stars[p].sy = -(((new_rand() % 200) << 16) / 10);   /* 4031 */
                }
            if (next_aight > 999)                                       /* 4033 */
                next_aight += 500;                                      /* 4034 */
            else
                next_aight += 50;                                       /* 4037 */
        }
        if (!ply[player_id]->edge)                                      /* 4042 */
            ply[player_id]->edge_drawn = 0;
        if (ply[player_id]->edge_drawn) {                               /* 4043 */
            if (ply[player_id]->edge_drawn == 11 && !ply[player_id]->status)  /* 4044 */
                play_sound(custom.edge, 1, 1);
            if (ply[player_id]->edge_drawn == 50)                       /* 4045 */
                ply[player_id]->edge_drawn = 0;                         /* 4046 */
        }
        if (debug) {                                                    /* 4049 */
            if (ply[player_id]->dead <= 99)                             /* 4050 */
                playing = FALSE;
        }
        else if (recording && ply[player_id]->dead > 100)               /* 4056 */
            playing = FALSE;
        if (!itrcheck && key[KEY_F1]) {                                 /* 4062 */
            int pauseTime, addTime;
            pauseTime = time(NULL);                                     /* 4063 */
            take_screenshot(swap_screen);                               /* 4064 */
            while (key[KEY_F1]) ;                                       /* 4065 */
            addTime = time(NULL) - pauseTime;                           /* 4066 */
            if (addTime > 0)                                            /* 4067 */
                startTime += addTime;                                   /* 4068 */
            if (checkMusicVoiceID >= 0) {                               /* 4075 */
                musicCounter = voice_get_position(checkMusicVoiceID) * 50.0f / 44000.0f;  /* 4077 */
                totMusics = 0;
                accMusics = 0.0f;
            }
            clockTimeStart = clock();                                   /* 4083 */
            QueryPerformanceCounter(&li);                               /* 4085 */
            qpc_start = li.LowPart;                                     /* 4086 */
            timeTimeStart = time(NULL);                                 /* 4090 */
            time_cheat_count = 0;
        }
        if (ply[player_id]->shake) {                                    /* 4094 */
            ply[player_id]->shake--;                                    /* 4095 */
            shake = new_rand() % 8;                                     /* 4097 */
        }
        update_frame();                                                 /* 4100 */
        if (!quit && closeButtonClicked) {                              /* 4104 */
            quit = 1;
            playing = FALSE;
        }
        if (recording) {                                                      /* 4109 */
            if (key[KEY_ESC]) {                                               /* 4110 */
                if (ply[player_id]->dead) {                                   /* 4111 */
                    log2file("  player quit after dying");                    /* 4112 */
                    playing = 0;                                              /* 4112 */
                } else {
                    /* REGION W3a: ESC pause screen, lines 4117..4182 */
                    int pauseTime, fc, ca, addTime; /* block-scoped DWARF locals (block 132596), not in the 52-local skeleton */

                    pauseTime = time(NULL);                                   /* 4117 */
                    fc = fall_count;                                          /* 4118 */
                    ca = clock_angle;                                         /* 4119 */
                    log2file("  game paused with esc");                      /* 4120 */
                    for (i = 0; i < 640; i += 2) {                            /* 4122 */
                        vline(swap_screen, i, 0, 480, 0);                     /* draw.inl:46 */
                        hline(swap_screen, 0, i, 640, 0);                     /* draw.inl:54 */
                    }
                    textout_centre_ex(swap_screen, data[50].dat,               /* 4126: GCC attributes a call's
                                                                                  bytes to its opening line, so the
                                                                                  annotation moves here (was on the
                                                                                  closing line, which left the
                                                                                  line_budget tool crediting each
                                                                                  call's bytes to the call above it
                                                                                  and showing 4128 as 0 bytes even
                                                                                  though the call is present). */
                                       "DO YOU REALLY WANT TO EXIT?", 320, 160, -1, -1);
                    textout_centre_ex(swap_screen, data[52].dat,               /* 4127 */
                                       "Press any key to resume", 320, 210, -1, -1);
                    textout_centre_ex(swap_screen, data[52].dat,               /* 4128 */
                                       "Press ESC to exit", 320, 240, -1, -1);
                    blit_to_screen(swap_screen);                              /* 4129 */
                    play_sound(custom.wazup, 0, 1);                           /* 4130 */
                    /* Two-loop wait, re-derived from the assembly (source-view 4117..4182):
                     * outer loop's entry jmp lands on its is_any/is_pause/ESC compound test
                     * (offsets 1717.. no -- offsets 5357/5316 etc, see function_lines source-view),
                     * a plain bottom-tested `while (cond) body`; the inner "key still held" loop's
                     * entry jmp lands on keypressed() alone (offset 5490), meaning keypressed() is
                     * the sole loop condition and the is_any/is_pause/closeButtonClicked/key[ESC]
                     * checks are an if-break inside its body -- GCC then thread the break's three
                     * different truth cases into different entry points of the loop that follows
                     * (closeButtonClicked jumps straight past that loop's own redundant
                     * closeButtonClicked test; key[ESC] jumps into its is_pause call for the same
                     * reason), which is why the reconstruction only needs three plain loops. */
                    poll_control(&ctrl, 0);                                    /* 4132 */
                    while (is_any(&ctrl) || is_pause(&ctrl)                    /* 4133 */
                           || (!closeButtonClicked && key[KEY_ESC])) {         /* 4133 */
                        poll_control(&ctrl, 0);                                /* 4134 */
                        rest(2);                                               /* 4135 */
                    }
                    clear_keybuf();                                            /* 4137 */
                    while (!keypressed()) {                                    /* 4138 */
                        if (is_any(&ctrl) || is_pause(&ctrl) ||
                            closeButtonClicked || key[KEY_ESC])                /* 4138 */
                            break;                                            /* 4138 */
                        poll_control(&ctrl, 0);                                /* 4139 */
                        rest(2);                                               /* 4140 */
                    }
                    while (!closeButtonClicked && is_pause(&ctrl)) {           /* 4142 */
                        poll_control(&ctrl, 0);                                /* 4143 */
                        rest(2);                                               /* 4144 */
                    }
                    if (key[KEY_ESC]) {                                       /* 4146 */
                        log2file("  game quit from esc pause");               /* 4150 */
                        profile->games_quit++;                                /* 4151 */
                        endTime = time(NULL);                                 /* 4152 */
                        quit = 1;                                             /* 4152 */
                        playing = 0;                                          /* 4152 */
                    }
                    clear_keybuf();                                           /* 4154 */
                    fall_count = fc;                                          /* 4156 */
                    clock_angle = ca;                                         /* 4157 */
                    log2file("  game unpaused");                              /* 4158 */
                    addTime = time(NULL) - pauseTime;                         /* 4159 */
                    if (addTime > 0)                                          /* 4160 */
                        startTime += addTime;                                 /* 4161 */
                    if (checkMusicVoiceID >= 0) {                               /* 4168 */
                        musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); /* 4170 */
                        totMusics = 0;
                        accMusics = 0.0f;
                    }
                    clockTimeStart = clock();                                 /* 4175 */
                    QueryPerformanceCounter(&li);                             /* 4177 */
                    qpc_start = li.LowPart;                                   /* 4178 */
                    timeTimeStart = time(NULL);                               /* 4182 */
                    time_cheat_count = 0;
                }
            }
            if (is_pause(&ctrl) && ply[player_id]->dead == 0) {               /* 4186 */
                /* REGION W3b: pause-key screen, lines 4187..4245 (near-identical to W3a) */
                int pauseTime, fc, ca, addTime; /* block-scoped DWARF locals (block 132838), not in the 52-local skeleton */

                pauseTime = time(NULL);                                       /* 4187 */
                fc = fall_count;                                              /* 4188 */
                ca = clock_angle;                                             /* 4189 */
                log2file("  game paused with pause key");                     /* 4190 */
                for (i = 0; i < 640; i += 2) {                                /* 4192 */
                    vline(swap_screen, i, 0, 480, 0);                         /* draw.inl:46 */
                    hline(swap_screen, 0, i, 640, 0);                         /* draw.inl:54 */
                }
                textout_centre_ex(swap_screen, data[50].dat,                  /* 4196 (annotation kept on the
                                                                                  opening line; see the note above
                                                                                  the ESC screen's identical calls) */
                                   "Game Paused", 320, 160, -1, -1);
                textout_centre_ex(swap_screen, data[52].dat,                  /* 4197 */
                                   "Press any key to resume", 320, 210, -1, -1);
                blit_to_screen(swap_screen);                                  /* 4198 */
                play_sound(custom.wazup, 0, 1);                               /* 4199 */
                /* Same three-loop shape as the ESC screen (W3a above), but this screen's
                 * outer wait has no closeButtonClicked term (source-view 4186..4245 never
                 * loads it before is_any/is_pause/ESC), and its final loop tests
                 * is_pause() || key[KEY_ESC] directly (continues on either, offset
                 * 7018/7027 both jump back to the poll_control/rest body) rather than the
                 * negated-AND the ESC screen's closing loop uses. */
                poll_control(&ctrl, 0);                                       /* 4200 */
                while (is_any(&ctrl) || is_pause(&ctrl)) {                    /* 4201 */
                    poll_control(&ctrl, 0);                                   /* 4202 */
                    rest(2);                                                  /* 4203 */
                }
                clear_keybuf();                                               /* 4206 */
                while (!keypressed()) {                                       /* 4207 */
                    if (is_any(&ctrl) || is_pause(&ctrl) || key[KEY_ESC])      /* 4207 */
                        break;                                                /* 4207 */
                    poll_control(&ctrl, 0);                                   /* 4208 */
                    rest(2);                                                  /* 4209 */
                }
                poll_control(&ctrl, 0);                                       /* 4212 */
                while (is_pause(&ctrl) || key[KEY_ESC]) {                     /* 4213 */
                    poll_control(&ctrl, 0);                                   /* 4214 */
                    rest(2);                                                  /* 4215 */
                }
                fall_count = fc;                                              /* 4219 */
                clock_angle = ca;                                             /* 4220 */
                log2file("  game unpaused");                                  /* 4221 */
                addTime = time(NULL) - pauseTime;                             /* 4222 */
                if (addTime > 0)                                              /* 4223 */
                    startTime += addTime;                                     /* 4224 */
                if (checkMusicVoiceID >= 0) {                                   /* 4231 */
                    musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); /* 4233 */
                    totMusics = 0;
                    accMusics = 0.0f;
                }
                clockTimeStart = clock();                                     /* 4238 */
                QueryPerformanceCounter(&li);                                 /* 4240 */
                qpc_start = li.LowPart;                                       /* 4241 */
                timeTimeStart = time(NULL);                                   /* 4245 */
                time_cheat_count = 0;
            }
        }
        else {
            if (!itrcheck) {                                                 /* 4249 */
                poll_control(&rec_ctrl, 0);                                   /* 4251 */
                if (ply[player_id]->dead) {                                   /* 4253 */
                    log2file("  replay ended after death");                   /* 4255 */
                    playing = 0;                                              /* 4255 */
                }
                if (key[KEY_ESC]) {                                           /* 4264 */
                    log2file("  quit from replay");                           /* 4265 */
                    quit = 1;                                                 /* 4265 */
                    playing = 0;                                              /* 4265 */
                }
                if (key[KEY_SPACE]) {                                        /* 4271 */
                    if (ply[player_id]->dead == 0) {
                        log2file("  replay paused");                          /* 4272 */
                        while (key[KEY_SPACE])                                /* 4273: debounce-wait loop */
                            poll_control(&rec_ctrl, 1);
                        while (!key[KEY_SPACE] && !key[KEY_RIGHT] &&          /* 4274: pause-wait loop */
                               !key[KEY_ESC] && !key[KEY_UP]) {               /* 4274: pause-wait loop */
                            poll_control(&rec_ctrl, 1);                       /* 4275 */
                            if (key[KEY_F1]) {                                /* 4276 */
                                take_screenshot(swap_screen);                 /* 4277 */
                            /* main.c:4278's own jne loops back to its OWN fragment's start
                             * (offset 10425 == 0x4142b9 - 0x411a00), with nothing else between --
                             * a bare debounce spin on key[KEY_F1] alone, no poll_control() call
                             * (unlike the KEY_SPACE debounce loops at 4273/4281), that was simply
                             * missing from this block. */
                                while (key[KEY_F1])                            /* 4278 */
                                    ;
                            }
                        }
                        while (key[KEY_SPACE])                                /* 4281: debounce-wait loop */
                            poll_control(&rec_ctrl, 1);
                        fast_forward = 0;                                     /* 4282 */
                        fast_fast_forward = 0;                                /* 4283 */
                        log2file("  replay unpaused");                       /* 4284 */
                    }
                }
                /* NOT an else: main.c:4271's own je (SPACE not pressed) lands at offset 5965,
                 * exactly the reload that starts main.c:4284's tail; the dead!=0 fallthrough at
                 * offset 3262 lands exactly at 4287's own first fragment; and 4284's own tail
                 * falls straight into 4287's SECOND fragment (offsets 5971..5984) after finishing
                 * the pause/unpause sequence. All three paths -- space not pressed, space pressed
                 * while dead, and space pressed-and-unpaused -- converge on the same KEY_RIGHT
                 * test, so 4287..4310 run unconditionally after the block above, not only when it
                 * was skipped. */
                if (key[KEY_RIGHT]) {                                     /* 4287 */
                    fast_forward++;                                       /* 4288 */
                    fast_fast_forward = 0;                                /* 4289 */
                }
                else
                    fast_forward = 0;                                     /* 4292 */
                if (key[KEY_UP]) {                                        /* 4295 */
                    if (!ply[player_id]->dead && ply[player_id]->level < demo->floor - 10) {  /* 4296 */
                        fast_fast_forward++;                              /* 4297 */
                        fast_forward = 0;                                 /* 4298 */
                        next_floor = ((ply[player_id]->level + 100) / 100) * 100;  /* 4299 */
                        if (next_floor > demo->floor - 10)                /* 4301 */
                            next_floor = demo->floor - 10;
                    }
                    else {
                        fast_fast_forward = 0;
                        next_floor = -1;
                    }
                }
                else if (ply[player_id]->level >= next_floor || ply[player_id]->dead) {  /* 4309 */
                    fast_fast_forward = 0;                                /* 4310 */
                    next_floor = -1;
                }
            }
        }
        if (!itrcheck) {                                                     /* 4319 */
            static int someCounter;
            int ffstep; /* DWARF block 132354 [2740..2804 6150..6448]: someCounter, ffstep, drew, skipDrawing */

            someCounter++;                                                    /* 4324 */
            ffstep = fast_forward ? 4 : 1;                                   /* 4327 */
            if (fast_fast_forward)                                            /* 4330 */
                ffstep = 32;                                                  /* 4330 */
            if (!quit && someCounter % ffstep == 0) {                        /* 4337 */
                draw_frame(swap_screen);                                      /* 4338 */
                if (ply[player_id]->shake) {                                  /* 4346 */
                    acquire_screen();                                          /* gfx.inl:221/203 */
                    blit(swap_screen, swap_screen, 0, shake, 0, 0,             /* 4348: args from
                                                                                    fragments 6207..6271 (source-view
                                                                                    4340 4352) -- src=dst=swap_screen,
                                                                                    src_x=0, src_y=shake (ebp-0x96c),
                                                                                    dst_x=dst_y=0, w/h read back from
                                                                                    swap_screen's own struct fields
                                                                                    (mov (%eax),%edx / mov 0x4(%eax));
                                                                                    annotation moved to the opening
                                                                                    line for the same reason as 4126
                                                                                    above (GCC attributes a call's
                                                                                    bytes to where it opens). */
                         swap_screen->w, swap_screen->h);
                    blit_to_screen(swap_screen);                              /* 4349 */
                    release_screen();                                         /* gfx.inl:227/212 */
                } else {
                    blit_to_screen(swap_screen);                              /* 4353 */
                }
                if (!debug) {                                                 /* 4356 */
                    while (cycle_count == 0)                                  /* 4357 */
                        rest(2);                                              /* 4357 */
                } else if (key[KEY_TAB] && key[KEY_LSHIFT]) {                  /* 4360 */
                    while (cycle_count <= 7) { }                              /* 4361: busy wait */
                } else {
                    while (cycle_count == 0)
                        rest(2);                                              /* 4363 */
                }
            }
        }
        if (!itrcheck)                                                       /* 4369 */
            rest(2);
        }
    }

    /* lines 4374..4426: recording gates a small profile play-time update vs. the full
     * gameData stats snapshot + itrcheck-gated XML dump. */
    if (recording) {                                                           /* 4374 */
        diff = endTime - startTime;                                            /* 4375 */
        if (diff > 0)                                                          /* 4376 */
            profile->seconds_spent_playing += diff;                           /* 4377 */
    } else {
        gameData->score = ply[player_id]->level * 10 + ply[player_id]->score;  /* 4389 */
        gameData->floor = ply[player_id]->level;                               /* 4390 */
        gameData->combo = ply[player_id]->best_combo;                          /* 4391 */
        gameData->no_combo_top_floor = ply[player_id]->no_combo_top_floor;     /* 4392 */
        gameData->biggest_lost_combo = ply[player_id]->biggest_lost_combo;     /* 4393 */
        for (i = 0; i < 5; i++)                                                /* 4395 */
            gameData->ccc[i] = ply[player_id]->ccc[i];                        /* 4395 */
        for (i = 0; i < 5; i++)                                                /* 4398 */
            gameData->jc[i] = ply[player_id]->jcTop[i];                       /* 4398 */
        {
            int keys_pressed[7] = {0};
            int key_flag[7] = { 16, 1, 2, 4, 8, 32, 128 };                     /* 4403 */
            int last_keys[7] = {0};
            int k;


            if (demo->size > 0) {                                              /* 4406 */

                for (i = 0; i < demo->size; i++) {                             /* 4406 */
                    int flags = demo->data[i].key_flags;                       /* 4406 */
                    for (k = 0; k < 7; k++) {                                  /* 4408 */
                        if (!last_keys[k] && (key_flag[k] & flags)) /* 4409 */
                            keys_pressed[k]++; /* 4410 */
                        last_keys[k] = key_flag[k] & flags;                                      /* 4412 */
                    }
                }
            }
            gameData->jump = keys_pressed[0];                                  /* 4416 */
            gameData->left = keys_pressed[1];                                  /* 4417 */
            gameData->right = keys_pressed[2];                                 /* 4418 */
        }
        if (itrcheck) {                                                       /* 4421 */
            char *xmlStr = getGameDataXML(gameData);                          /* 4422 */
            printf("%s", xmlStr);                                             /* 4423 */
            free(xmlStr);                                                     /* 4424 */
        }
        if (itrcheck)                                                         /* 4426 */
            return 0;
    }

    /* lines 4456..4458 */
    log2file(" play ended");                                                  /* 4456 */
    fast_forward = 0;                                                         /* 4457 */
    fast_fast_forward = 0;                                                    /* 4458 */

    /* lines 4500..4643: demo/profile stat snapshot (recording && !quit only), then
     * syncProfileFromOptions()/save_profile() unconditionally, then the quit/closeButtonClicked
     * guard around highscore qualification. Traced from three save_profile() call sites all
     * tagged historical line 4641 (offsets 8235, 9307, 10956): the !recording predecessor
     * (offset 8179) calls sync+save BEFORE ever testing quit (offset 8240's test comes after
     * the call, not before it); the recording&&quit predecessor jumps straight past the whole
     * replay-file block to its own sync+save copy (offset 10900, target of the "jne 414494"
     * at offset 8315); the recording&&!quit predecessor falls through the replay-file block
     * into a third sync+save copy (offset 9251) whose *own* trailing test reads
     * closeButtonClicked directly (offset 9312, "cmpl $0x0,closeButtonClicked") rather than
     * quit -- because on that path quit was already resolved false by the earlier test at
     * offset 8308. That is only consistent with sync+save being unconditional statements
     * textually AFTER this whole if/else, not folded into either arm or gated by !quit. */
    if (recording) {                                                          /* 4500 */
        if (!quit) {                                                          /* 4500 */
            demo->score = ply[player_id]->level * 10 + ply[player_id]->score;  /* 4503 */
            demo->floor = ply[player_id]->level;                               /* 4504 */
            demo->combo = ply[player_id]->best_combo;                         /* 4505 */
            demo->rejump = options.jump_hold;                                  /* 4506 */
            demo->no_combo_top_floor = ply[player_id]->no_combo_top_floor;     /* 4507 */
            demo->biggest_lost_combo = ply[player_id]->biggest_lost_combo;     /* 4508 */
            for (i = 0; i < 5; i++)                                            /* 4510 */
                demo->ccc[i] = ply[player_id]->ccc[i];                        /* 4510 */
            for (i = 0; i < 5; i++)                                            /* 4513 */
                demo->jc[i] = ply[player_id]->jcTop[i];                       /* 4513 */

            if (!is_playing_custom_game) {                                    /* 4519 */
                profile->games_played++;                                      /* 4520 */
                profile->total_floors += demo->floor;                         /* 4522 */
                profile->total_score += demo->score;                          /* 4523 */
                profile->total_combos += numComboJumps;                       /* 4524 */
                profile->total_combo_floors += totComboFloors;                /* 4525 */
                for (i = 0; i < 5; i++) {                                      /* 4526 */
                    if (demo->ccc[i] > 0) {                                    /* 4527 */
                        profile->cccNum[i]++;                                  /* 4528 */
                        profile->cccTotal[i] += demo->ccc[i];                  /* 4529 */
                    }
                }
            } else {
                profile->custom_games_played++;                               /* 4534 */
            }

            /* lines 4541..4624: replay directory + per-category replay files */
            if (!file_exists(replay_directory, -1, NULL))                     /* 4541 */
                mkdir(replay_directory);                                      /* 4543 */

            if (!is_playing_custom_game) {                                    /* 4550 */
                int rank;

                if (profile->best_floor < demo->floor) {                      /* 4551 */
                    profile->best_floor = demo->floor;                        /* 4552 */
                    myDeleteFile(replay_directory, profile->best_replay_names[2]);  /* 4553 */
                    sprintf(profile->best_replay_names[2], "%s_best_floor_%d.itr",  /* 4554 */
                            profile->handle, demo->floor);
                    save_replay(replay_directory, profile->best_replay_names[2], demo,  /* 4555 */
                                rec_pos + 2, 1);
                    new_personal_best[2] = 1;                                  /* 4556 */
                }
                if (profile->best_combo < demo->combo) {                      /* 4559 */
                    profile->best_combo = demo->combo;                        /* 4560 */
                    myDeleteFile(replay_directory, profile->best_replay_names[1]);  /* 4561 */
                    sprintf(profile->best_replay_names[1], "%s_best_combo_%d.itr",  /* 4562 */
                            profile->handle, demo->combo);
                    save_replay(replay_directory, profile->best_replay_names[1], demo,  /* 4563 */
                                rec_pos + 2, 1);
                    new_personal_best[1] = 1;                                  /* 4564 */
                }
                if (profile->best_score < demo->score) {                      /* 4567 */
                    profile->best_score = demo->score;                        /* 4568 */
                    myDeleteFile(replay_directory, profile->best_replay_names[0]);  /* 4569 */
                    sprintf(profile->best_replay_names[0], "%s_best_score_%d.itr",  /* 4570 */
                            profile->handle, demo->score);
                    save_replay(replay_directory, profile->best_replay_names[0], demo,  /* 4571 */
                                rec_pos + 2, 1);
                    new_personal_best[0] = 1;                                  /* 4572 */
                }
                if (profile->no_combo_top_floor < ply[player_id]->no_combo_top_floor) {  /* 4575 */
                    profile->no_combo_top_floor = ply[player_id]->no_combo_top_floor;  /* 4576 */
                    myDeleteFile(replay_directory, profile->best_replay_names[4]);  /* 4577 */
                    sprintf(profile->best_replay_names[4], "%s_best_no_combo_%d.itr",  /* 4578 */
                            profile->handle, demo->no_combo_top_floor);
                    save_replay(replay_directory, profile->best_replay_names[4], demo,  /* 4579 */
                                rec_pos + 2, 1);
                    new_personal_best[4] = 1;                                  /* 4580 */
                }
                if (profile->biggest_lost_combo < ply[player_id]->biggest_lost_combo) {  /* 4583 */
                    profile->biggest_lost_combo = ply[player_id]->biggest_lost_combo;  /* 4584 */
                    myDeleteFile(replay_directory, profile->best_replay_names[3]);  /* 4585 */
                    sprintf(profile->best_replay_names[3], "%s_best_lost_combo_%d.itr",  /* 4586 */
                            profile->handle, demo->biggest_lost_combo);
                    save_replay(replay_directory, profile->best_replay_names[3], demo,  /* 4587 */
                                rec_pos + 2, 1);
                    new_personal_best[3] = 1;                                  /* 4588 */
                }
                for (rank = 1; rank < 6; rank++) {                             /* 4591 */
                    if (profile->ccc[rank - 1] < ply[player_id]->ccc[rank - 1]) {  /* 4592 */
                        profile->ccc[rank - 1] = ply[player_id]->ccc[rank - 1];  /* 4593 */
                        myDeleteFile(replay_directory, profile->best_replay_names[4 + rank]);  /* 4594 */
                        sprintf(profile->best_replay_names[4 + rank], "%s_best_cc%d_%d.itr",  /* 4595 */
                                profile->handle, rank, ply[player_id]->ccc[rank - 1]);
                        save_replay(replay_directory, profile->best_replay_names[4 + rank],  /* 4596 */
                                    demo, rec_pos + 2, 1);
                        new_personal_best[4 + rank] = 1;                       /* 4597 */
                    }
                }
                for (rank = 1; rank < 6; rank++) {                             /* 4601 */
                    if (profile->jc[rank - 1] < ply[player_id]->jcTop[rank - 1]) {  /* 4602 */
                        profile->jc[rank - 1] = ply[player_id]->jcTop[rank - 1];  /* 4603 */
                        myDeleteFile(replay_directory, profile->best_replay_names[9 + rank]);  /* 4604 */
                        sprintf(profile->best_replay_names[9 + rank], "%s_best_jj%d_%d.itr",  /* 4605 */
                                profile->handle, rank, ply[player_id]->jcTop[rank - 1]);
                        save_replay(replay_directory, profile->best_replay_names[9 + rank],  /* 4606 */
                                    demo, rec_pos + 2, 1);
                        new_personal_best[9 + rank] = 1;                       /* 4607 */
                    }
                }
            }

            if (save_replay(replay_directory, "last_game.itr", demo, rec_pos + 2, 1) < 0) {  /* 4613 */
                my_alert("Failed to save replay.", "(last_game.itr)", 0, 1);  /* 4614 */
                uberChecksum = 0;                                             /* 4615 */
            } else {
                char fbuf[2048];
                Treplay *rr;

                sprintf(fbuf, "%slast_game.itr", replay_directory);          /* 4620 */
                rr = load_replay(fbuf);                                      /* 4621 */
                if (rr) {                                                    /* 4622 */
                    uberChecksum = calc_replay_checksum(demo);               /* 4623 */
                    destroy_replay(rr);                                     /* 4624 */
                }
            }
        }
    }

    /* lines 4641..4643: unconditional, reached from all three predecessors above */
    syncProfileFromOptions();                                                /* 4641 */
    save_profile(profile);                                                   /* 4641 */

    {
        /* DWARF block 133269 begins after the replay filename buffer's scope. */
        float hy;
        int gotHigh;
        int qualify[15];
        int qualifyValue[15];
        int gameover_bmp_id;

    /* lines 4643..4683: highscore qualification, guarded by quit && closeButtonClicked */
    play_again = 0;
    if (!quit && !closeButtonClicked) {
                                           
            int rank;   


            for (i = 0; i < 15; i++)                                         
                qualify[i] = 0;                                             
            qualifyValue[0] = ply[player_id]->level * 10 + ply[player_id]->score;  
            qualifyValue[2] = ply[player_id]->level;                         
            qualifyValue[1] = ply[player_id]->best_combo;                    
            qualifyValue[3] = ply[player_id]->biggest_lost_combo;            
            qualifyValue[4] = ply[player_id]->no_combo_top_floor;            
            for (i = 0; i < 5; i++) {                                        
                qualifyValue[5 + i] = ply[player_id]->ccc[i];                
                qualifyValue[10 + i] = ply[player_id]->jcTop[i];             
            }
                                                       
            gotHigh = 0;                                                    
            for (rank = 0; rank < 15; rank++) {                              
                qualify[rank] = qualify_hisc_table(hisc_tables[rank], qualifyValue[rank]);  
                gotHigh += qualify[rank];                                    
            }
                                                 

















            if (recording) {                                                
                gameover_bmp_id = (gotHigh > 0) ? 0x3e : 0x37;                  


            } else {
                                                   





                gotHigh = 0;                                                
                gameover_bmp_id = 0x37;                                     
            }
            if (is_playing_custom_game)                                     
                gameover_bmp_id = 0x37;                                     

            if (gotHigh) {                                                     


                if (!is_playing_custom_game) {                              
                    log2file(" player qualified for highscore");            
                    play_sound(sounds[7], 0, 0);                            
                }
            } else {
                log2file(" player did not qualify for highscore");          
                play_sound(speaker[1], 0, 0);                               
            }

            
                if (!debug) {
       /* original 9619 branches around the results work */
        hy = 480.0f;       /* 4704: panel starts off-screen at 480 and eases up toward 130.0f */
        int alpha_pos;      /* letter-navigation cursor; first write is at 4821 (rank-up reset) */
        char *initials = NULL;

        {
            /* DWARF and instructions separate four name-entry variables: alpha_pos
             * at stack -0x930 indexes letters[], pos in esi indexes buf[], done at
             * stack -0x938 controls the loop, and skip_keys in edi delays repeat
             * input. The previous candidate conflated these states. scrollerY is
             * not read from the earlier results loop; its DWARF register range
             * starts at offset 12095 in this inner block. */
            int scrollerY;   /* ticker-bar Y offset; %ebx from the 4821 reset (-20) through the
                               * 4831-4838 easing -- was wrongly conflated with alpha_pos before. */
            char letters[31] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ .\244\0";
            int len;
            char buf[8] = { '.', 0, '.', 0, '.', 0, 0, 0 };  /* 4689 */
            int done;
            int skip_keys;
            int pos;
            int isGuest;
            int new_rank_id;
            int rank_bmp_id;
            int rank_y;
            int k;
            char guestName[4];
            char postName[32];

            len = strlen(letters) - 1;                                              /* 4688 */
            isGuest = !stricmp(profile->handle, "guest");                           /* 4692 */

            /* qualify, qualifyValue and gotHigh are NOT recomputed here: they are declared in
             * the enclosing DWARF block 133269, which opens in REGION W4 at main.c:4650 and
             * does not close until the end of the function, so W4's own computation (main.c
             * 4650-4664, and the recording/is_playing_custom_game handling through 4671) is
             * still in scope and still holds. A prior pass at this region assumed the opposite
             * ("this region cannot see the values") and inserted an unannotated fourteen-
             * statement duplicate of W4's block here; function_lines.py --source-view 4650 4687
             * shows the original computes qualify[]/qualifyValue[]/gotHigh exactly ONCE, entirely
             * within W4's own address range (offsets 9327-9497), with nothing resembling it
             * again before 4687. The duplicate also disagreed with W4's real logic (W4 only
             * zeroes gotHigh when !recording, never when is_playing_custom_game while recording
             * -- the deleted duplicate zeroed it in both cases). Removed. */

            /* results screen: slide the results panel in and wait for the
             * highscore chime / fade timer (4695..4737). */
            for (;;) {
                cycle_count = 0;                                                     /* 4696 */
                ply[player_id]->dead -= 16;                                          /* 4697 */
                hy = hy + (130.0f - hy) * 0.1;                                       /* 4699 */
                update_frame();                                                      /* 4700 */
                for (i = 0; i < 512; i++)                                            /* 4701 */
                    if (stars[i].intensity)
                        update_particle(&stars[i]);
                if ((unsigned)(hurry_y + 99) <= 578u)                                 /* 4702 */
                    hurry_y -= 2;
                draw_frame(swap_screen);                                             /* 4703 */
                /* 4704 */
                draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify,
                             qualifyValue,
                             is_playing_custom_game ? 0 : (recording != 0));
                if (isGuest && gotHigh && !is_playing_custom_game && recording) { /* 4705 */
                    /* 4706 */ textout_centre_ex(swap_screen, data[52].dat, "Enter your initials",
                                       320, (int)(hy * 2.0 + 80.0), -1, -1);
                }
                if (falling)                                                   /* 4709: cmp/sbb */
                    falling++;
                if (falling > ply[player_id]->level * 5 || falling > 250) {     /* 4710 */
                    play_sound(sounds[6], 0, 1);                                      /* 4711 */
                    if (custom.falling)                                               /* 4712 */
                        stop_sample(custom.falling);
                    ply[player_id]->shake = 24;                                       /* 4714 */
                    falling = 0;
                }
                if (ply[player_id]->shake) {                                          /* 4716 */
                    acquire_screen();                                                /* 4716: inlined */
                    /* 4718 */ blit(swap_screen, screen, 0, new_rand() % 8, 0, 0,
                         swap_screen->w, swap_screen->h);
                    release_screen();                                                /* 4718: inlined */
                    ply[player_id]->shake--;                                          /* 4720 */
                } else
                    blit_to_screen(swap_screen);                                          /* 4722 */
                if (key[KEY_F1]) {                                                    /* 4725 */
                    take_screenshot(swap_screen);                                     /* 4726 */
                    while (key[KEY_F1]) { }  /* 4727: wait for release at 13091..13105 */
                }
                if (!key[KEY_TAB] || !key[KEY_LSHIFT]) { /* 4731 */
                    while (cycle_count == 0)
                        rest(2); /* 4734 */
                }
                if (!(hy > 140.0f))  /* sole original 140.0f comparison at offset 11863 */
                    break;
            }
            ply[player_id]->dead = 0;                                                 /* 4737 */
            clear_keybuf();                                                           /* 4738 */

            /* summary scroller message: custom/guest/personal-record tip (4742..4775). */
            if (!recording)
                summary_scroller_message[0] = 0;                                      /* 4743 */
            else {
                if (is_playing_custom_game) {
                    /* 4746: annotate the statement's FIRST physical line, not just the last --
                     * an unannotated opening line inherits the previous statement's historical
                     * line (4743) and misattributes this call's bytes to it. */
                    memcpy(summary_scroller_message,               /* 4746 */
                           "Custom mode is crazy fun but does not add to your profile. "
                           "Play Classic Mode to compete in the highscore lists and "
                           "climb in rank!", 0x82);                                    /* 4746 */
                } else {
                    if (gotHigh)                                                         /* 4749 */
                        memcpy(summary_scroller_message, "New personal records!    ", 0x1a); /* 4753 */
                    strcpy(summary_scroller_message,
                           isGuest ? "You're playing in guest mode. Start a profile and record your progress!"
                                   : hints[new_rand() % 45]);
                }
            }

            /* name entry loop: scroller/rank banner setup and per-frame draw + input
             * (4781..4923). */
            /* 4781 */ init_scroller(&summary_scroller, data[54].dat, summary_scroller_message,
                           640, 30, -1);
            scroll_scroller(&summary_scroller, -150);                                  /* 4782 */
            new_rank_id = get_rank_id(profile);                                        /* 4787 */
            rank_bmp_id = new_rank_id + 0x4a;
            alpha_pos = 0;
            rank_y = 0x244;
            scrollerY = -20;
            skip_keys = 0;
            pos = 0;
            done = 20;

            for (;;) {
                if (done == 0)                          /* 4792: cmp $0, done slot -0x938 */
                    break;
                if (closeButtonClicked)                                                /* 4793 */
                    return 0;
                cycle_count = 0;                                                       /* 4797 */
                step_count++;                                                          /* 4798 */
                update_frame();                                                        /* 4800 */
                if (key[KEY_F1]) {                                                     /* 4802 */
                    take_screenshot(swap_screen);                                      /* 4803 */
                    while (key[KEY_F1]) { }                                            /* 4804 */
                }
                if ((unsigned)(hurry_y + 99) <= 578u)                                   /* 4808 */
                    hurry_y -= 2;
                draw_frame(swap_screen);                                               /* 4809 */
                /* 4810 */ draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify,
                             qualifyValue, is_playing_custom_game ? 0 : (recording != 0));
                if (isGuest && gotHigh && !is_playing_custom_game && recording) {  /* 4811 */
                    /* 4812 */ textout_centre_ex(swap_screen, data[52].dat, "Enter your initials",
                                       320, (int)(hy * 2.0 + 80.0), -1, -1);
                    if (pos != 0 || (step_count & 4))                                   /* 4814 */
                        textout_centre_ex(swap_screen, data[52].dat, &buf[0],
                                           300, (int)(hy * 2.0 + 120.0), -1, -1);
                    if (pos != 1 || (step_count & 4))                                   /* 4815 */
                        textout_centre_ex(swap_screen, data[52].dat, &buf[2],
                                           320, (int)(hy * 2.0 + 120.0), -1, -1);
                    if (pos != 2 || (step_count & 4))                                   /* 4816 */
                        textout_centre_ex(swap_screen, data[52].dat, &buf[4],
                                           340, (int)(hy * 2.0 + 120.0), -1, -1);
                    if (pos == 3 && (step_count & 4))                                   /* 4817 */
                        textout_centre_ex(swap_screen, data[52].dat, "%",
                                           360, (int)(hy * 2.0 + 120.0), -1, -1);
                }
                if (new_rank_id != current_rank_id) {                                   /* 4820 */
                    /* 4821: banner state is initialized before the loop, as in
                     * original offsets 12034..12075. The rank-change branch
                     * starts with the bitmap lookup at offset 12351. */
                    draw_sprite(swap_screen, data[rank_bmp_id].dat, 20, rank_y);         /* 4821 (inlined) */
                    /* 4822 */ textout_ex(swap_screen, data[52].dat, "rank up!",
                               20, rank_y + 0x46, -1, -1);
                    /* 4823 */ rank_y = (int)((320 - rank_y) * 0.1 + rank_y);
                    /* current_rank_id = new_rank_id was here in an earlier pass, but
                     * local_slot_trace for current_rank_id shows exactly 3 accesses in the
                     * whole function -- init at 3441, a write at 3443 (both before region
                     * W1a), and the single read at 4820 -- with NO write anywhere inside this
                     * loop. Removed as invented: nothing in the trace supports it, and per
                     * large-body-reconstruction-pitfalls this makes new_rank_id!=current_rank_id
                     * loop-invariant-true after the first rank-up, i.e. the block re-triggers
                     * every frame, which is consistent with the evidence even if it looks like
                     * a game bug. */
                    /* Tried: moving this draw_sprite/textout_ex/easing out of the if-block to
                     * run unconditionally every frame (on the theory that evidence/census/
                     * line-mappings.json shows exactly two draw.inl:238 sites in the whole
                     * function, both here, so rank_y should have two readers). Measured worse
                     * on every axis: play grew 16556->16678 bytes and this line's own delta
                     * went from +70 (4820) to +168, so reverted. The second draw.inl:238 site
                     * (offset 0x35b3, historical) is real but is NOT reached by simply hoisting
                     * this draw out of the if -- its actual source position is still unknown. */
                }
                if (summary_scroller_message[0]) {                                       /* 4827 */
                    scroll_scroller(&summary_scroller, -2);                              /* 4828 */
                    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                              /* 4829 */
                    set_trans_blender(0, 0, 0, 110);                                     /* 4830 */
                    /* 4831-4833: original computes these through draw.inl's inlined rectfill,
                     * with y1=scrollerY and y2=scrollerY+20/+18/+16 (confirmed via
                     * `lea 0x14(%ebx),%eax` / `lea 0x12(%ebx),%eax` / `lea 0x10(%ebx),%eax`
                     * at offsets 13301/13375/13449, all inside scrollerY's DWARF-register
                     * range) -- not the constants this used to hardcode. */
                    rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 20, makecol(0, 0, 0)); /* 4831 */
                    rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 18, makecol(0, 0, 0)); /* 4832 */
                    rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 16, makecol(0, 0, 0)); /* 4833 */
                    solid_mode();                                                         /* 4834 */
                    /* 4835/4836: the draw_scroller 4th argument is 0xc(%esp)=%ebx at offsets
                     * 13519/13584, both inside scrollerY's register range -- not alpha_pos. */
                    draw_scroller(&summary_scroller, swap_screen, 1, scrollerY,          /* 4835 */
                                   makecol(150, 150, 150));
                    if (!draw_scroller(&summary_scroller, swap_screen, 0, scrollerY,     /* 4836 */
                                        makecol(200, 200, 200)))
                        restart_scroller(&summary_scroller);
                }
                /* 4838: this easing computation (ebx=ebx+(int)(-ebx*0.1)) sits at offset
                 * 13633-13707, inside scrollerY's DWARF register range (13150-13795), not
                 * alpha_pos's (alpha_pos's own local_slot_trace has no access at 4838 at all).
                 * scrollerTargetY has no DWARF location (eliminated), consistent with the
                 * target being the constant 0 folded into this formula, the same way
                 * hyTarget==130.0f and rankTargetY==320 are already folded into hy/rank_y. */
                scrollerY = (int)(-scrollerY * 0.1 + scrollerY);  /* 4838 */

                if (falling) falling++; /* 4843 */
                if (falling > ply[player_id]->level * 5 || falling > 250) {         /* 4844 */
                    play_sound(sounds[6], 0, 1);                                          /* 4845 */
                    if (custom.falling)                                                   /* 4846 */
                        stop_sample(custom.falling);
                    ply[player_id]->shake = 24;                                           /* 4850 */
                    falling = 0;
                }
                if (ply[player_id]->shake) {                                              /* 4852 */
                    acquire_screen();                                                    /* 4852: inlined */
                    /* 4855 */ blit(swap_screen, screen, 0, new_rand() % 8, 0, 0,
                         swap_screen->w, swap_screen->h);
                    release_screen();                                                    /* 4855: inlined */
                    ply[player_id]->shake--;                                              /* 4857 */
                } else
                    blit_to_screen(swap_screen);                                              /* 4860 */

                if (isGuest && gotHigh && !is_playing_custom_game && recording) {      /* 4863 */
                    poll_control(&ctrl, 0);                                                /* 4864 */
                    if (keypressed()) {                                                     /* 4865 */
                        if (done == 20) {                                                    /* 4865 */
                            k = readkey() & 0xff;                                            /* 4866 */
                            k -= 0x20;
                            if (k == -24)                                                    /* 4867 */
                                k = (signed char)0xa4;
                            else if (k == 14)                                               /* 4868 */
                                k = '.';
                            else if (k == 1)                                                /* 4869 */
                                k = '!';
                            if (k != 0x20) {                                                /* 4870 */
                                for (i = 0; i < len; i++) {                                 /* 4871 */
                                    char typed = letters[i];
                                    if ((signed char)typed == k) {                          /* 4872 */
                                        buf[pos * 2] = typed;                               /* 4875 */
                                        pos++;                                              /* 4876 */
                                        alpha_pos = i;
                                        skip_keys = 100;
                                        if (pos == 3)                                      /* 4877 */
                                            done = 19;
                                    }
                                }
                            }
                        }
                    }
                    if (skip_keys) {
                        skip_keys--;
                    } else {
                    if (is_right(&ctrl)) {                                                 /* 4884 */
                        alpha_pos++;                                                        /* 4885 */
                        skip_keys = 8;
                        if (alpha_pos > len)                                                /* 4886 */
                            alpha_pos = 0;
                    }
                    if (is_left(&ctrl)) {                                                  /* 4889 */
                        skip_keys = 8;
                        alpha_pos--;                                                        /* 4891 */
                        if (alpha_pos < 0)
                            alpha_pos = len;
                    }
                    if (is_fire(&ctrl)) {                                                  /* 4894 */
                        if (letters[alpha_pos] == (char)0xa4) {                            /* 4895: blank slot confirmed */
                            if (pos != 0) {                                                 /* 4895 */
                                buf[pos * 2] = '.';                                         /* 4896 */
                                pos--;                                                      /* 4897 */
                            }
                            skip_keys = 100;
                        } else if (pos <= 1) {                                              /* 4899 */
                            pos++;
                            skip_keys = 100;
                        } else {
                            if (done == 20) {                                               /* 4900 */
                                pos++;                                                      /* 4902 */
                                done = 19;
                            }
                            skip_keys = 100;
                        }
                    }
                    if (key[KEY_DEL] || key[KEY_BACKSPACE]) {                               /* 4906 */
                        buf[pos * 2] = '.';                                                 /* 4907 */
                        skip_keys = 7;
                        if (pos != 0)                                                       /* 4908 */
                            pos--;
                    } else if (skip_keys) {
                        skip_keys--;                                                        /* 4912 */
                    }
                    }
                    if (!is_any(&ctrl) && !key[KEY_DEL] && !key[KEY_BACKSPACE])             /* 4913 */
                        skip_keys = 0;
                    if (pos <= 2)
                        buf[pos * 2] = letters[alpha_pos];                                  /* 4915 */
                }
                if (done != 20)                                                             /* 4918 */
                    done--;  /* original 12918..12930, reached from both paths */
                poll_control(&ctrl, 0);                                                     /* 4920 */
                if (!isGuest || !gotHigh || is_playing_custom_game) {                         /* 4921 */
                    if (keypressed() || is_fire(&ctrl)) {                                   /* 4922 */
                        if (done == 20)                                                     /* 4923 */
                            done = 14;  /* original 12956..13003, 13732 */
                    }
                }
                if (!key[KEY_TAB] || !key[KEY_LSHIFT]) { /* 4929 */
                    while (cycle_count == 0)
                        rest(2); /* 4932 */
                }
            }

            /* highscore entry: commit the typed initials into every qualified table
             * (4940..4951). */
            if (!is_playing_custom_game && recording) {                                  /* 4940 */
                guestName[0] = buf[0];                                                       /* 4941 */
                guestName[1] = buf[2];
                guestName[2] = buf[4];
                guestName[3] = 0;
                strcpy(postName, isGuest ? guestName : profile->handle);
                initials = postName;
                for (k = 0; k < 15; k++) {                                                   /* 4948 */
                    if (qualify[k] > 0) {                                                    /* 4949 */
                        enter_hisc_table(hisc_tables[k], qualifyValue[k], initials);          /* 4950 */
                        sort_hisc_table(hisc_tables[k]);                                      /* 4951 */
                    }
                }
            }

        } /* original debug route rejoins before the unlock check */

        }
            /* post-game "new start floor unlocked" message (4963..4987). */
            if (recording && !debug && !is_playing_custom_game) {                        /* 4963 */
                int f = ply[player_id]->level / 100;                                          /* 4964 */
                if (f > oldUnlockedFloors && f <= 9) {                                        /* 4966 */
                    fadeOut(16);                                                              /* 4968 */
                    blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);                   /* 4971 */
                    set_trans_blender(0, 0, 0, 158);                                          /* 4974 */
                    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                                   /* 4975 */
                    {                                                                           /* 4976 */
                        int black = makecol(0, 0, 0);
                        int width = 0;
                        int height = 0;
                        GFX_DRIVER *driver = gfx_driver;
                        if (driver) {
                            width = driver->w;
                            height = driver->h;
                        }
                        rectfill(swap_screen, 0, 0, width, height, black);
                    }
                    solid_mode();                                                             /* 4977 */
                    draw_sprite(swap_screen, data[58].dat,
                                320 - ((BITMAP *)data[58].dat)->w / 2, 20);                    /* 4980 (inlined) */
                    /* 4981 */ textout_centre_ex(swap_screen, data[54].dat, "A new start floor",
                                       320, 0x12c, -1, -1);
                    /* 4982 */ textout_centre_ex(swap_screen, data[54].dat, "has been unlocked!",
                                       320, 0x15e, -1, -1);
                    /* 4983 */ textout_centre_ex(swap_screen, data[54].dat,
                                       "(Get it in the options menu)",
                                       320, 0x1b8, -1, -1);
                    play_sound(sounds[2], 0, 0);                                                /* 4984 */
                    fadeIn(swap_screen, 16);                                                   /* 4985 */
                    while (key[KEY_ESC] || key[KEY_ENTER] || key[KEY_SPACE]) {  /* 4986 */
                    }
                    while (!key[KEY_ESC] && !key[KEY_ENTER] && !key[KEY_SPACE]) {  /* 4986 */
                    }
                }
            }
        save_config();                                                                        /* 4994 */
        stopGameMusic();                                                                       /* 4997 */
        if (checkMusicVoiceID >= 0)                                                            /* 4998 */
            voice_stop(checkMusicVoiceID);                                                     /* 4999 */

        if (recording && !debug) {
            in_replay_menu = 1;
            play_again = do_replay_menu();
            in_replay_menu = 0;
        }
    }
        if (recording) play_sound(speaker[2], 0, 0);
        stopGameMusic();
        if (checkMusicVoiceID >= 0) voice_stop(checkMusicVoiceID);
        clear_bitmap(screen);
    }
    return play_again;
}