/* Complete stars.c candidate reconstructed from original DWARF and code. */
#include <stdlib.h>
#include <allegro.h>
#include "stars.h"

void init_star_field(Tstar_field *sf, int w, int h, int num, int first_col,
                     int last_col, int dep, int cc)
{
    int i;
    sf->clear_color=cc;
    sf->depth=dep;
    sf->col1=first_col;
    sf->col_step=(abs(last_col-first_col)+1)/dep;
    if (first_col>last_col) sf->col_step=-sf->col_step;
    sf->width=w;
    sf->height=h;
    sf->stars=num;
    for (i=0;i<1024;i++) {
        sf->star[i].x=rand()%sf->width;
        sf->star[i].y=rand()%sf->height;
        sf->star[i].z=rand()%sf->depth;
    }
}

void draw_star_field(Tstar_field *sf, BITMAP *bmp, int x, int y)
{
    int i;
    if (sf->clear_color!=-1)
        rectfill(bmp,x,y,x+sf->width-1,y+sf->height-1,sf->clear_color);
    for (i=0;i<sf->stars;i++)
        putpixel(bmp,x+(int)sf->star[i].x,y+(int)sf->star[i].y,
                 sf->col1+(sf->depth-1-sf->star[i].z)*sf->col_step);
}

void scroll_star_field(Tstar_field *sf, double xstep, double ystep)
{
    int i;
    for (i=0;i<sf->stars;i++) {
        sf->star[i].x+=(sf->star[i].z+1)*xstep;
        sf->star[i].y+=(sf->star[i].z+1)*ystep;
        if (sf->star[i].x<0) {
            sf->star[i].x=sf->width-1;
            sf->star[i].z=rand()%sf->depth;
        }
        if (sf->star[i].y<0) {
            sf->star[i].y=sf->height-1;
            sf->star[i].z=rand()%sf->depth;
        }
        if (sf->star[i].x>=sf->width) {
            sf->star[i].x=0;
            sf->star[i].z=rand()%sf->depth;
        }
        if (sf->star[i].y>=sf->height) {
            sf->star[i].y=0;
            sf->star[i].z=rand()%sf->depth;
        }
    }
}
