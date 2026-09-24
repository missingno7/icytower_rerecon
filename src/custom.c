/* Complete custom.c candidate reconstructed from original DWARF/disassembly.
 * Code and data equality are tracked by the experiment, not assumed here. */
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "custom.h"
#include "game_services.h"

RGB black;
RGB pink = {255, 0, 255, 0};

void custom_alert(char *txt1, char *txt2)
{
    gui_fg_color = makecol(255,255,255);
    gui_bg_color = makecol(0,0,0);
    alert("CUSTOM CHARACTER", txt1, txt2, "OK", NULL, 0, 0);
}

char *get_string_data(char *key, char *string)
{
    int i;
    int len = strlen(key);
    if (strncmp(key, string, len)) return NULL;
    i = len;
    while (string[i] == ' ' || string[i] == '\t') i++;
    return string + i;
}

BITMAP *load_character_bmp(const char *name, int *uses_datafile, RGB *pal)
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
    if (!fp) { log2file("Could not open %s", filename); return NULL; }
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
}

int init_custom(Tcustom *c,const char *name,int uses_datafile)
{
    int i;
    strcpy(c->name,name);
    for (i=0;i<15;i++) c->frame[i]=NULL;
    c->jump_sound[0]=NULL; c->jump_sound[1]=NULL; c->jump_sound[2]=NULL;
    c->bg_music=NULL; c->bg_midi=NULL;
    c->falling=NULL; c->edge=NULL; c->yo=NULL; c->wazup=NULL;
    c->uses_datafile=uses_datafile;
    c->df=NULL; c->ok=0;
    return 1;
}

int load_frames(Tcustom *c)
{
    FILE *fp;
    char buf[1024];
    char *ret;
    char data[1024];
    char filename[1024];
    int ok=0;
    unsigned int i;
    int depth;
    get_character_dir(filename,sizeof(filename),c->name);
    strcat(filename,c->name);
    strcat(filename,".txt");
    fp=fopen(filename,"rt");
    if (!fp) return 0;
    if (!c->uses_datafile) {
        ret=fgets(buf,sizeof(buf),fp);
    while (ret && !ok) {
            ret=get_string_data("[frames]",buf);
            if (ret) {
                strcpy(data,ret);
                for (i=0;i<strlen(data);i++)
                    if (data[i]==' ' || data[i]=='\t' || data[i]=='\n' || data[i]=='\r') data[i]=0;
                ok=1;
            }
            ret=fgets(buf,sizeof(buf),fp);
    }
        fclose(fp);
        if (!ok) return 0;
        get_character_dir(filename,sizeof(filename),c->name);
        strcat(filename,data);
        {
            BITMAP *bmp;
            int color_conversion=get_color_conversion();
            set_color_conversion(COLORCONV_NONE);
            bmp=load_bitmap(filename,c->pal);
            depth=bitmap_color_depth(bmp);
            set_color_conversion(color_conversion);
            destroy_bitmap(bmp);
            if (depth==8) {
                set_color_conversion(COLORCONV_NONE);
                bmp=load_bitmap(filename,c->pal);
                set_color_conversion(color_conversion);
                c->pal[0]=pink;
            } else bmp=load_bitmap(filename,c->pal);
            if (!bmp) {
                char buf[256];
                sprintf(buf,"failed loading: %s",filename);
                custom_alert(buf,"character will be skipped");
                return 0;
            }
            {
                int x=0,y,w,h;
                unsigned int frame_color=getpixel(bmp,0,0);
                for (i=0;i<15;i++) {
                    PALETTE myPal;
                    BITMAP *tmp_bmp;
                    y=0; w=0; h=0;
                    while (getpixel(bmp,x,y)==frame_color && x<bmp->w) {
                        y++;
                        if (y==bmp->h) { x++; y=0; }
                    }
                    if (x>=bmp->w) {
                        char buf1[256],buf2[256];
                        sprintf(buf1,"not enough frames in %s",filename);
                        sprintf(buf2,"or unaligned frames (ERR: %d)",i);
                        custom_alert(buf1,buf2);
                        destroy_bitmap(bmp);
                        return 0;
                    }
                    while (getpixel(bmp,x+w,y)!=frame_color && x+w<bmp->w-1) w++;
                    while (getpixel(bmp,x,y+h)!=frame_color && y+h<bmp->h-1) h++;
                    if (!w || !h) {
                        char buf1[256],buf2[256];
                        sprintf(buf1,"failed to load %s",filename);
                        strcpy(buf2,"palette might be messed up or frames are too few or unaligned");
                        clear_bitmap(screen);
                        select_palette(desktop_palette);
                        custom_alert(buf1,buf2);
                        clear_bitmap(screen);
                        destroy_bitmap(bmp);
                        return 0;
                    }
                        get_palette(myPal);
                        select_palette(c->pal);
                        tmp_bmp=create_bitmap_ex(bitmap_color_depth(bmp),w,h);
                        blit(bmp,tmp_bmp,x,y,0,0,w,h);
                        c->frame[i]=create_bitmap(w,h);
                        clear_to_color(c->frame[i],makecol(255,0,255));
                        draw_sprite(c->frame[i],tmp_bmp,0,0);
                        destroy_bitmap(tmp_bmp);
                        select_palette(myPal);
                    x+=w;
                }
            }
            destroy_bitmap(bmp);
            return 1;
        }
    } else {
        fp=fopen(filename,"rt");
        if (!fp) return 0;
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
        if (!ok) return 0;
        get_character_dir(filename,sizeof(filename),c->name);
        strcat(filename,data);
        {
            int color_conversion=get_color_conversion();
            c->df=load_datafile(filename);
            set_color_conversion(color_conversion);
        }
        if (!c->df) {
            char buf[256];
            sprintf(buf,"failed loading: %s",filename);
            custom_alert(buf,"character will be skipped");
            return 0;
        }
        for (i=0;i<256;i++) c->pal[i]=((RGB *)c->df[0].dat)[i];
        c->pal[0]=black;
        for (i=0;i<15;i++) {
            BITMAP *orgFrame=c->df[i+1].dat;
            c->frame[i]=create_bitmap(orgFrame->w,orgFrame->h);
            clear_to_color(c->frame[i],makecol(255,0,255));
            draw_sprite(c->frame[i],orgFrame,0,0);
        }
        return 1;
    }
}

