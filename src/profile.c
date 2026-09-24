#include "recovered/Tcontrol.h"
#include <stdio.h>
#include <allegro.h>
#include "recovered/Tcontrol.h"
extern void save_control(Tcontrol*, FILE*);
extern void poll_control(Tcontrol*, int);
extern int is_up(Tcontrol*);
extern int is_fire(Tcontrol*);
extern int is_down(Tcontrol*);
extern int is_any(Tcontrol*);

typedef struct Tprofile_rank {
    unsigned char before_score[0x4c];
    int score;
    int combo;
    unsigned char before_ccc[4];
    int ccc;
    unsigned char before_nml[0x2c];
    int no_combo_lost;
} Tprofile_rank;

char *jcLabels[5] = {
    "Singles in a Row:   ", "Doubles in a Row:   ",
    "Triples in a Row:   ", "Quadruples in a Row:",
    "Quintuples in a Row:"
};
char *rankLables[12] = {
    "no rank", "F", "E", "D", "C", "B", "A", "*", "**", "***",
    "****", "*****"
};
int rankFloors[12] = { 0, 50, 100, 150, 200, 300, 400, 500, 600, 750, 1000, 1500 };
int rankCombos[12] = { 0, 0, 7, 15, 25, 35, 70, 120, 200, 300, 400, 650 };
int rankCCCs[12] = { 0, 0, 55, 65, 75, 85, 95, 105, 115, 125, 135, 145 };
int rankNMLs[12] = { 0, 0, 0, 0, 0, 0, 0, 400, 500, 600, 700, 1200 };
char *comboNames[10] = {
    "Good:", "Sweet:", "Great:", "Super:", "WOW:", "Amazing:",
    "Extreme:", "Fantastic:", "Splendid:", "No way!"
};

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_checksum;

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_extra;

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_basic;

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_advanced;

typedef struct Tprofile_datafile {
    void *dat;
    int type;
    long size;
    void *prop;
} Tprofile_datafile;

extern Tprofile_datafile *data;
extern int makecol(int r, int g, int b);
extern int stricmp(const char *s1, const char *s2);
extern void set_trans_blender(int r, int g, int b, int a);
extern void solid_mode(void);

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_load;

extern int get_profile_dir_for_profile(char *buffer, size_t buflen,
                                       const char *profile);
extern Tcontrol *get_controls(void);
extern void load_control(Tcontrol*, FILE*);

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_create;

typedef struct Tprofile_tm {
    int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year;
} Tprofile_tm;

extern void log2file(const char *format, ...);
extern long time(long *t);
extern void init_control(Tcontrol*);
extern int save_profile(Tprofile_create *p);

#include "recovered/Tprofile.h"
typedef Tprofile Tprofile_general;

/* These interfaces and the profile viewer's layout are recovered from
 * profile.c lines 538--633 and 0x419aec..0x41a3b5. */
typedef struct Tprofile_bitmap {
    int w;
    int h;
} Tprofile_bitmap;

extern void *swap_screen;
extern volatile int cycle_count;
extern int closeButtonClicked;
extern int keypressed(void);
extern void clear_keybuf(void);
extern void checkMenuFocus(void);
extern void blit_to_screen(BITMAP*);

#include "recovered/Tcontrol.h"
typedef Tcontrol Tprofile_control;
extern int readkey(void);
extern void simulate_keypress(int keycode);
extern int my_alert(char *func, char *txt, int choice, int enter_hint);
extern void replaceBadCharacters(char *string, char newChar);
extern void play_menu_select(void);
extern void play_menu_move(void);
extern int get_string(BITMAP*, char*, int, int, FONT*, int, int, int, int);

/* Forward declarations; definitions follow in their original source order. */
unsigned int hash2(unsigned int a);
int generate_profile_checksum(Tprofile_checksum *p);
Tprofile_create *create_profile(char *handle, int overwrite);
inline int get_rank_id(Tprofile_rank *profile);
inline char *get_rank(Tprofile_rank *profile);
void set_next_rank_message(char *buf, Tprofile_rank *p);
void delete_profile(char *handle);
Tprofile_load *load_profile(char *handle);
char *profile_data_page_extra(Tprofile_extra *p);
char *profile_data_page_general(Tprofile_general *p, char *filler);
char *profile_data_page_basic(Tprofile_basic *p);
char *profile_data_page_advanced(Tprofile_advanced *p);
int save_profile(Tprofile_create *p);
int draw_buffer(BITMAP *bmp, char *buffer, int x, int y);
int view_profile(Tprofile*);

