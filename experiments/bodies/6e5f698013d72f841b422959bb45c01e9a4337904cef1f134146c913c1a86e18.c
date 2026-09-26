{
    BITMAP *bg;
    int y;
    int ok;
    char new_name[32];

    bg=create_bitmap(SCREEN_W,SCREEN_H);
    blit(screen,bg,0,0,0,0,SCREEN_W,SCREEN_H);
    memset(new_name,0,sizeof(new_name));
    for (;;) {
        int res;
        char buf[129];

        checkMenuFocus();
        blit(bg,swap_screen,0,0,0,0,SCREEN_W,SCREEN_H);
        set_trans_blender(0,0,0,158);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        draw_sprite(swap_screen,data[87].dat,100,120);
        textout_ex(swap_screen,data[51].dat,"Welcome to Icy Tower",130,127,-1,-1);
        textout_ex(swap_screen,data[54].dat,"Yo, wazup? In Icy Tower, all your highscores",130,160,0,-1);
        textout_ex(swap_screen,data[54].dat,"and progress will be stored in a personal profile.",130,175,0,-1);
        textout_ex(swap_screen,data[54].dat,"AWESOME!",130,190,0,-1);
        textout_ex(swap_screen,data[54].dat,"Please enter a name for your profile:",130,220,0,-1);
        textout_right_ex(swap_screen,data[54].dat,"...and press enter.",430,260,0,-1);
        rectfill(swap_screen,129,240,430,258,makecol(255,255,255));
        rect(swap_screen,129,240,430,258,makecol(80,80,80));
        blit_to_screen(swap_screen);
        res=get_string(swap_screen,new_name,300,32,data[54].dat,130,240,makecol(0,0,0),-1);
        if (res>=-1) {
            if (res==-1) {
            my_alert("You can create a profile later in the OPTIONS menu.","Oh Well...",0,1);
            profile=load_profile("guest");
            if (!profile)
                profile=create_profile("guest",1);
            syncOptionsFromProfile();
            break;
            }
            if (!new_name[0])
                continue;
            replaceBadCharacters(new_name,'_');
            profile=create_profile(new_name,0);
            if (!profile) {
                my_alert("That profile name is taken.","Ooops!",0,1);
                continue;
            }
            sprintf(buf,"Welcome %s!",profile->handle);
            my_alert(buf,"Your profile has been created!",0,1);
            break;
        }
    }
    destroy_bitmap(bg);
    strcpy(options.lastProfile,profile->handle);
    rebuild_profile_list(0);
}