{
    int fh;
    int fg;
    double view_percentage;
    double view_offset;
    int width;
    int height;
    int scrollHeight;
    int i;

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

    i=1;
    if (max_posts>0 && offset<numProfiles) {
        char *current_handle;
        int row_bottom;
        current_handle=current_profile->handle;
        y+=32;
        row_bottom=y+fh-1;
        do {
            int profile_index;
            char icon;
            int selected;
            profile_index=offset-1+i;
            icon=profile_index<1 ? '~' : '{';
            if (profile_index==selection) {
                drawing_mode(5,0,0,0);
                set_trans_blender(0,0,0,50);
                rectfill(bmp,x+7,y,x+253,row_bottom,fg);
                solid_mode();
                selected=1;
            } else {
                selected=0;
            }
            textprintf_ex(bmp,font,x+8,y,fg,-1,"%c %c %s %s",
                          selected ? '>' : ' ',icon,
                          profiles[profile_index].handle,
                          stricmp(profiles[profile_index].handle,current_handle)==0 ? "(current)" : "");
            if (i>=max_posts)
                break;
            ++i;
            y+=fh;
            row_bottom+=fh;
        } while (offset-1+i<numProfiles);
    }
    set_clip_rect(bmp,0,0,((int *)bmp)[0]-1,((int *)bmp)[1]-1);
}