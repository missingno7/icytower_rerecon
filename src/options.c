#include "recovered/Toptions.h"
#include <allegro.h>
/* Historical CU: F:\projects\icytower\trunk\source\options.c
 * Ownership: GAME
 */

unsigned int hash3(unsigned int a)
{
    a = (a ^ 0x3dU) ^ (a >> 16);
    a *= 9U;
    a ^= a >> 4;
    a *= 668265261U;
    a ^= a >> 15;
    return a;
}

#include "recovered/Toptions.h"

extern char *strcpy(char *dst,const char *src);
extern int get_sort_method(void);
extern void set_sort_method(int sm);
void reset_options(Toptions *o);

int generate_options_checksum(Toptions *o)
{
    int i,cs;
    int values[11] = {
        o->flash, o->full_screen, o->jump_hold, o->msc_volume, o->snd_volume,
        o->floor_size, o->floor_shrink, o->gravity, o->start_speed,
        o->speed_increase, o->posterSize
    };

    for (i=0,cs=0;i<11;i++) cs+=(values[i]+i)*17;
    for (i=0;i<8;i++) cs+=o->updateDate[i];
    for (i=0;i<16;i++) cs+=o->updateDate[i]+o->posterDate[i];
    for (i=0;i<256;i++) cs+=o->posterUrl[i]+o->posterSrc[i];
    return hash3(cs);
}

void save_options(Toptions *o,PACKFILE *fp)
{
    o->sort_method=get_sort_method();
    o->checksum=generate_options_checksum(o);
    pack_fwrite(o,sizeof(Toptions),fp);
}

void load_options(Toptions *o,PACKFILE *fp)
{
    int cs;

    pack_fread(o,sizeof(Toptions),fp);
    cs=generate_options_checksum(o);
    if (cs!=o->checksum) reset_options(o);
    set_sort_method(o->sort_method);
}

void reset_options(Toptions *o)
{
    o->flash=0;
    o->full_screen=0;
    o->jump_hold=1;
    o->msc_volume=150;
    o->snd_volume=150;
    o->floor_shrink=1;
    o->speed_increase=1;
    o->start_speed=5;
    o->floor_size=1;
    o->gravity=1;
    o->timesStarted=0;
    o->sort_method=1;
    strcpy(o->updateDate,"2001-12-22");
    strcpy(o->lastProfile,"guest");
    strcpy(o->posterDate,"1111-22-33");
    strcpy(o->posterUrl,"http://www.freelunchdesign.com/?src=it15_game");
    strcpy(o->posterSrc,"default.dat");
    o->posterSize=file_size_ex("data/com/default.dat");
}