/* Forward declarations; definitions follow in their original source order. */
unsigned int hash2(unsigned int a);
int generate_profile_checksum(Tprofile_checksum *p);
Tprofile_create *create_profile(char *handle, int overwrite);
inline int get_rank_id(Tprofile_rank *profile);
inline char *get_rank(Tprofile_rank *profile);
void set_next_rank_message(char *buf, Tprofile_rank *p);
void delete_profile(char *handle);
Tprofile_load *load_profile(char *handle);
char *profile_data_page_extra(Tprofile_extra *p);
char *profile_data_page_general(Tprofile_general *p, char *filler);
char *profile_data_page_basic(Tprofile_basic *p);
char *profile_data_page_advanced(Tprofile_advanced *p);
int save_profile(Tprofile_create *p);
int draw_buffer(BITMAP *bmp, char *buffer, int x, int y);
int view_profile(Tprofile*);

#include "recovered/Tavailable_profile.h"
extern int rebuild_profile_list(Tavailable_profile **profs);
void draw_profile_selector(BITMAP *bmp, Tprofile *current_profile, Tavailable_profile *profiles, int numProfiles, int selection, int offset, int max_posts, int x, int y);
Tprofile_create *select_profile(Tprofile_create *current_profile, Tavailable_profile *profiles, int numProfiles, Tprofile_control *ctrl);

/* Historical CU: F:\projects\icytower\trunk\source\profile.c
 * Ownership: GAME
 * Current recovery status: src/recovery.json and docs/current/.
 * Historical function: hash2 @ 0x004189cc, 71 bytes
 * Historical function: generate_profile_checksum @ 0x00418a14, 112 bytes
 * Historical function: get_rank_id @ 0x00418a84, 75 bytes
 * Historical function: get_rank @ 0x00418ad0, 82 bytes
 * Historical function: set_next_rank_message @ 0x00418b24, 431 bytes
 * Historical function: draw_profile_selector @ 0x00418cd4.
 * Historical function: draw_buffer @ 0x004191c8, 185 bytes
 * Historical function: profile_data_page_advanced @ 0x00419284, 332 bytes
 * Historical function: profile_data_page_basic @ 0x004193d0, 637 bytes
 * Historical function: profile_data_page_extra @ 0x00419650, 85 bytes
 * Historical function: profile_data_page_general @ 0x004196a8, 1091 bytes
 * Historical function: view_profile @ 0x00419aec, 2249 bytes
 * Historical function: save_profile @ 0x0041a3b8, 1073 bytes
 * Historical function: load_profile @ 0x0041a7ec, 188 bytes
 * Historical function: delete_profile @ 0x0041a8a8, 222 bytes
 * Historical function: create_profile @ 0x0041a988, 823 bytes
 * Historical function: select_profile @ 0x0041acc0, 3070 bytes
 */

unsigned int hash2(unsigned int a)
{
    a = (a ^ 0x3dU) ^ (a >> 16);
    a *= 9U;
    a ^= a >> 4;
    a *= 668265261U;
    a ^= a >> 15;
    return a;
}

int generate_profile_checksum(Tprofile_checksum *p)
{
    int i, cs;
    int oldCS;
    int *pos;

    oldCS = p->checksum;
    pos = (int *)p;
    p->checksum = 0;
    cs = 0;
    for (i = 0; i < sizeof(Tprofile) / 4; i++) cs += pos[i] * (i + 1);
    p->checksum = oldCS;
    return hash2(cs);
}

