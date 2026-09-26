{
    double vol;
    double vol_step;
    int gc;
    BITMAP *logoBMP;

    vol=options.msc_volume;
    vol_step=vol/150.0f;
    clear_bitmap(screen);
    logoBMP=data[125].dat;
    blit(data[126].dat,screen,0,0,0,0,640,480);
    draw_sprite(screen,logoBMP,320-logoBMP->w/2,10);
    textout_centre_ex(screen,data[50].dat,"Thanks for playing!",320,280,-1,-1);
    textout_centre_ex(screen,data[52].dat,"DESIGN & CODING: Johan Peitz",320,360,-1,-1);
    textout_centre_ex(screen,data[52].dat,"GRAPHICS: Emanuel Garnheim",320,390,-1,-1);
    fadeIn(screen,16);
    closeButtonClicked=0;
    cycle_count=0;
    while (!key[KEY_ESC] && cycle_count<=149) {
        gc=cycle_count;
        checkMenuFocus();
        if (bg_menu)
            adjust_sample(bg_menu,(int)vol,128,1000,1);
        while (gc==cycle_count)
            rest(2);
        if (closeButtonClicked)
            break;
        vol-=vol_step;
    }
    fadeOut(16);
}