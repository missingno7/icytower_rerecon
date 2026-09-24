#include "map.h"
#include <stdlib.h>

#include "recovered/Treplay.h"
typedef Treplay Tmap_replay;

Tmap_replay *get_demo(void);

int floor_size_modifiers[5] = {2, 0, -2, -4, -6};

void reset_map(Tmap *m)
{
    int i;
    for (i = 0; i < 32; i++) {
        m->room[i].empty = -1;
        m->room[i].level = 0;
        m->room[i].sign = 0;
    }
    m->offset = 0;
}

void add_floor(Tmap *m)
{
    int i;
    int width;

    for (i=0; i<31; i++)
        m->room[i]=m->room[i+1];

    m->room[31].tiles = m->room[31].level>4999 ? 10 : m->room[31].level/500;
    if ((m->room[31].level%250==0 && m->room[31].level<=5004) || m->room[31].level%2500==0) {
        m->room[31].empty=0;
        m->room[31].level++;
        m->room[31].start_tile=0;
        m->room[31].end_tile=40;
    }
    else if (m->room[31].level%5==0) {
        m->room[31].empty=0;
        m->room[31].level++;
        if (get_demo()->floor_shrink) {
            if (m->room[31].level>2999) {
                if (m->room[31].level<=5004)
                    width=6;
                else {
                    width=5;
                    if (m->room[31].level>7504) {
                        width=4;
                        if (m->room[31].level>10004)
                            width=m->room[31].level<50005 ? 3 : 2;
                    }
                }
            }
            else {
                width=rand();
                if (((float)(300-m->room[31].level/5)/300.0f)*10.0f<1.0f) width=6;
                else {
                    int max_w=(int)(((float)(300-m->room[31].level/5)/300.0f)*10.0f);

                    width=width%max_w+6;
                }
            }
        }
        else width=rand()%10+6;

        width+=floor_size_modifiers[get_demo()->floor_size];
        if (width>0) {
            m->room[31].start_tile=rand()%(30-width)+5;
            m->room[31].end_tile=m->room[31].start_tile+width;
        }
        else {
            m->room[31].start_tile=rand()%29+5;
            m->room[31].end_tile=m->room[31].start_tile+1;
        }
    }
    else {
        m->room[31].empty=-1;
        m->room[31].level++;
    }

    if ((m->room[31].level-1)%50==0)
        m->room[31].sign=m->room[31].level/5;
    else
        m->room[31].sign=0;
}

int is_solid(Tmap *m, int cx, int cy)
{
    int x, y;
    y = (cy + 1) >> 4;
    if (29 - y < 0 || 29 - y > 31) return 0;
    if (m->room[29 - y].empty != 0) return 0;
    x = cx >> 4;
    if (x < m->room[29 - y].start_tile) return 0;
    if (x > m->room[29 - y].end_tile) return 0;
    return cy - (m->offset % 16) + 10000 - (y << 4);
}

int get_level(Tmap *m, int cy)
{
    int y = 29 - ((cy + 1) >> 4);
    if (y < 0 || y > 31) return 0;
    return m->room[y].level;
}

void getFloorData(Tmap *m, int cy, int *fy, int *fx1, int *fx2)
{
    int y = (cy + 1) >> 4;
    if (29 - y < 0 || 29 - y > 31) return;
    if (m->room[29 - y].empty != 0) return;
    *fx1 = (m->room[29 - y].start_tile << 4) - 2;
    *fx2 = ((m->room[29 - y].end_tile + 1) << 4) + 1;
    *fy = (y << 4) + (m->offset % 16);
}