Tprofile_create *create_profile(char *handle, int overwrite)
{
    char file[1024];
    Tprofile_create *p;
    int i;
    long now;
    Tprofile_tm *my_time;
    int year, month, day;

    get_profile_dir_for_profile(file, 1024, handle);
    sprintf(file, "%s%s.itp", file, handle);
    if (!overwrite && exists(file)) {
        log2file("Overwrite is false and %s already exists", file);
        return 0;
    }
    p = malloc(sizeof(Tprofile_create));
    p->header[0] = 'I';
    p->header[1] = 'T';
    p->header[2] = 'P';
    p->header[3] = '1';
    p->header[4] = '4';
    p->header[5] = '0';
    strcpy(p->handle, handle);
    p->custom_games_played = 0;
    p->games_played = 0;
    p->games_quit = 0;
    p->seconds_spent_playing = 0;
    p->best_score = 0;
    p->best_combo = 0;
    p->best_floor = 0;
    p->total_combo_floors = 0;
    p->total_combos = 0;
    p->total_score = 0;
    p->total_floors = 0;
    p->no_combo_top_floor = 0;
    p->biggest_lost_combo = 0;
    p->cccTotal[4] = 0;
    p->cccTotal[3] = 0;
    p->cccTotal[2] = 0;
    p->cccTotal[1] = 0;
    p->cccTotal[0] = 0;
    p->cccNum[4] = 0;
    p->cccNum[3] = 0;
    p->cccNum[2] = 0;
    p->cccNum[1] = 0;
    p->cccNum[0] = 0;
    p->ccc[4] = 0;
    p->ccc[3] = 0;
    p->ccc[2] = 0;
    p->ccc[1] = 0;
    p->ccc[0] = 0;
    p->jc[4] = 0;
    p->jc[3] = 0;
    p->jc[2] = 0;
    p->jc[1] = 0;
    p->jc[0] = 0;
    for (i = 0; i < 10; i++)
        p->rewards[i] = 0;
    for (i = 0; i < 32; i++)
        *p->best_replay_names[i] = 0;
    p->total_jumps = 0;
    strcpy(p->last_avatar, "harold_the_homeboy");
    p->jump_hold = 1;
    p->start_floor = 0;
    p->flash = 0;
    p->msc_volume = 150;
    p->snd_volume = 150;
    now = time(0);
    my_time = localtime(&now);
    year = my_time->tm_year;
    month = my_time->tm_mon + 1;
    day = my_time->tm_mday;
    sprintf(p->creationDate, "%d-%s%d-%s%d", year + 1900,
            month < 10 ? "0" : "", month, day < 10 ? "0" : "", day);
    strcpy(p->saveDate, p->creationDate);
    init_control(get_controls());
    if (save_profile(p) < 0) {
        log2file("failed to create profile %s", handle);
        free(p);
        return 0;
    }
    return p;
}

inline int get_rank_id(Tprofile_rank *profile)
{
    int i;

    for (i = 11; i >= 0; i--) {
        if (profile->score >= rankFloors[i]
         && profile->combo >= rankCombos[i]
         && profile->ccc >= rankNMLs[i]
         && profile->no_combo_lost >= rankCCCs[i])
            return i;
    }
    return 0;
}

inline char *get_rank(Tprofile_rank *profile)
{
    return rankLables[get_rank_id(profile)];
}

void set_next_rank_message(char *buf, Tprofile_rank *p)
{
    int current_rank;
    int next_rank;
    int next_floor;
    int next_combo;
    int next_nml;
    int next_ccc;

    buf[0] = 0;
    current_rank = get_rank_id(p);
    if (current_rank == 11)
        return;
    next_rank = current_rank + 1;
    next_floor = rankFloors[next_rank];
    next_combo = rankCombos[next_rank];
    next_nml = rankNMLs[next_rank];
    next_ccc = rankCCCs[next_rank];
    if ((next_floor > p->score || p->score == 0) && next_floor)
        sprintf(buf, "%s\n - Get to floor %d!", buf, next_floor);
    if ((next_combo > p->combo || p->combo == 0) && next_combo)
        sprintf(buf, "%s\n - Make a %d floor combo!", buf, next_combo);
    if ((next_ccc > p->no_combo_lost || p->no_combo_lost == 0) && next_ccc)
        sprintf(buf, "%s\n - Reach floor %d before 1st Hurry Up!", buf,
                next_ccc);
    if ((next_nml > p->ccc || p->ccc == 0) && next_nml)
        sprintf(buf, "%s\n - Reach floor %d without combos!", buf,
                next_nml);
}