void clear_trailing_whitespace(char *data)
{
    unsigned int i;
    for (i=0;i<strlen(data);i++)
        if (data[i]==' ' || data[i]=='\t' || data[i]=='\n' || data[i]=='\r') data[i]=0;
}

SAMPLE *loadCustomSoundDF(DATAFILE *df,int id)
{
    if (df[id].type==DAT_SAMPLE) {
        SAMPLE *smpl=df[id].dat;
        df[id].dat=NULL;
        return smpl;
    }
    return getSampleFromOggDatafile(df,id);
}

SAMPLE *loadCustomSoundFILE(char *tag,char *filename)
{
    SAMPLE *snd;
    log2file("   %-12s loading sound %s",tag,filename);
    if (!stricmp(get_extension(filename),"ogg")) return logg_load(filename);
    snd=load_sample(filename);
    if (!snd) log2file("   %-12s could not open %s",tag,filename);
    return snd;
}

int load_sounds(Tcustom *c)
{
    FILE *fp;
    char buf[1024];
    char *ret;
    char data[1024];
    char filename[1024];
    int ok=0;
    get_character_dir(filename,sizeof(filename),c->name);
    strcat(filename,c->name);
    strcat(filename,".txt");
    fp=fopen(filename,"rt");
    if (!fp) return 0;
    if (c->uses_datafile) {
        int df_length=0;
        if (!c->df) return 0;
        while (c->df[df_length].type!=DAT_END) df_length++;
        df_length-=2;
        if (c->df[0].type!=DAT_SAMPLE) log2file("  loading from datafile");
        if (df_length>=16) {
            log2file("    loading %s","jump lo");
            c->jump_sound[0]=loadCustomSoundDF(c->df,16);
        }
        if (df_length>=17) {
            log2file("    loading %s","jump med");
            c->jump_sound[1]=loadCustomSoundDF(c->df,17);
        }
        if (df_length>=18) {
            log2file("    loading %s","jump hi");
            c->jump_sound[2]=loadCustomSoundDF(c->df,18);
        }
        if (df_length>=19) {
            log2file("    loading %s","yo");
            c->yo=loadCustomSoundDF(c->df,19);
        }
        if (df_length>=20) {
            log2file("    loading %s","wazup");
            c->wazup=loadCustomSoundDF(c->df,20);
        }
        if (df_length>=21) {
            log2file("    loading %s","falling");
            c->falling=loadCustomSoundDF(c->df,21);
        }
        if (df_length>=22) {
            log2file("    loading %s","edge");
            c->edge=loadCustomSoundDF(c->df,22);
        }
        if (df_length>=23) {
            log2file("    loading %s","bg music");
            c->bg_music=loadCustomSoundDF(c->df,23);
        }
        return 1;
    }
    log2file("  loading from txtfile");
    ret=fgets(buf,sizeof(buf),fp);
    while (ret) {
        ret=get_string_data("[greeting]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->yo=loadCustomSoundFILE("[greeting]",filename);
        }
        ret=get_string_data("[edge]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->edge=loadCustomSoundFILE("[edge]",filename);
        }
        ret=get_string_data("[pause]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->wazup=loadCustomSoundFILE("[pause]",filename);
        }
        ret=get_string_data("[death]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->falling=loadCustomSoundFILE("[death]",filename);
        }
        ret=get_string_data("[jumplo]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->jump_sound[0]=loadCustomSoundFILE("[jumplo]",filename);
        }
        ret=get_string_data("[jumpmed]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->jump_sound[1]=loadCustomSoundFILE("[jumpmed]",filename);
        }
        ret=get_string_data("[jumphi]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            c->jump_sound[2]=loadCustomSoundFILE("[jumphi]",filename);
        }
        ret=get_string_data("[bgmusic]",buf);
        if (ret) {
            strcpy(data,ret);
            clear_trailing_whitespace(data);
            get_character_dir(filename,sizeof(filename),c->name);
            strcat(filename,data);
            if (!stricmp(get_extension(filename),"mid")) c->bg_midi=load_midi(filename);
            if (!stricmp(get_extension(filename),"midi")) c->bg_midi=load_midi(filename);
            else c->bg_music=loadCustomSoundFILE("[bgmusic]",filename);
        }
        ret=fgets(buf,sizeof(buf),fp);
    }
    fclose(fp);
    return 1;
}

