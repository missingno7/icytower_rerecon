{
    FILE *fp;
    char buf[1024];
    char *ret;
    char data[1024];
    char filename[1024];
    int ok = 0;
    unsigned int i;
    int depth;
    get_character_dir(filename, sizeof(filename), name);
    strcat(filename, name);
    strcat(filename, ".txt");
    fp = fopen(filename, "rt");
    if (fp) {
    ret=fgets(buf,sizeof(buf),fp);
    while (ret && !ok) {
        ret = get_string_data("[frames]", buf);
        if (ret) {
            strcpy(data, ret);
            for (i=0; i<strlen(data); i++)
                if (data[i]==' ' || data[i]=='\t' || data[i]=='\n' || data[i]=='\r') data[i]=0;
            ok=1;
        }
        ret=fgets(buf,sizeof(buf),fp);
    }
    fclose(fp);
    if (ok) {
        int color_conversion = get_color_conversion();
        BITMAP *bmp;
        *uses_datafile = 0;
        get_character_dir(filename, sizeof(filename), name);
        strcat(filename, data);
        set_color_conversion(COLORCONV_NONE);
        bmp=load_bitmap(filename, pal);
        if (!bmp) {
            char buf[256];
            set_color_conversion(color_conversion);
            sprintf(buf, "failed depth loading: %s", filename);
            custom_alert(buf, "character will be skipped");
            return bmp;
        }
        depth=bitmap_color_depth(bmp);
        destroy_bitmap(bmp);
        set_color_conversion(color_conversion);
        if (depth==8) {
            set_color_conversion(COLORCONV_NONE);
            bmp=load_bitmap(filename,pal);
            set_color_conversion(color_conversion);
            pal[0]=pink;
        } else bmp=load_bitmap(filename,pal);
        if (!bmp) {
            char buf[256];
            sprintf(buf,"failed loading: %s",filename);
            custom_alert(buf,"character will be skipped");
            return bmp;
        }
        else {
            int x=0,y=0,w=0,h=0;
            BITMAP *char_bmp;
            int frame_color=getpixel(bmp,0,0);
            BITMAP *tmp_bmp;
            while (getpixel(bmp,x,y)==frame_color && x<bmp->w) {
                y++;
                if (y==bmp->h) { x++; y=0; }
            }
            if (x>=bmp->w) {
                char buf[256];
                sprintf(buf,"failed loading: %s",filename);
                custom_alert(buf,"frame background color must be index 255 (no such pixel color found)");
            }
            while (getpixel(bmp,x+w,y)!=frame_color && x+w<bmp->w) w++;
            while (getpixel(bmp,x,y+h)!=frame_color && y+h<bmp->h) h++;
            select_palette(pal);
            tmp_bmp=create_bitmap_ex(bitmap_color_depth(bmp),w,h);
            blit(bmp,tmp_bmp,x,y,0,0,w,h);
            char_bmp=create_bitmap(w,h);
            clear_to_color(char_bmp,makecol(255,0,255));
            draw_sprite(char_bmp,tmp_bmp,0,0);
            destroy_bitmap(tmp_bmp);
            destroy_bitmap(bmp);
            return char_bmp;
        }
    }
    fp=fopen(filename,"rt");
    if (!fp) return NULL;
    ok=0;
    ret=fgets(buf,sizeof(buf),fp);
    while (ret && !ok) {
        ret=get_string_data("[datafile]",buf);
        if (ret) {
            strcpy(data,ret);
            for (i=0;i<strlen(data);i++)
                if (data[i]==' ' || data[i]=='\t' || data[i]=='\n' || data[i]=='\r') data[i]=0;
            ok=1;
        }
        ret=fgets(buf,sizeof(buf),fp);
    }
    fclose(fp);
    if (ok) {
        DATAFILE *df;
        BITMAP *char_bmp;
        int color_conversion;
        *uses_datafile=1;
        get_character_dir(filename,sizeof(filename),name);
        strcat(filename,data);
        color_conversion=get_color_conversion();
        df=load_datafile(filename);
        set_color_conversion(color_conversion);
        if (!df) {
            char buf[256];
            sprintf(buf,"failed loading: %s",filename);
            custom_alert(buf,"character will be skipped");
            return NULL;
        }
        {
            int w=((BITMAP *)df[1].dat)->w;
            int h=((BITMAP *)df[1].dat)->h;
            for (i=0;i<256;i++) pal[i]=((RGB *)df[0].dat)[i];
            pal[0]=pink;
            char_bmp=create_bitmap_ex(bitmap_color_depth((BITMAP *)df[1].dat),w,h);
            clear_bitmap(char_bmp);
            blit((BITMAP *)df[1].dat,char_bmp,0,0,0,0,w,h);
            unload_datafile(df);
            return char_bmp;
        }
    }
    return NULL;
} else {
        log2file("Could not open %s", filename);
        return NULL;
    }
}