void delete_profile(char *handle)
{
    char file[1024];

    get_profile_dir_for_profile(file, 1024, handle);
    strcat(file, "replays");
    rmdir(file);
    get_profile_dir_for_profile(file, 1024, handle);
    sprintf(file, "%s%s.itp", file, handle);
    delete_file(file);
    get_profile_dir_for_profile(file, 1024, handle);
    sprintf(file, "%s%s_stats.txt", file, handle);
    delete_file(file);
    get_profile_dir_for_profile(file, 1024, handle);
    rmdir(file);
}

Tprofile_load *load_profile(char *handle)
{
    char file[1024];
    void *fp;
    Tprofile_load *p;
    int cs;

    get_profile_dir_for_profile(file, 1024, handle);
    sprintf(file, "%s%s.itp", file, handle);
    fp = fopen(file, "rb");
    if (!fp)
        return 0;
    p = malloc(0x550);
    fread(p, 0x550, 1, fp);
    load_control(get_controls(), fp);
    fclose(fp);
    cs = generate_profile_checksum(p);
    if (cs != p->checksum) {
        free(p);
        p = 0;
    }
    return p;
}

char *profile_data_page_extra(Tprofile_extra *p)
{
    char *data;

    data = malloc(2048);
    data[0] = 0;
    sprintf(data, "%sTotal jumps:    %d\n", data, p->total_jumps);
    sprintf(data, "%s\n", data);
    return data;
}

char *profile_data_page_general(Tprofile_general *p, char *filler)
{
    char *data;
    int seconds;
    int minutes;
    int hours;
    int days;
    char timeSpent[256];
    char buf[80];
    int len;
    int i;

    data = malloc(2048);
    data[0] = 0;
    sprintf(data, "%sCreated:        %s%s\n", data, filler, p->creationDate);
    sprintf(data, "%s\n", data);
    seconds = p->seconds_spent_playing % 60;
    minutes = (p->seconds_spent_playing / 60) % 60;
    hours = (p->seconds_spent_playing / 3600) % 24;
    days = p->seconds_spent_playing / 86400;
    strcpy(timeSpent, "none");
    if (seconds > 0)
        sprintf(timeSpent, "%d second%s", seconds, seconds == 1 ? "" : "s");
    if (minutes > 0)
        sprintf(timeSpent, "%d minute%s, %d second%s", minutes,
                minutes == 1 ? "" : "s", seconds,
                seconds > 1 ? "s" : "");
    if (hours > 0)
        sprintf(timeSpent, "%d hour%s, %d minute%s, %d second%s", hours,
                hours == 1 ? "" : "s", minutes,
                minutes > 1 ? "s" : "", seconds,
                seconds > 1 ? "s" : "");
    if (days > 0)
        sprintf(timeSpent, "%d day%s, %d hour%s, %d minute%s, %d second%s",
                days, days == 1 ? "" : "s", hours,
                hours > 1 ? "s" : "", minutes,
                minutes > 1 ? "s" : "", seconds,
                seconds > 1 ? "s" : "");
    sprintf(data, "%sTime played:    %s%s\n", data, filler, timeSpent);
    sprintf(buf, "%d", p->games_played);
    len = strlen(buf);
    buf[0] = 0;
    for (i = 10; i >= len; i--)
        strcat(buf, " ");
    sprintf(data, "%sGames played:   %s%d     %s  Custom games played:  %5d\n",
            data, filler, p->games_played, buf, p->custom_games_played);
    sprintf(data, "%sGames quit:     %s%d\n", data, filler, p->games_quit);
    sprintf(data, "%s\n", data);
    return data;
}