int destroy_custom_data(Tcustom *c)
{
    int i;
    if (c->uses_datafile) {
        log2file("    destroying %s","datafile");
        unload_datafile(c->df);
    } else {
        log2file("    destroying %s","frames");
        for (i=0;i<15;i++) destroy_bitmap(c->frame[i]);
    }
    if (c->jump_sound[0]) {
        destroy_sample(c->jump_sound[0]);
        log2file("    destroying %s","jump lo");
    }
    if (c->jump_sound[1]) {
        destroy_sample(c->jump_sound[1]);
        log2file("    destroying %s","jump med");
    }
    if (c->jump_sound[2]) {
        destroy_sample(c->jump_sound[2]);
        log2file("    destroying %s","jump hi");
    }
    if (c->falling) {
        destroy_sample(c->falling);
        log2file("    destroying %s","fall");
    }
    if (c->edge) {
        destroy_sample(c->edge);
        log2file("    destroying %s","edge");
    }
    if (c->yo) {
        destroy_sample(c->yo);
        log2file("    destroying %s","yo");
    }
    if (c->wazup) {
        destroy_sample(c->wazup);
        log2file("    destroying %s","wazup");
    }
    if (c->bg_music) {
        destroy_sample(c->bg_music);
        log2file("    destroying %s","bg music");
    }
    if (c->bg_midi) {
        destroy_midi(c->bg_midi);
        log2file("    destroying %s","bg midi");
    }
    init_custom(c,"none",0);
    return 0;
}
