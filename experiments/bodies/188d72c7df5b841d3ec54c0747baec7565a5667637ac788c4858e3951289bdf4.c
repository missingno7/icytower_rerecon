{
    PACKFILE *fp;
    RGB black;
    int i;
    char title[64];
    char tmpHandle[32];
    WSADATA wsaData;
    WORD wVersionRequested;
    char cfgfilename[256];

    tmpHandle[0]=0; /* 1382 */
    init_ok=0; /* 1385 */
    log2file("\nINIT GAME"); /* 1393 */
    packfile_password(NULL); /* 1394 */
    sprintf(title,"Icy Tower v%s","1.5.1"); /* 1395 */
    set_window_title(title); /* 1395 */
    wVersionRequested=MAKEWORD(2,2); /* 1403 */
    if (WSAStartup(wVersionRequested,&wsaData)!=0)
        log2file(" !!! Failed to setup Winsock"); /* 1404 */
    if (LOBYTE(wsaData.wVersion)<2 || HIBYTE(wsaData.wVersion)<2) /* 1407 */
        log2file(" !!! Failed to get proper Winsock version (wanted 2.2, got %d.%d)",
                 LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion)); /* 1408 */
    play_char.max=0; /* 1413 */
    play_char.value=0; /* 1413 */
    play_char.bmp=NULL; /* 1414 */
    eyecandy_selection.value=0; /* 1416 */
    eyecandy_selection.size=3; /* 1417 */
    eyecandy_selection.caption[0]=strdup("Lots"); /* 1418 */
    eyecandy_selection.caption[1]=strdup("Some"); /* 1419 */
    eyecandy_selection.caption[2]=strdup("None"); /* 1420 */
    scroll_speed_selection.value=0; /* 1422 */
    scroll_speed_selection.size=6; /* 1423 */
    scroll_speed_selection.caption[5]=strdup("Normal"); /* 1424 */
    scroll_speed_selection.caption[4]=strdup("Hasty"); /* 1425 */
    scroll_speed_selection.caption[3]=strdup("Fast"); /* 1426 */
    scroll_speed_selection.caption[2]=strdup("Faster"); /* 1427 */
    scroll_speed_selection.caption[1]=strdup("Fastest"); /* 1428 */
    scroll_speed_selection.caption[0]=strdup("Insane"); /* 1429 */
    floor_size_selection.value=0; /* 1431 */
    floor_size_selection.size=5; /* 1432 */
    floor_size_selection.caption[0]=strdup("Wide"); /* 1433 */
    floor_size_selection.caption[1]=strdup("Normal"); /* 1434 */
    floor_size_selection.caption[2]=strdup("Shorter"); /* 1435 */
    floor_size_selection.caption[3]=strdup("Shortest"); /* 1436 */
    floor_size_selection.caption[4]=strdup("Tiny"); /* 1437 */
    floor_size_selection.value=2; /* 1438 */
    gravity_selection.value=0; /* 1440 */
    gravity_selection.size=3; /* 1441 */
    gravity_selection.caption[0]=strdup("Helium"); /* 1442 */
    gravity_selection.caption[1]=strdup("Normal"); /* 1443 */
    gravity_selection.caption[2]=strdup("Heavy"); /* 1444 */
    fldads_start(); /* 1448 */
    if (argc>2) { /* 1489 */
        int check;
        char *checkFile;
        checkFile=NULL;
        check=0;
        for (i=1;i<argc;i++) { /* 1493 */
            if (argv[i][0]!='-') checkFile=argv[i]; /* 1494 */
            if (!stricmp(argv[i],"-check")) check=1; /* 1497 */
            if (!stricmp(argv[i],"-jumps")) cmdline.jumps=1; /* 1500 */
            if (!stricmp(argv[i],"-combos")) cmdline.combos=1; /* 1503 */
            if (!stricmp(argv[i],"-sd")) cmdline.sd=1; /* 1506 */
            if (!stricmp(argv[i],"-keys")) cmdline.keys=1; /* 1509 */
            if (!stricmp(argv[i],"-all")) { /* 1512 */
                cmdline.keys=1; /* 1513 */
                cmdline.jumps=1; /* 1514 */
                cmdline.combos=1; /* 1515 */
                cmdline.sd=1; /* 1516 */
            }
            if (!stricmp(argv[i],"-tiny")) cmdline.tiny=1; /* 1518 */
        }
        if (check) { /* 1524 */
            log2file("Loading %s",argv[2]); /* 1525 */
            demo=load_replay(checkFile); /* 1526 */
            if (!demo) { /* 1527 */
                set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1528 */
                printf("<itrcheck_results status=\"error\">%s</itrcheck_results>\n",
                       get_filename(checkFile)); /* 1529 */
                log2file("*** Failed!"); /* 1530 */
                dropped_file_is_not_a_replay=1; /* 1531 */
                return 0; /* 1532 */
            }
        }
        else {
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1536 */
            allegro_message("<%s>\nis not a vaild option.",argv[1]); /* 1537 */
            log2file("*** Erroneous option (%s)",argv[1]); /* 1538 */
            dropped_file_is_not_a_replay=1; /* 1539 */
            return 0;
        }
        itrcheck=1; /* 1544 */
        log2file("ITRCHECK activated, checking <%s>",checkFile); /* 1545 */
    } else if (argc==2) { /* 1550 */
        log2file("Loading %s",argv[1]); /* 1551 */
        demo=load_replay(argv[1]); /* 1552 */
        if (!demo) { /* 1553 */
            strcpy(tmpHandle,get_filename(argv[1])); /* 1555 */
            {
                char *ext;
                ext=get_extension(tmpHandle); /* 1556 */
                ext[-1]=0; /* 1557 */
            }
            profile=load_profile(tmpHandle); /* 1558 */
            if (!profile) { /* 1559 */
                tmpHandle[0]=0; /* 1560 */
                set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1561 */
                allegro_message("The file\n<%s>\nis not a vaild Icy Tower profile.",
                               get_filename(argv[1])); /* 1562 */
                log2file("*** Failed!"); /* 1563 */
                dropped_file_is_not_a_replay=1; /* 1564 */
                return 0; /* 1565 */
            }
            free(profile); /* 1567 */
            profile=NULL; /* 1568 */
        }
    }
    log2file("Creating hiscore tables"); /* 1575 */
    for (i=0;i<15;i++) { /* 1576 */
        hisc_tables[i]=make_hisc_table(hisc_names[i]); /* 1577 */
        if (!hisc_tables[i]) { /* 1578 */
            log2file(" *** failed"); /* 1579 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1580 */
            allegro_message("Failed reserve memory for highscore table."); /* 1581 */
            return 0;
        }
    }
    for (i=0;i<15;i++) /* 1585 */
        reset_hisc_table(hisc_tables[i],"Harold",1000,0); /* 1586 */
    log2file("Initiating controls"); /* 1590 */
    init_control(&ctrl); /* 1591 */
    get_configfile_path(cfgfilename,sizeof(cfgfilename)); /* 1595 */
    log2file("Loading config file"); /* 1597 */
    fp=pack_fopen(cfgfilename,"rp"); /* 1598 */
    if (fp) { /* 1599 */
        load_options(&options,fp); /* 1600 */
        for (i=0;i<15;i++) /* 1601 */
            if (!load_hisc_table(hisc_tables[i],fp)) /* 1602 */
                reset_hisc_table(hisc_tables[i],"FLD",1000,0); /* 1603 */
        pack_fclose(fp); /* 1606 */
    } else
    {
        log2file(" *** failed"); /* 1609 */
        log2file("Resetting to default config"); /* 1610 */
        reset_options(&options); /* 1611 */
    }
    if (tmpHandle[0]) { /* 1614 */
        log2file("Setting last profile"); /* 1615 */
        strcpy(options.lastProfile,tmpHandle); /* 1616 */
    }
    if (!itrcheck) { /* 1619 */
        options.timesStarted++; /* 1620 */
        log2file("Game started %d times",options.timesStarted); /* 1621 */

    set_color_depth(desktop_color_depth()); /* 1624 */
    if (!options.full_screen) { /* 1627 */
        log2file("Setting windowed mode 640x480"); /* 1628 */
        if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0)<0) { /* 1629 */
            log2file(" *** failed"); /* 1633 */
            options.full_screen=-1; /* 1634 */
        } else
            window=1; /* 1630 */
    }
    if (options.full_screen) {
        log2file("Setting fullscreen mode 640x480"); /* 1638 */
        if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0)<0) { /* 1639 */
            log2file(" *** failed"); /* 1643 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1644 */
            allegro_message("Failed to set graphics mode."); /* 1645 */
            return 0; /* 1646 */
        }
        window=0; /* 1640 */
    }
    if (!screen) { /* 1651 */
        log2file("ERROR: screen was not set"); /* 1652 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1653 */
        allegro_message("For some reason, the game failed to go into\ngraphics mode. Try starting the game again.\n\nIf this problem persists,\nplease visit www.freelunchdesign.com."); /* 1654 */
        return 0; /* 1655 */
    }
    install_mouse(); /* 1659 */
    enable_hardware_cursor(); /* 1660 */
    select_mouse_cursor(2); /* 1661 */
    if (!options.full_screen) /* 1663 */
        show_mouse(screen); /* 1664 */
    log2file("Graphics mode set. (screen = %d)",screen); /* 1668 */

    {
    DATAFILE *loader;
    BITMAP *fldLogo;
    int whiteColor;
    textprintf_centre_ex(screen,font,320,220,makecol(180,180,180),-1,
                         "please wait"); /* 1671 */
    set_color_conversion(COLORCONV_NONE); /* 1674 */
    packfile_password("(c) Free Lunch Design"); /* 1675 */
    loader=load_datafile("data/loading.dat"); /* 1676 */
    log2file("Loading loader"); /* 1677 */
    if (!loader) { /* 1678 */
        log2file(" *** failed"); /* 1679 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1680 */
        allegro_message("Failed to load loader datafile."); /* 1681 */
        return 0; /* 1682 */
    }
    packfile_password(NULL); /* 1684 */
    log2file("Putting FLD Logo on screen"); /* 1687 */
    fldLogo=loader[1].dat; /* 1692 */
    select_palette(loader[0].dat); /* 1696 */
    whiteColor=makecol(255,255,255); /* 1702 */
    clear_to_color(screen,whiteColor); /* 1707 */
    draw_sprite(screen,fldLogo,320-fldLogo->w/2,200-fldLogo->h/2); /* 1711 */
    unload_datafile(loader); /* 1715 */

    }
    log2file("Setting focus modes"); /* 1721 */
    if (options.full_screen)
        set_display_switch_mode(SWITCH_BACKAMNESIA); /* 1723 */
    else
        set_display_switch_mode(SWITCH_BACKGROUND); /* 1726 */
    log2file("Setting focus callbacks"); /* 1728 */
    set_display_switch_callback(SWITCH_IN,switchedToProgram); /* 1729 */
    set_display_switch_callback(SWITCH_OUT,switchedFromProgram); /* 1730 */
    set_close_button_callback(clickedCloseButton); /* 1731 */
    srand((unsigned int)time(NULL)); /* 1733 */
    log2file("Installing timers"); /* 1736 */
    draw_progress_bar(); /* 1737 */
    install_timers(); /* 1738 */
    cycle_count=0; /* 1741 */
    log2file("Installing keyboard"); /* 1743 */
    draw_progress_bar(); /* 1744 */
    install_keyboard(); /* 1745 */
    log2file("Installing sound"); /* 1749 */
    draw_progress_bar(); /* 1750 */
    install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL); /* 1753 */
    log2file("Installing joystick/gamepad"); /* 1756 */
    draw_progress_bar(); /* 1757 */
    got_joystick=(install_joystick(JOY_TYPE_AUTODETECT)==0); /* 1758 */
    if (got_joystick) { /* 1759 */
        ctrl.use_joy=1; /* 1760 */
        log2file(" gamepad has %d buttons",joy[0].num_buttons); /* 1761 */
        if (exists("gamepad.txt")) { /* 1763 */
            int i;
            Tgamepad *gp;
            gp=get_gamepad(); /* 1765 */
            log2file(" getting values from gamepad.txt"); /* 1766 */
            set_config_file("gamepad.txt"); /* 1767 */
            gp->up=get_gamepad_value("up"); /* 1768 */
            gp->left=get_gamepad_value("left"); /* 1769 */
            gp->right=get_gamepad_value("right"); /* 1770 */
            gp->down=get_gamepad_value("down"); /* 1771 */
            for (i=0;i<32;i++) { /* 1772 */
                char buf[8];
                sprintf(buf,"b%d",i+1); /* 1774 */
                gp->b[i]=get_gamepad_value(buf); /* 1775 */
            }
        } else {
            Tgamepad *gp;
            gp=get_gamepad(); /* 1779 */
            log2file(" gamepad.txt is missing, setting defaults"); /* 1780 */
            gp->up=4; /* 1781 */
            gp->left=1; /* 1782 */
            gp->right=2; /* 1783 */
            gp->down=8; /* 1784 */
            for (i=0;i<32;i++) /* 1785 */
                gp->b[i]=16; /* 1786 */
        }
    } else
        log2file(" no gamepad or joystick found, play with keyboard only"); /* 1791 */

    log2file("Reserving memory"); /* 1795 */
    draw_progress_bar(); /* 1796 */
    swap_screen=create_bitmap(640,480); /* 1797 */
    if (!swap_screen) { /* 1798 */
        log2file(" *** failed"); /* 1799 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1800 */
        allegro_message("Failed reserve memory screen buffers."); /* 1801 */
        return 0; /* 1802 */
    }

    pwd_garble_string(init_string,50); /* 1806 */
    log2file("Loading data"); /* 1818 */
    set_color_conversion(0x00ffffff); /* 1820 */
    draw_progress_bar(); /* 1821 */
    packfile_password(init_string); /* 1822 */
    data=load_datafile_callback("data/data.dat",datafile_callback_slow); /* 1823 */
    if (!data) { /* 1824 */
        log2file(" *** failed"); /* 1825 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1826 */
        allegro_message("Failed to load datafile."); /* 1827 */
        return 0; /* 1828 */
    }
    packfile_password(NULL); /* 1830 */
    }
    log2file("Initiating player"); /* 1835 */
    draw_progress_bar(); /* 1836 */
    player_id=rand()%1000; /* 1837 */
    ply[player_id]=malloc(sizeof(*ply[player_id])); /* 1838 */
    if (!ply[player_id]) { /* 1839 */
        log2file(" *** failed"); /* 1840 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1841 */
        allegro_message("Failed to allocate memory for player."); /* 1842 */
        return 0; /* 1843 */
    }
    if (!itrcheck) { /* 1846 */
        char profiledir[1024];
        int last_cc;
        ((RGB *)data[0].dat)[0].r=((RGB *)data[0].dat)[0].g=((RGB *)data[0].dat)[0].b=0; /* 1849 */
        gameover_bmp=data[55].dat; /* 1850 */
        log2file("Checking profile directory"); /* 1855 */
        get_profiledir(profiledir,sizeof(profiledir)); /* 1858 */
        if (!file_exists(profiledir,FA_ALL,0)) { /* 1860 */
            log2file("  does not exist, trying to create"); /* 1861 */
            mkdir(profiledir); /* 1863 */
            if (!file_exists(profiledir,FA_ALL,0)) { /* 1867 */
                log2file("  *** failed!"); /* 1868 */
                set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1869 */
                allegro_message("Failed to create profile directory %s",profiledir); /* 1870 */
                return 0; /* 1871 */
            }
        }
        log2file("Checking available profiles"); /* 1876 */
        draw_progress_bar(); /* 1877 */
        rebuild_profile_list(0); /* 1878 */
        log2file("Loading profile"); /* 1880 */
        draw_progress_bar(); /* 1881 */
        log2file(" loading '%s'",options.lastProfile); /* 1882 */
        profile=load_profile(options.lastProfile); /* 1883 */
        if (!profile) { /* 1884 */
            log2file(" profile not found '%s'",options.lastProfile); /* 1885 */
            log2file(" trying to load default profile '%s'","guest"); /* 1887 */
            profile=load_profile("guest"); /* 1888 */
            if (!profile) { /* 1889 */
                log2file(" profile not found '%s'","guest"); /* 1890 */
                profile=create_profile("guest",1); /* 1892 */
                log2file(" created profile '%s'",profile->handle); /* 1893 */
            }
        }
        if (!profile) { /* 1896 */
            log2file("  *** failed!"); /* 1897 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1898 */
            allegro_message("Failed create profile."); /* 1899 */
            return 0; /* 1900 */
        }
        strcpy(options.lastProfile,profile->handle); /* 1903 */
        syncOptionsFromProfile(); /* 1904 */
        log2file("Checking available characters"); /* 1910 */
        draw_progress_bar(); /* 1911 */
        if (!check_characters()) { /* 1912 */
            log2file(" *** no characters available"); /* 1913 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1914 */
            allegro_message("No characters available.\nPlease reinstall game or add custom characters.\nRefer to readme.txt."); /* 1915 */
            return 0;
        }
        select_palette(data[0].dat); /* 1919 */
        log2file("Loading SFX"); /* 1924 */
        draw_progress_bar(); /* 1925 */
        log2file(" loading sounds"); /* 1926 */
        packfile_password(init_string); /* 1927 */
        sfx=load_datafile_callback("data/sfx15.dat",datafile_callback); /* 1928 */
        if (!sfx) { /* 1929 */
            log2file(" could not load data/sfx15.dat"); /* 1930 */
            strcpy(sfx_file,"no sound"); /* 1931 */
            log2file(" no sounds loaded"); /* 1932 */
        }
        else {
            strcpy(sfx_file,"sfx15.dat"); /* 1935 */
            log2file(" sfx15.dat loaded"); /* 1936 */
        }
        packfile_password(NULL); /* 1938 */
        if (sfx) { /* 1940 */
            log2file("Getting sounds from data file"); /* 1941 */
            draw_progress_bar(); /* 1942 */
            combo_sound[0]=getSampleFromOggDatafile(sfx,8); /* 1943 */
            combo_sound[1]=getSampleFromOggDatafile(sfx,18); /* 1944 */
            combo_sound[2]=getSampleFromOggDatafile(sfx,9); /* 1945 */
            combo_sound[3]=getSampleFromOggDatafile(sfx,17); /* 1946 */
            combo_sound[4]=getSampleFromOggDatafile(sfx,21); /* 1947 */
            combo_sound[5]=getSampleFromOggDatafile(sfx,1); /* 1948 */
            combo_sound[6]=getSampleFromOggDatafile(sfx,5); /* 1949 */
            combo_sound[7]=getSampleFromOggDatafile(sfx,6); /* 1950 */
            combo_sound[8]=getSampleFromOggDatafile(sfx,15); /* 1951 */
            combo_sound[9]=getSampleFromOggDatafile(sfx,20); /* 1952 */
            bg_beat=getSampleFromOggDatafile(sfx,2); /* 1953 */
            bg_menu=getSampleFromOggDatafile(sfx,3); /* 1954 */
            speaker[0]=getSampleFromOggDatafile(sfx,10); /* 1955 */
            speaker[1]=getSampleFromOggDatafile(sfx,7); /* 1956 */
            speaker[2]=getSampleFromOggDatafile(sfx,19); /* 1957 */
            sounds[2]=getSampleFromOggDatafile(sfx,0); /* 1958 */
            sounds[4]=getSampleFromOggDatafile(sfx,13); /* 1959 */
            sounds[6]=getSampleFromOggDatafile(sfx,14); /* 1960 */
            sounds[7]=getSampleFromOggDatafile(sfx,4); /* 1961 */
            sounds[8]=getSampleFromOggDatafile(sfx,16); /* 1962 */
            menu_sounds[0]=getSampleFromOggDatafile(sfx,11); /* 1963 */
            menu_sounds[1]=getSampleFromOggDatafile(sfx,12); /* 1964 */
            jump_sound[0]=NULL; /* 1965 */
            jump_sound[1]=NULL; /* 1966 */
            jump_sound[2]=NULL; /* 1967 */
            sounds[0]=NULL; /* 1968 */
            sounds[1]=NULL; /* 1969 */
            sounds[3]=NULL; /* 1970 */
            sounds[5]=NULL; /* 1971 */
            log2file("Releasing ogg datafile."); /* 1972 */
            unload_datafile(sfx); /* 1973 */
            sfx=NULL; /* 1974 */
        }
        log2file("Setting menu values"); /* 1978 */
        snd_volume_slider.value=options.snd_volume; /* 1979 */
        msc_volume_slider.value=options.msc_volume; /* 1980 */
        eyecandy_selection.value=options.flash; /* 1981 */
        gravity_selection.value=options.gravity; /* 1982 */
        floor_size_selection.value=options.floor_size; /* 1983 */
        scroll_speed_selection.value=options.start_speed; /* 1984 */
        floors.max=profile->best_floor>999 ? 9 : profile->best_floor/100; /* 1986 */
        floors.value=profile->start_floor>floors.max ? floors.max : profile->start_floor; /* 1987 */
        log2file("Cleaning up"); /* 2063 */
        draw_progress_bar(); /* 2064 */
        log2file("Welcome to Icy Tower"); /* 2069 */
        draw_progress_bar(); /* 2070 */
        last_cc=0; /* 2072 */
        while (!keypressed() && cycle_count<=149) { /* 2072 */
            if (!(cycle_count%10) && cycle_count!=last_cc) { /* 2073 */
                last_cc=cycle_count; /* 2074 */
                draw_progress_bar(); /* 2075 */
                last_cc=cycle_count; /* 2076 */
            }
            rest(2); /* 2078 */
        }
        seed=rand()%2367; /* 2082 */
        fadeOut(16); /* 2083 */
        clear_bitmap(screen); /* 2084 */
        vsync(); /* 2085 */
        clear_keybuf(); /* 2086 */
    }
    init_ok=1; /* 2090 */
    return -1;
}