char *profile_data_page_basic(Tprofile_basic *p)
{
    char *data;
    int i;

    data = malloc(2048);
    data[0] = 0;
    sprintf(data, "%sBest score ever:    %7d\n", data, p->best_score);
    if (p->games_played > 0)
        sprintf(data, "%sAvg score per game: %7d\n", data,
                p->total_score / p->games_played);
    sprintf(data, "%sTotal score:        %7d\n", data, p->total_score);
    sprintf(data, "%s\n", data);
    sprintf(data, "%sHighest floor ever: %7d\n", data, p->best_floor);
    if (p->games_played > 0)
        sprintf(data, "%sAvg floors per game:%7d\n", data,
                p->total_floors / p->games_played);
    sprintf(data, "%sFloors jumped:      %7d\n", data, p->total_floors);
    sprintf(data, "%s\n", data);
    if (p->no_combo_top_floor) {
        sprintf(data, "%sTop Floor, No Combo:%7d\n", data,
                p->no_combo_top_floor);
        sprintf(data, "%s\n", data);
    }
    sprintf(data, "%sBest combo ever:    %7d\n", data, p->best_combo);
    if (p->games_played > 0)
        sprintf(data, "%sAvg combos per game:%7d\n", data,
                p->total_combos / p->games_played);
    if (p->total_combos > 0)
        sprintf(data, "%sAvg combo length:   %7d\n", data,
                p->total_combo_floors / p->total_combos);
    sprintf(data, "%sCombos jumped:      %7d\n", data, p->total_combos);
    sprintf(data, "%s\n", data);
    if (p->biggest_lost_combo > 0) {
        sprintf(data, "%sLongest Lost Combo: %7d\n", data,
                p->biggest_lost_combo);
        sprintf(data, "%s\n", data);
    }
    for (i = 0; i < 5; i++)
        if (p->jc[i] > 0)
            sprintf(data, "%s%s%7d\n", data, jcLabels[i], p->jc[i]);
    if (p->jc[0] + p->jc[1] + p->jc[2] + p->jc[3] + p->jc[4] > 0)
        sprintf(data, "%s\n", data);
    return data;
}

char *profile_data_page_advanced(Tprofile_advanced *p)
{
    char *data;
    int i;
    int rows;

    data = malloc(2048);
    data[0] = 0;
    for (i = 1; i < 6; i++)
        if (p->ccc[i - 1] > 0)
            sprintf(data, "%sClock Challenge %d:  %7d\n", data, i,
                    p->ccc[i - 1]);
    if (p->ccc[0] > 0)
        sprintf(data, "%s\n", data);
    for (i = 0; i < 5; i++)
        if (p->ccc[i] > 0 && p->cccNum[i] > 0)
            sprintf(data, "%sAverage CC %d:       %7d\n", data, i + 1,
                    p->cccTotal[i] / p->cccNum[i]);
    if (p->cccTotal[0] > 0)
        sprintf(data, "%s\n", data);
    rows = 0;
    for (i = 0; i < 10; i++)
        if (p->rewards[i] > 0) {
            sprintf(data, "%s%-12s        %7d\n", data, comboNames[i],
                    p->rewards[i]);
            rows++;
        }
    if (rows)
        sprintf(data, "%s\n", data);
    return data;
}

int save_profile(Tprofile_create *p)
{
    char file[1024];
    long now;
    Tprofile_tm *my_time;
    int year, month, day;
    void *fp;
    char *data_basic;
    char *data_advanced;
    char *data_general;
    char *data_extra;

    get_profile_dir_for_profile(file, 1024, p->handle);
    if (!file_exists(file, 16, 0))
        mkdir(file);
    strcat(file, "replays/");
    if (!file_exists(file, 16, 0))
        mkdir(file);
    get_profile_dir_for_profile(file, 1024, p->handle);
    sprintf(file, "%s%s.itp", file, p->handle);
    now = time(0);
    my_time = localtime(&now);
    year = my_time->tm_year;
    month = my_time->tm_mon + 1;
    day = my_time->tm_mday;
    sprintf(p->saveDate, "%d-%s%d-%s%d", year + 1900,
            month < 10 ? "0" : "", month, day < 10 ? "0" : "", day);
    p->checksum = generate_profile_checksum((Tprofile_checksum *)p);
    fp = fopen(file, "wb");
    if (!fp) {
        log2file("Failed to open \"%s\" for writing", file);
        return -1;
    }
    fwrite(p, 0x550, 1, fp);
    save_control(get_controls(), fp);
    fclose(fp);
    get_profile_dir_for_profile(file, 1024, p->handle);
    sprintf(file, "%s%s_stats.txt", file, p->handle);
    fp = fopen(file, "wt");
    if (!fp) {
        log2file("failed to open profile stats \"%s\" for writing", file);
        return -1;
    }
    fwrite("ICY TOWER 1.4 PROFILE\n", 1, 22, fp);
    fwrite("**********************************************\n", 1, 47, fp);
    fprintf(fp, "Profile name:          %s\n", p->handle);
    fprintf(fp, "Last updated:          %s\n", p->saveDate);
    fprintf(fp, "Rank:                  %s\n", get_rank((Tprofile_rank *)p));
    data_general = profile_data_page_general((Tprofile_general *)p, "       ");
    data_basic = profile_data_page_basic((Tprofile_basic *)p);
    data_advanced = profile_data_page_advanced((Tprofile_advanced *)p);
    data_extra = profile_data_page_extra((Tprofile_extra *)p);
    fputs(data_general, fp);
    fputs(data_basic, fp);
    fputs(data_advanced, fp);
    fputs(data_extra, fp);
    fputc('\n', fp);
    fclose(fp);
    free(data_general);
    free(data_basic);
    free(data_advanced);
    free(data_extra);
    return 0;
}

