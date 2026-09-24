#include "recovered/Thisc_table.h"
#include <allegro.h>
#include "control.h"
#include "timer.h"

/* Historical CU: F:\projects\icytower\trunk\source\hisc.c
 * Ownership: GAME; current status: src/recovery.json.
 * Historical function: view_scores @ 0x00404c38, 2552 bytes
 */

#include "recovered/Thisc.h"
typedef Thisc Thisc_post;

#include "recovered/Thisc_table.h"

extern void free(void *ptr);
extern void *malloc(unsigned int size);
extern char *strcpy(char *dst,const char *src);
/* DWARF declares the shared list as DATAFILE *.  The original's 0x420,
 * 0x410, and 0x400 offsets identify entries 66, 65, and 64. */
extern DATAFILE *data;
extern BITMAP *swap_screen;
extern int closeButtonClicked;
extern Tcontrol *get_controls(void);
extern void checkMenuFocus(void);
extern void blit_to_screen(BITMAP *bmp);

Thisc_table *make_hisc_table(char *name)
{
    Thisc_table *table;

    table=malloc(sizeof(Thisc_table));
    if (table) {
        table->posts=malloc(180);
        if (table->posts) {
            strcpy(table->name,name);
            return table;
        }
    }
    return 0;
}

void destroy_hisc_table(Thisc_table *table)
{
    free(table->posts);
    free(table);
}

int qualify_hisc_table(Thisc_table *table,int value)
{
    int i;

    if (value)
        for (i=0;i<5;i++)
            if (table->posts[i].value<value) return i+1;
    return 0;
}

void sort_hisc_table(Thisc_table *table)
{
    int i,j;
    Thisc_post post;

    for (i=1;i<5;i++) {
        post=table->posts[i];
        for (j=i;j>0 && table->posts[j-1].value<post.value;j--)
            table->posts[j]=table->posts[j-1];
        table->posts[j]=post;
    }
}

void enter_hisc_table(Thisc_table *table,int value,char *name)
{
    unsigned int lo=10000000;
    int loID=-1;
    int i;

    for (i=0;i<5;i++) {
        if (table->posts[i].value<lo) {
            loID=i;
            lo=table->posts[i].value;
        }
    }
    if (loID!=-1) {
        table->posts[loID].value=value;
        strcpy(table->posts[loID].name,name);
    }
}

void reset_hisc_table(Thisc_table *table,char *name,int hi,int lo)
{
    int i;
    int d;

    for (i=0;i<5;i++) {
        strcpy(table->posts[i].name,name);
        table->posts[i].value=0;
    }
}

int generate_checksum(Thisc_post *entry)
{
    int i;
    char *s;

    i=entry->value;
    s=entry->name;
    while (*s) {
        i=i*140+*s;
        s++;
    }
    return i;
}

/* Source candidate recovered from the viewer's bitmap construction, control
 * flow, and rendering call surface.  Its animation constants and asset order
 * are oracle-derived; the complete 2552-byte function still needs matching
 * source structure for exact code generation. */
int load_hisc_table(Thisc_table *table,PACKFILE *fp)
{
    int i;
    int ok=1;

    for (i=0;i<5;i++) {
        int c_disk,c_real;
        pack_fread(&table->posts[i],sizeof(Thisc_post),fp);
        pack_fread(&c_disk,sizeof(int),fp);
        c_real=generate_checksum(&table->posts[i]);
        if (c_disk!=c_real) ok=0;
    }
    return ok;
}

void save_hisc_table(Thisc_table *table,PACKFILE *fp)
{
    int i;

    for (i=0;i<5;i++) {
        int checksum;
        pack_fwrite(&table->posts[i],sizeof(Thisc_post),fp);
        checksum=generate_checksum(&table->posts[i]);
        pack_fwrite(&checksum,sizeof(int),fp);
    }
}

int draw_table(BITMAP *dst,int x,int y,char *header,Thisc_table *table)
{
    int i;
    int yPos;
    int col;

    col=makecol(30,20,10);
    if (dst) textprintf_ex(dst,data[51].dat,x,y-20,-1,-1,"%s",header);
    for (yPos=y+15,i=0;i<5;i++) {
        if (table->posts[i].value) {
            if (dst) {
                textprintf_right_ex(dst,data[53].dat,x+20,yPos,col,-1,"%d.",i+1);
                textprintf_ex(dst,data[53].dat,x+25,yPos,col,-1,"%s",table->posts[i].name);
                textprintf_right_ex(dst,data[53].dat,x+220,yPos,col,-1,"%d",table->posts[i].value);
            }
            yPos+=12;
        }
    }
    return yPos;
}

void view_scores(Thisc_table **tables,char **names)
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
        pageY=(int)((targetY-pageY)*0.2+pageY);
        dark=(int)((targetDark-dark)*0.2+dark);
        blit(bg,swap_screen,0,0,0,0,SCREEN_W,SCREEN_H);
        set_trans_blender(0,0,0,dark);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        draw_sprite(swap_screen,bmp,160,pageY);
        draw_sprite(swap_screen,data[9].dat,626-dark,380);
        draw_sprite(swap_screen,data[6].dat,626-dark,40);
        blit_to_screen(swap_screen);
        while (!cycle_count)
            rest(2);
    }
    clear_keybuf();
    destroy_bitmap(bmp);
    destroy_bitmap(bg);
}
