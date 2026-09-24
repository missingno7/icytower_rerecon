extern int new_rand(void);
/* Complete particle.c candidate reconstructed from original DWARF and code. */
#include <allegro.h>
#include "particle.h"

int create_particle(Tparticle *p, int x, int y)
{
    int i;
    for (i=0;i<512;i++) {
        if (!p[i].intensity) {
            p[i].x=x<<16;
            p[i].y=y<<16;
            p[i].sx=((new_rand()%50)-25)<<16;
            p[i].sx/=10;
            p[i].sy=((new_rand()%50)-25)<<16;
            p[i].sy/=50;
            p[i].intensity=255;
            p[i].color=new_rand()%8;
            return i;
        }
    }
    return 0;
}

void update_particle(Tparticle *p)
{
    p->x+=p->sx;
    p->y+=p->sy;
    p->sy+=0x4ccd;
    p->intensity--;
    if (new_rand()%5==1) p->color=new_rand()%8;
}

void reset_particles(Tparticle *p)
{
    int i;
    for (i=0;i<512;i++) p[i].intensity=0;
}