int draw_buffer(BITMAP *bmp, char *buffer, int x, int y)
{
    int pos;
    char tempBuf[256];
    int tempPos;
    char c;

    pos = y;
    tempPos = 0;
    c = *buffer;
    while (c) {
        if (c == '\n') {
            tempBuf[tempPos] = 0;
            textprintf_ex(bmp, data[53].dat, x, pos, makecol(30, 20, 10),
                          -1, "%s", tempBuf);
            pos += 10;
            tempPos = 0;
        } else {
            tempBuf[tempPos] = c;
            tempPos++;
        }
        c = buffer[1];
        buffer++;
    }
    return pos;
}

int view_profile(Tprofile *profile)
{
    char *data_basic;
    char *data_advanced;
    char *data_general;
    int pageY;
    int targetY;
    void *bg;
    void *bmp;
    int y;
    int y1;
    int y2;
    char nextRankMessage[1024];
    char totalNextRankMessage[1024];
    int done;
    int rank;

    clear_keybuf();
    while (is_any(get_controls()) || key[KEY_SPACE])
        poll_control(get_controls(), 0);
    clear_keybuf();

    bg = create_bitmap(640, 480);
    draw_sprite(bg, screen, 0, 0);
    bmp = create_bitmap(((Tprofile_bitmap *)data[86].dat)->w + 50,
                        ((Tprofile_bitmap *)data[86].dat)->h + 50);
    clear_to_color(bmp, makecol(255, 0, 255));
    draw_sprite(bmp, data[86].dat, 0, 50);

    data_general = profile_data_page_general((Tprofile_general *)profile,
                                             "");
    sprintf(data_general,
            "%s-------------------------------------------------------------\n\n",
            data_general);
    data_basic = profile_data_page_basic((Tprofile_basic *)profile);
    data_advanced = profile_data_page_advanced((Tprofile_advanced *)profile);

    textprintf_ex(bmp, data[51].dat, 75, 10, -1, -1, "profile: %s",
                  (char *)profile + 6);
    y = draw_buffer(bmp, data_general, 85, 45);
    y1 = draw_buffer(bmp, data_basic, 85, y);
    y2 = draw_buffer(bmp, data_advanced, 290, y);
    if (y2 < y1)
        y2 = y1;
    draw_buffer(bmp,
                "-------------------------------------------------------------\n\n",
                85, y2);

    rank = get_rank_id((Tprofile_rank *)profile);
    draw_sprite(bmp, data[74 + rank].dat, 35,
                ((Tprofile_bitmap *)bmp)->h - 140);
    set_next_rank_message(nextRankMessage, (Tprofile_rank *)profile);
    if (strlen(nextRankMessage) > 1) {
        sprintf(totalNextRankMessage, "To reach next rank:%s\n",
                nextRankMessage);
        draw_buffer(bmp, totalNextRankMessage, 150,
                    ((Tprofile_bitmap *)bmp)->h - 120);
    }

    pageY = 500;
    targetY = 15;
    done = 0;
    while (!closeButtonClicked && !done) {
        cycle_count = 0;
        checkMenuFocus();
        draw_sprite(swap_screen, bg, 0, 0);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_sprite(swap_screen, bmp, 70, pageY);
        blit_to_screen(swap_screen);
        poll_control(get_controls(), 0);
        done = is_fire(get_controls());
        if (keypressed())
            done = 1;
        while (!cycle_count)
            rest(2);
        pageY += (int)((targetY - pageY) * 0.2f);
    }

    targetY = 500;
    while (pageY <= 480) {
        cycle_count = 0;
        checkMenuFocus();
        draw_sprite(swap_screen, bg, 0, 0);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_sprite(swap_screen, bmp, 70, pageY);
        pageY += (int)((targetY - pageY) * 0.2f);
        blit_to_screen(swap_screen);
        while (!cycle_count)
            rest(2);
    }

    free(data_basic);
    free(data_advanced);
    free(data_general);
    destroy_bitmap(bmp);
    destroy_bitmap(bg);
    clear_keybuf();
}

