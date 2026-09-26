{
    int i;

    BITMAP *bg;
    bg=create_bitmap(SCREEN_W,SCREEN_H);
    blit(screen,bg,0,0,0,0,SCREEN_W,SCREEN_H);
    clear_keybuf();
    while (is_any(get_controls()) || key[KEY_K])
        poll_control(get_controls(),0);
    clear_keybuf();

    int pageY;
    int targetY;
    int dark;
    int targetDark;
    int listHeight;
    listHeight=0;
    for (i=0;i<15;i++) {
        if (tables[i]->posts[0].value) {
            listHeight=draw_table(0,0,listHeight,names[i],tables[i]);
            listHeight+=18;
        }
    }

    int th;
    int mh;
    int bh;
    th=((BITMAP *)data[66].dat)->h;
    mh=((BITMAP *)data[65].dat)->h;
    bh=((BITMAP *)data[64].dat)->h;
    int lh;
    lh=listHeight/bh;
    int bmpHeight;
    bmpHeight=lh>2 ? lh-1 : 2;
    BITMAP *bmp;
    bmp=create_bitmap(((BITMAP *)data[66].dat)->w,bh+th+bmpHeight*mh);
    clear_to_color(bmp,makecol(255,0,255));
    draw_sprite(bmp,(BITMAP *)data[66].dat,0,0);
    for (i=0;i<bmpHeight;i++)
        draw_sprite(bmp,(BITMAP *)data[65].dat,0,th+i*mh);
    draw_sprite(bmp,(BITMAP *)data[64].dat,0,bmp->h-bh);

    int yPos;
    yPos=80;
    for (i=0;i<15;i++) {
        if (tables[i]->posts[0].value) {
            yPos=draw_table(bmp,40,yPos,names[i],tables[i]);
            yPos+=18;
        }
    }

    pageY=500;
    targetY=0;
    dark=0;
    targetDark=158;
    int done;
    int canDone;
    done=0;
    canDone=0;
    while (!closeButtonClicked && !done) {
        cycle_count=0;
        checkMenuFocus();
        blit(bg,swap_screen,0,0,0,0,SCREEN_W,SCREEN_H);
        set_trans_blender(0,0,0,dark);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        draw_sprite(swap_screen,bmp,160,pageY);
        dark=(int)((targetDark-dark)*0.2+dark);
        draw_sprite(swap_screen,data[9].dat,626-dark,380);
        draw_sprite(swap_screen,data[6].dat,626-dark,40);
        blit_to_screen(swap_screen);
        poll_control(get_controls(),0);
        if (is_up(get_controls())) {
            if (targetY < -15)
                targetY+=16;
            else
                targetY=0;
        }
        if (is_down(get_controls()) && targetY>485-bmp->h)
            targetY-=16;
        done=is_fire(get_controls());
        if ((key[KEY_F1] || key[KEY_ENTER] || key[KEY_K]) && canDone)
            done=1;
        if (!key[KEY_F1] && !key[KEY_ENTER] && !key[KEY_K])
            canDone=1;
        while (!cycle_count)
            rest(2);
        pageY=(int)((targetY-pageY)*0.2+pageY);
    }

    targetY=500;
    targetDark=0;
    while (pageY<=480) {
        cycle_count=0;
        checkMenuFocus();


        draw_sprite(swap_screen,bg,0,dark);
        set_trans_blender(0,0,0,dark);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        draw_sprite(swap_screen,bmp,160,pageY);
        pageY=(int)((targetY-pageY)*0.2+pageY);
        dark=(int)((targetDark-dark)*0.2+dark);
        draw_sprite(swap_screen,data[9].dat,626-dark,380);
        draw_sprite(swap_screen,data[6].dat,626-dark,40);
        blit_to_screen(swap_screen);
        keypressed();
        while (!cycle_count)
            rest(2);
    }
    clear_keybuf();
    destroy_bitmap(bmp);
    destroy_bitmap(bg);
}