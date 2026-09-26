{
    int fh;
    int fg;
    double view_percentage;
    double view_offset;
    int width;
    int height;
    int scrollHeight;
    int i;
    int profile_index;
    int row_y;
    char *current_handle;

    fh=text_height(font);
    fg=makecol(25,25,25);
    width=260;
    height=300;
    scrollHeight=328;
    view_percentage=(double)max_posts/numProfiles;
    if (view_percentage>1.0)
        view_percentage=1.0;

    draw_sprite(bmp,data[86].dat,x-15,y-15);
    set_trans_blender(0,0,0,150);
    drawing_mode(5,0,0,0);
    rect(bmp,x+5,y+30,x+5+width,y+29+height,fg);
    rect(bmp,x+width-5,y+30,x+width+5,y+29+height,fg);
    view_offset=(1.0-view_percentage)*scrollHeight/(numProfiles-max_posts)*offset;
    rectfill(bmp,x+width-3,y+32+view_offset,x+width+3,y+view_percentage*scrollHeight+view_offset,fg);
    solid_mode();
    textout_ex(bmp,data[51].dat,"SELECT PROFILE",x+10,y-12,-1,-1);
    draw_sprite(bmp,data[73].dat,x+270,y+24);
    set_clip_rect(bmp,x+6,0,x+290,((int *)bmp)[1]-1);

    current_handle=current_profile->handle;
    row_y=y+fh+31;
    for (i=1,profile_index=offset-1;
         i<=max_posts && profile_index+i<numProfiles;
         i++,row_y+=fh) {
        char icon;
        int selected;
        icon=profile_index+i<1 ? '~' : '{';
        if (profile_index+i==selection) {
            drawing_mode(5,0,0,0);
            set_trans_blender(0,0,0,50);
            rectfill(bmp,x+7,row_y-47,x+263,row_y+fh,fg);
            solid_mode();
            selected=1;
        } else {
            selected=0;
        }
        textprintf_ex(bmp,font,x+8,row_y,fg,-1,"%c %c %s %s",
                      selected ? '>' : ' ',icon,profiles[profile_index+i].handle,
                      stricmp(profiles[profile_index+i].handle,current_handle)==0 ? "(current)" : "");
    }
    set_clip_rect(bmp,0,0,((int *)bmp)[0]-1,((int *)bmp)[1]-1);
}