void draw_profile_selector(BITMAP *bmp, Tprofile *current_profile, Tavailable_profile *profiles,
                           int numProfiles, int selection, int offset,
                           int max_posts, int x, int y)
{
    int fh;
    int fg;
    double view_percentage;
    double view_offset;
    int i;
    int profile_index;
    int row_y;
    char *profile_name;
    char *current;

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
    textout_ex(bmp,data[51].dat,x+10,y-12,-1,-1,"SELECT PROFILE");
    draw_sprite(bmp,data[73].dat,x+270,y+24);
    set_clip_rect(bmp,x+6,0,x+290,((int *)bmp)[1]-1);

    row_y=y+fh+31;
    for (i=1,profile_index=offset;
         i<=max_posts && profile_index<numProfiles;
         i++,profile_index++,row_y+=fh) {
        profile_name=profiles[profile_index].handle;
        current=stricmp(profile_name,current_profile->handle)==0 ? "(current)" : "";
        if (profile_index==selection) {
            drawing_mode(5,0,0,0);
            set_trans_blender(0,0,0,50);
            rectfill(bmp,x+7,row_y-47,x+263,row_y+fh,fg);
            solid_mode();
        }
        textprintf_ex(bmp,data[51].dat,x+8,row_y,fg,-1,"%c %c %s %s.",
                      profile_index==selection ? '>' : ' ',
                      profile_index<1 ? '~' : '{',profile_name,current);
    }
    set_clip_rect(bmp,0,0,((int *)bmp)[0]-1,((int *)bmp)[1]-1);
}

/* Recovered from profile.c lines 705--867.  The selector owns neither the
 * packed name list nor its input control; it returns a newly loaded profile. */
