{
    int fh;
    int i;
    int fg;
    double view_percentage;
    int width;
    int height;
    int scrollHeight;
    double view_offset;
    int row_y;

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
    for (i=1; i<=max_posts && i+offset-1<numProfiles; i++,row_y+=fh) {
        char icon;
        int selected;
        icon=i+offset-1<1 ? '~' : '{';
        selected=i+offset-1==selection;
        if (selected) {
            drawing_mode(5,0,0,0);
            set_trans_blender(0,0,0,50);
            rectfill(bmp,x+7,row_y-47,x+263,row_y+fh,fg);
            solid_mode();
        }
        textprintf_ex(bmp,font,x+8,row_y,fg,-1,"%c %c %s %s",
                      selected ? '>' : ' ',icon,
                      profiles[i+offset-1].handle,
                      stricmp(profiles[i+offset-1].handle,current_profile->handle)==0 ? "(current)" : "");
    }
    set_clip_rect(bmp,0,0,((int *)bmp)[0]-1,((int *)bmp)[1]-1);
}