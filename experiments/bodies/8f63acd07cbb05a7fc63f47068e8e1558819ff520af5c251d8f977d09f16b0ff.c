{
    int fh;
    int fg;
    double view_percentage;
    double view_offset;
    int i;
    int profile_index;
    int row_y;
    char *profile_name;

    fh=text_height(font);
    fg=makecol(25,25,25);
    view_percentage=(double)max_posts/numProfiles;
    if (view_percentage>1.0)
        view_percentage=1.0;

    draw_sprite(bmp,data[86].dat,x-15,y-15);
    set_trans_blender(0,0,0,150);
    drawing_mode(5,0,0,0);
    rect(bmp,x+5,y+30,x+265,y+329,fg);
    rect(bmp,x+255,y+30,x+265,y+329,fg);
    view_offset=(1.0-view_percentage)*328/(numProfiles-max_posts)*offset;
    rectfill(bmp,x+257,y+32+view_offset,x+263,y+view_percentage*328+view_offset,fg);
    solid_mode();
    textout_ex(bmp,data[51].dat,"SELECT PROFILE",x+10,y-12,-1,-1);
    draw_sprite(bmp,data[73].dat,x+270,y+24);
    set_clip_rect(bmp,x+6,0,x+290,((int *)bmp)[1]-1);

    row_y=y+fh+31;
    for (i=1,profile_index=offset;
         i<=max_posts && profile_index<numProfiles;
         i++,profile_index++,row_y+=fh) {
        profile_name=profiles[profile_index].handle;
        if (profile_index==selection) {
            drawing_mode(5,0,0,0);
            set_trans_blender(0,0,0,50);
            rectfill(bmp,x+7,row_y-47,x+263,row_y+fh,fg);
            solid_mode();
        }
        textprintf_ex(bmp,font,x+8,row_y,fg,-1,"%c %c %s %s",
                      profile_index==selection ? '>' : ' ',
                      profile_index<1 ? '~' : '{',profile_name,
                      stricmp(profile_name,current_profile->handle)==0 ? "(current)" : "");
    }
    set_clip_rect(bmp,0,0,((int *)bmp)[0]-1,((int *)bmp)[1]-1);
}