Tprofile_create *select_profile(Tprofile_create *current_profile, Tavailable_profile *profiles,
                                int numProfiles, Tprofile_control *ctrl)
{
    Tprofile_create *selectedProfile;
    int kp;
    int done;
    void *old_font;
    int profileIndex;
    int offset;
    int page_size;
    int ctrl_wait;
    int pageY;
    int targetY;
    void *bgbmp;

    old_font = font;
    font = data[54].dat;
    bgbmp = create_bitmap(640, 480);
    blit(screen, bgbmp, 0, 0, 0, 0, 640, 480);
    clear_keybuf();
    selectedProfile = 0;
    profileIndex = 0;
    offset = 0;
    page_size = 17;
    ctrl_wait = 1000;
    pageY = 500;
    targetY = 50;
    done = 0;

    while (!closeButtonClicked && !done) {
        cycle_count = 0;
        checkMenuFocus();
        poll_control(ctrl, 1);
        if (is_any(ctrl) && !ctrl_wait) {
            if (is_down(ctrl))
                simulate_keypress(0x5500);
            else if (is_up(ctrl))
                simulate_keypress(0x5400);
            else if (is_fire(ctrl))
                simulate_keypress(0x4300);
            ctrl_wait = 20;
        }
        if (is_any(ctrl)) {
            if (ctrl_wait > 0)
                ctrl_wait--;
        }

        if (keypressed()) {
            kp = readkey() >> 8;
            switch (kp) {
            case 85: {
                if (profileIndex < numProfiles - 1) {
                    profileIndex++;
                    if (profileIndex >= offset + page_size)
                        offset++;
                    play_menu_move();
                } else {
                    profileIndex = numProfiles - 1;
                    offset = numProfiles - page_size;
                    if (offset < 0)
                        offset = 0;
                }
                break;
            }
            case 84: {
                if (profileIndex > 0) {
                    profileIndex--;
                    if (offset > profileIndex)
                        offset--;
                    play_menu_move();
                } else {
                    profileIndex = 0;
                    offset = 0;
                }
                break;
            }
            case 83: {
                char *name = profiles[profileIndex].handle;
                char buff[256];
                if (stricmp(name, "guest") &&
                    stricmp(name, current_profile->handle)) {
                    sprintf(buff, "Really delete '%s'?", name);
                    if (my_alert(buff, "WARNING: It will be gone forever.",
                                 1, 0)) {
                        delete_profile(name);
                        numProfiles = rebuild_profile_list(&profiles);
                        if (profileIndex >= numProfiles)
                            profileIndex = numProfiles - 1;
                    }
                }
                break;
            }
            case 67: {
                char *name = profiles[profileIndex].handle;
                char new_name[32];
                play_menu_select();
                if (!stricmp(name, "CREATE NEW PROFILE")) {
                    set_trans_blender(0, 0, 0, 158);
                    drawing_mode(5, 0, 0, 0);
                    rectfill(swap_screen, 0, 0,
                             SCREEN_W,
                             SCREEN_H,
                             makecol(0, 0, 0));
                    solid_mode();
                    new_name[0] = 0;
                    draw_sprite(swap_screen, data[88].dat, 100, 140);
                    textout_ex(swap_screen, data[51].dat, "Enter profile name:",
                               140, 140, -1, -1);
                    textout_right_ex(swap_screen, data[54].dat,
                                     "...and press enter.", 480, 210, 0, -1);
                    rect(swap_screen, 139, 191, 480, 210,
                         makecol(255, 255, 255));
                    rectfill(swap_screen, 139, 191, 480, 210,
                             makecol(80, 80, 80));
                    if (get_string(swap_screen, new_name, 340, 32, data[54].dat,
                                   140, 191, makecol(0, 0, 0), -1) >= 0 &&
                        new_name[0]) {
                        replaceBadCharacters(new_name, '_');
                        selectedProfile = create_profile(new_name, 0);
                        if (selectedProfile) {
                            my_alert("CREATE PROFILE", "Profile created!", 0, 1);
                            done = -1;
                        } else {
                            my_alert("CREATE PROFILE", "Failed to create profile.", 0, 1);
                        }
                    }
                } else {
                    selectedProfile = load_profile(name);
                    if (selectedProfile)
                        done = -1;
                    else
                        my_alert("SELECT PROFILE",
                                 "The profile you selected is broken.", 0, 1);
                }
                break;
            }
            case 59: {
                play_menu_select();
                clear_keybuf();
                done = -1;
                break;
            }
            }
        }

        blit(bgbmp, swap_screen, 0, 0, 0, 0, 640, 480);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_profile_selector(swap_screen, current_profile,
                              profiles, numProfiles, profileIndex, offset,
                              page_size, 16, pageY);
        blit_to_screen(swap_screen);
        while (!cycle_count)
            rest(2);
        pageY += (int)((targetY - pageY) * 0.2f);
    }

    if (selectedProfile) {
        char buf[128];
        sprintf(buf, "Now using profile '%s'", selectedProfile->handle);
        my_alert("Profile Changed!", buf, 0, 1);
    }

    targetY = 510;
    while (pageY <= 499) {
        cycle_count = 0;
        pageY += (int)((targetY - pageY) * 0.2f);
        blit(bgbmp, swap_screen, 0, 0, 0, 0, 640, 480);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_profile_selector(swap_screen, current_profile,
                              profiles, numProfiles, profileIndex, offset,
                              page_size, 16, pageY);
        blit_to_screen(swap_screen);
        while (!cycle_count)
            rest(2);
    }
    destroy_bitmap(bgbmp);
    font = old_font;
    return selectedProfile;
}
