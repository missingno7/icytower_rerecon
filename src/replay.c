#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <allegro.h>
#include <recovered/Treplay_post.h>
#include "control.h"

/* Historical CU: replay.c; current status: src/recovery.json.
 * Ownership: GAME.
 *
 * Historical function inventory (current status: docs/current/):
 * Historical function: calc_replay_checksum_131 @ 0x0041ba10, 177 bytes
 * Historical function: calc_replay_checksum @ 0x0041bac4, 676 bytes
 * Historical function: update_file_list @ 0x0041bda0, 184 bytes
 * Historical function: draw_replay_selector @ 0x0041be58, 3726 bytes
 * Historical function: create_replay @ 0x0041cce8, 254 bytes
 * Historical function: load_replay @ 0x0041cde8, 1136 bytes
 * Historical function: replay_selector @ 0x0041d258, 2845 bytes
 * Historical function: save_replay @ 0x0041dd78, 1227 bytes
 * Historical function: get_replay_property @ 0x0041e244, 1147 bytes
 * Historical function: my_strcmp @ 0x0041e6c0, 128 bytes
 * Historical function: add_itr_file @ 0x0041e740, 360 bytes
 */

int sort_method = 1;
extern BITMAP *swap_screen;
extern volatile int cycle_count;

extern int stricmp(const char *a, const char *b);
extern int get_replay_property(const char *file_name, int property);
extern int for_each_file_ex(const char *pattern, int in_attrib, int out_attrib,
                            int (*callback)(const char *, int, void *), void *param);
extern int add_itr_file(const char *filename, int attrib, void *param);
extern int my_strcmp(const void *c, const void *d);
extern void qsort(void *base, size_t count, size_t size,
                  int (*compare)(const void *, const void *));
extern char *get_filename(const char *path);
extern char *get_extension(const char *path);
#ifndef FA_DIREC
#define FA_DIREC 0x10
#endif

Treplay_post itr_file_list[1024];
int num_itr_files = 0;
extern int closeButtonClicked;
static const char REPLAY_HEADER[6] = "ITR140";

#include "recovered/Trecord.h"
typedef Trecord Treplay_data;

#include "recovered/Treplay.h"

extern void free(void *ptr);
extern int memcmp(const void *a, const void *b, size_t size);
extern void log2file(const char *format, ...);
extern DATAFILE *data;
extern int is_custom_replay(Treplay *r);
extern void blit_to_screen(BITMAP *bmp);
extern void checkMenuFocus(void);
extern void play_menu_move(void);
extern void play_menu_select(void);
extern int my_alert(char *func, char *txt, int choice, int enter_hint);
Treplay *load_replay(const char *filename);

/* Forward declarations; definitions follow in their original source order. */
int get_sort_method(void);
void set_sort_method(int sm);
Treplay *create_replay(int size);
void destroy_replay(Treplay *r);
unsigned int hash(unsigned int a);
int calc_replay_checksum_131(Treplay *r);
int calc_replay_checksum(Treplay *r);
int get_replay_property(const char *filename, int property);
Treplay *load_replay(const char *filename);
int save_replay(const char *path, const char *file, Treplay *r, int size, int make_new_date);
void draw_replay_selector(BITMAP *bmp, Treplay *rep, Treplay_post *file_list, int selection, int offset, int max_posts, int x, int y);
int add_itr_file(const char *filename, int attrib, void *param);
int my_strcmp(const void *c, const void *d);
void update_file_list(char *path);
Treplay *replay_selector(Tcontrol *ctrl, char *path);

/* Forward declarations; definitions follow in their original source order. */
int get_sort_method(void);
void set_sort_method(int sm);
Treplay *create_replay(int size);
void destroy_replay(Treplay *r);
unsigned int hash(unsigned int a);
int calc_replay_checksum_131(Treplay *r);
int calc_replay_checksum(Treplay *r);
int get_replay_property(const char *filename, int property);
Treplay *load_replay(const char *filename);
int save_replay(const char *path, const char *file, Treplay *r, int size, int make_new_date);
void draw_replay_selector(BITMAP *bmp, Treplay *rep, Treplay_post *file_list, int selection, int offset, int max_posts, int x, int y);
int add_itr_file(const char *filename, int attrib, void *param);
int my_strcmp(const void *c, const void *d);
void update_file_list(char *path);
Treplay *replay_selector(Tcontrol *ctrl, char *path);

/* Forward declarations; definitions follow in their original source order. */
int get_sort_method(void);
void set_sort_method(int sm);
Treplay *create_replay(int size);
void destroy_replay(Treplay *r);
unsigned int hash(unsigned int a);
int calc_replay_checksum_131(Treplay *r);
int calc_replay_checksum(Treplay *r);
int get_replay_property(const char *filename, int property);
Treplay *load_replay(const char *filename);
int save_replay(const char *path, const char *file, Treplay *r, int size, int make_new_date);
void draw_replay_selector(BITMAP *bmp, Treplay *rep, Treplay_post *file_list, int selection, int offset, int max_posts, int x, int y);
int add_itr_file(const char *filename, int attrib, void *param);
int my_strcmp(const void *c, const void *d);
void update_file_list(char *path);
Treplay *replay_selector(Tcontrol *ctrl, char *path);

int get_sort_method(void) { return sort_method; }

void set_sort_method(int sm) { sort_method = sm; }

Treplay *create_replay(int size)
{
    Treplay *r;
    int i;

    r = malloc(sizeof(Treplay));
    if (!r)
        return 0;
    memcpy(r->header, REPLAY_HEADER, 6);
    r->comment[0] = 0;
    r->size = size;
    r->combo = 0;
    r->floor = 0;
    r->score = 0;
    for (i = 0; i < 32; i++)
        r->name[i] = 0;
    for (i = 0; i < 32; i++)
        r->name[i] = 0;
    strcpy(r->name, "Harold");
    strcpy(r->date, "no date");
    r->data = malloc(r->size * sizeof(Treplay_data) + 32);
    if (!r->data) {
        free(r);
        return 0;
    }
    for (i = 0; i < size; i++) {
        r->data[i].key_flags = 0;
        r->data[i].cycle_count = 0;
    }
    return r;
}

void destroy_replay(Treplay *r)
{
    if (r) {
        if (r->data) free(r->data);
        free(r);
    }
}

unsigned int hash(unsigned int a)
{
    a = (a ^ 0x3dU) ^ (a >> 16);
    a *= 9U;
    a ^= a >> 4;
    a *= 668265261U;
    a ^= a >> 15;
    return a;
}

int calc_replay_checksum_131(Treplay *r)
{
    int i;
    int sum;

    sum = r->random_seed * 17;
    sum += r->rejump * 26;
    sum += (r->score + 1) * 7;
    sum += (r->floor + 1) * 13;
    sum += (r->combo + 1) * 23;
    for (i = 0; i < 32; i++)
        sum += (r->date[i] + i) * (r->name[i] + i) * (i + 1) * 117;
    for (i = 0; i < r->size; i++)
        sum += (i * r->data[i].cycle_count * 3 + i * r->data[i].key_flags * 5);
    return sum;
}

int calc_replay_checksum(Treplay *r)
{

    int i;

    unsigned int sum;

    sum = r->biggest_lost_combo * 17 + 17 + r->no_combo_top_floor * 127;

    sum += sum + r->floor_shrink * 102 + r->floor_size * 17 + 3702 + r->start_speed * 163 + r->speed_increase * 23 + r->gravity * 88;

    sum += r->score * 17 + 17 + r->tc_posts * 127 + r->random_seed * 329 + (r->combo + 1) * 73 + r->rejump * 13 + (r->floor + 1) * 113;





    for (i = 0; i < 5; i++)

        sum = sum + r->ccc[i] * (39 + i * 3) + r->jc[i] * (27 + i * 3);







    for (i = 0; i < 100; i++) {

        sum += r->tc_c_data[i] * ((i + 1) % 13);

        sum += r->tc_q_data[i] * ((i + 7) % 17);

        sum += r->tc_t_data[i] * ((i + 9) % 23);

    }



    for (i = 0; i < 32; i++)

        sum += (r->date[i] + i) * (r->name[i] + i) * (17 + i * 17);



    for (i = 0; i < 42; i++)

        sum += (r->comment[i] + i) * (r->comment[i] + i) * (-3 + i * 3);



    for (i = 0; i < r->size; i++)

        sum += r->data[i].cycle_count * 7 * (i % 167 + 1) + r->data[i].key_flags * 3 * (i % 193 + 1);

    return hash(sum);

}

int get_replay_property(const char *filename, int property)
{
    void *pf;
    Treplay r_temp;
    Treplay *r;
    int i;
    int retval;

    pf = pack_fopen(filename, "rb");
    if (!pf) {
        log2file("Couldn't open %s", filename);
        return 0;
    }
    pack_fread(r_temp.header, 6, pf);
    pack_fread(&r_temp.size, 4, pf);
    pack_fclose(pf);
    if (memcmp(r_temp.header, REPLAY_HEADER, 3)) {
        log2file("%s has wrong first 3 bytes of header", filename);
        return -1000;
    }
    if (r_temp.header[3] != '1' || r_temp.header[4] != '4' ||
        r_temp.header[5] != '0') {
        log2file("%s has wrong header version", filename);
        if (r_temp.header[3] == '0' && r_temp.header[4] == '0' &&
            r_temp.header[5] == '1')
            return -1001;
        if (r_temp.header[3] == '1' && r_temp.header[4] == '3' &&
            r_temp.header[5] == '0')
            return -1130;
        return -1000;
    }
    r = create_replay(r_temp.size);
    if (!r) {
        log2file("Couldn't create a replay object");
        return -1;
    }
    pf = pack_fopen(filename, "rb");
    if (!pf) {
        log2file("Can't open %s", filename);
        destroy_replay(r);
        return -1;
    }
    pack_fread(r->header, 6, pf);
    pack_fread(&r->size, 4, pf);
    pack_fread(r->name, 32, pf);
    pack_fread(r->date, 32, pf);
    pack_fread(&r->score, 4, pf);
    pack_fread(&r->floor, 4, pf);
    pack_fread(&r->combo, 4, pf);
    pack_fread(&r->no_combo_top_floor, 4, pf);
    pack_fread(&r->biggest_lost_combo, 4, pf);
    for (i = 0; i < 5; i++)
        pack_fread(&r->ccc[i], 4, pf);
    for (i = 0; i < 5; i++)
        pack_fread(&r->jc[i], 4, pf);
    pack_fread(&r->floor_shrink, 4, pf);
    pack_fread(&r->floor_size, 4, pf);
    pack_fread(&r->start_speed, 4, pf);
    pack_fread(&r->speed_increase, 4, pf);
    pack_fread(&r->gravity, 4, pf);
    pack_fclose(pf);
    retval = 0;
    switch (property) {
    case 2:
        retval = r->score;
        log2file("%s:score=%d", filename, retval);
        break;
    case 4:
        retval = r->floor;
        log2file("%s:floor=%d", filename, retval);
        break;
    case 3:
        retval = r->combo;
        log2file("%s:combo=%d", filename, retval);
        break;
    }
    destroy_replay(r);
    return retval;
}

Treplay *load_replay(const char *filename)
{
    PACKFILE *pf;
    Treplay r_temp;
    Treplay *r;
    int i;
    int sum;
    int cs;

    pf = pack_fopen(filename, "rb");
    if (!pf)
        return 0;
    pack_fread(r_temp.header, 6, pf);
    pack_fread(&r_temp.size, 4, pf);
    pack_fclose(pf);
    if (memcmp(r_temp.header, REPLAY_HEADER, 6))
        return 0;
    r = create_replay(r_temp.size);
    if (!r)
        return 0;
    pf = pack_fopen(filename, "rb");
    if (!pf)
        goto error;
    pack_fread(r->header, 6, pf);
    pack_fread(&r->size, 4, pf);
    pack_fread(r->name, 32, pf);
    pack_fread(r->date, 32, pf);
    pack_fread(&r->score, 4, pf);
    pack_fread(&r->floor, 4, pf);
    pack_fread(&r->combo, 4, pf);
    pack_fread(&r->no_combo_top_floor, 4, pf);
    pack_fread(&r->biggest_lost_combo, 4, pf);
    for (i = 0; i < 5; i++)
        pack_fread(&r->ccc[i], 4, pf);
    for (i = 0; i < 5; i++)
        pack_fread(&r->jc[i], 4, pf);
    pack_fread(&r->floor_shrink, 4, pf);
    pack_fread(&r->floor_size, 4, pf);
    pack_fread(&r->start_speed, 4, pf);
    pack_fread(&r->speed_increase, 4, pf);
    pack_fread(&r->gravity, 4, pf);
    pack_fread(&r->rejump, 4, pf);
    pack_fread(&r->random_seed, 4, pf);
    pack_fread(r->comment, 42, pf);
    pack_fread(&r->checksum, 4, pf);
    pack_fread(&r->tc_posts, 4, pf);
    for (i = 0; i < 100; i++) {
        pack_fread(&r->tc_c_data[i], 4, pf);
        pack_fread(&r->tc_q_data[i], 4, pf);
        pack_fread(&r->tc_t_data[i], 4, pf);
        pack_fread(&r->tc_s_data[i], 4, pf);
        pack_fread(&r->tc_f_data[i], 4, pf);
    }
    for (i = 0; i < r->size; i++) {
        pack_fread(&r->data[i].cycle_count, 4, pf);
        pack_fread(&r->data[i].key_flags, 1, pf);
    }
    pack_fclose(pf);
    cs = r->checksum;
    r->checksum = 0;
    sum = calc_replay_checksum(r);
    if (cs == sum)
        return r;
    log2file("Checksum failed for %s: got %d, expected %d", filename, sum, cs);
error:
    destroy_replay(r);
    return 0;
}

int save_replay(const char *path, const char *file, Treplay *r, int size, int make_new_date)
{
    void *pf;
    int i;
    time_t my_time;
    struct tm *timer;
    char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char filename[2048];
    int cs;

    sprintf(filename, "%s%s", path, file);
    log2file("  saving replay: %s", filename);
    if (make_new_date) {
        my_time = time(0);
        timer = localtime(&my_time);
        strcpy(r->date, "              ICYTOWERISGREAT ");
        sprintf(r->date, "%2d %3s %4d", timer->tm_mday,
                months[timer->tm_mon], timer->tm_year + 1900);
    } else {
        Treplay *tmp;

        tmp = load_replay(filename);
        if (tmp)
            strcpy(r->date, tmp->date);
    }
    r->size = size;
    cs = calc_replay_checksum(r);
    r->checksum = cs;
    pf = pack_fopen(filename, "wb");
    if (!pf)
        return -1;
    pack_fwrite(r->header, 6, pf);
    pack_fwrite(&r->size, 4, pf);
    pack_fwrite(r->name, 32, pf);
    pack_fwrite(r->date, 32, pf);
    pack_fwrite(&r->score, 4, pf);
    pack_fwrite(&r->floor, 4, pf);
    pack_fwrite(&r->combo, 4, pf);
    pack_fwrite(&r->no_combo_top_floor, 4, pf);
    pack_fwrite(&r->biggest_lost_combo, 4, pf);
    for (i = 0; i < 5; i++)
        pack_fwrite(&r->ccc[i], 4, pf);
    for (i = 0; i < 5; i++)
        pack_fwrite(&r->jc[i], 4, pf);
    pack_fwrite(&r->floor_shrink, 4, pf);
    pack_fwrite(&r->floor_size, 4, pf);
    pack_fwrite(&r->start_speed, 4, pf);
    pack_fwrite(&r->speed_increase, 4, pf);
    pack_fwrite(&r->gravity, 4, pf);
    pack_fwrite(&r->rejump, 4, pf);
    pack_fwrite(&r->random_seed, 4, pf);
    pack_fwrite(r->comment, 42, pf);
    pack_fwrite(&r->checksum, 4, pf);
    pack_fwrite(&r->tc_posts, 4, pf);
    for (i = 0; i < 100; i++) {
        pack_fwrite(&r->tc_c_data[i], 4, pf);
        pack_fwrite(&r->tc_q_data[i], 4, pf);
        pack_fwrite(&r->tc_t_data[i], 4, pf);
        pack_fwrite(&r->tc_s_data[i], 4, pf);
        pack_fwrite(&r->tc_f_data[i], 4, pf);
    }
    for (i = 0; i < r->size; i++) {
        pack_fwrite(&r->data[i].cycle_count, 4, pf);
        pack_fwrite(&r->data[i].key_flags, 1, pf);
    }
    pack_fclose(pf);
    return 0;
}

/* Recovered from replay.c:460, 0x41be58..0x41cce6.  The oracle's inline
 * Allegro drawing expands more heavily than this source form, but all list,
 * detail, clipping, version, and custom-replay presentation paths remain in
 * the owning replay CU. */
void draw_replay_selector(BITMAP *bmp, Treplay *rep, Treplay_post *file_list,
                          int selection, int offset, int max_posts, int x, int y)
{

    int w = 310;

    int h = 305;

    int fh = text_height(font);

    int i;

    char *curr_filename = NULL;

    int fg = makecol(25, 25, 25);

    int mg = makecol(85, 85, 85);

    double view_percentage;

    double view_offset;

    int show_directory = 0;

    int selected_version = 0;

    int isCustom = 0;



    i = num_itr_files;

    view_percentage = (double)max_posts / i;

    if (view_percentage > 1.0)

        view_percentage = 1.0;



    draw_sprite(bmp, data[86].dat, x - 15, y - 15);

    set_trans_blender(0, 0, 0, 150);

    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

    rect(bmp, x + 5, y + 30, x + 305, y + 300, fg);

    rect(bmp, x + 295, y + 30, x + 305, y + 300, fg);

    rectfill(bmp, x + 297, y + 32 + (1.0 - view_percentage) * 266.0 / (i - max_posts) * offset, x + 303, y + 32 + 266.0 * view_percentage + (1.0 - view_percentage) * 266.0 / (i - max_posts) * offset, fg);

    w = 400;

    h = 400;

    rect(bmp, x + 5, y + 310, x + w - 6, y + h - 6, fg);

    solid_mode();



    textout_ex(bmp, data[51].dat, "SELECT REPLAY", x + 10, y - 12, -1, -1);

    draw_sprite(bmp, data[89].dat, x + 310, y + 34);

    draw_sprite(bmp, data[112].dat, x + 311, y + 260);

    switch (sort_method) {

    case 1:

        draw_sprite(bmp, data[115].dat, x + 321, y + 274);

        break;

    case 4:

        draw_sprite(bmp, data[114].dat, x + 321, y + 286);

        break;

    case 2:

        draw_sprite(bmp, data[116].dat, x + 355, y + 274);

        break;

    case 3:

        draw_sprite(bmp, data[113].dat, x + 355, y + 286);

        break;

    }



    set_clip_rect(bmp, x + 6, 0, x + 290, bmp->h - 1);

    for (i = 0; i < max_posts; i++) {

        char name[1024];

        int is_dir;



        if (i + offset >= num_itr_files)

            break;



        if (file_list[i + offset].parent)

            strcpy(name, ".. (parent directory)");

        else

            strcpy(name, get_filename(file_list[i + offset].full_path));



        is_dir = file_list[i + offset].directory;



        if (i + offset != selection)

            textprintf_ex(bmp, font, x + 8, y + 32 + i * fh, is_dir ? mg : fg, -1, "  %c %s", is_dir ? '}' : '{', name);

        else {

            set_trans_blender(0, 0, 0, 50);

            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

            rectfill(bmp, x + 7, y + 32 + i * fh, x + 293, y + 32 + (i + 1) * fh - 1, fg);

            solid_mode();

            textprintf_ex(bmp, font, x + 8, y + 32 + i * fh, mg, -1, "> %c %s", is_dir ? '}' : '{', name);

            curr_filename = get_filename(file_list[i + offset].full_path);

            show_directory = is_dir;

            selected_version = file_list[i + offset].version;

        }

    }

    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);



    if (rep)

        isCustom = is_custom_replay(rep);

    set_clip_rect(bmp, x + 6, 0, x + w - 10 - (isCustom ? text_length(font, "CUSTOM GAME  ") : 0), bmp->h - 1);

    textprintf_ex(bmp, font, x + 10, y + 315, mg, -1, "%s", curr_filename ? curr_filename : "n/a");

    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);



    textout_ex(bmp, font, "NAME", x + 10, y + 335, fg, -1);

    textout_right_ex(bmp, font, "SCORE", x + 180, y + 335, fg, -1);

    textout_right_ex(bmp, font, "FLOOR", x + 230, y + 335, fg, -1);

    textout_right_ex(bmp, font, "COMBO", x + 280, y + 335, fg, -1);

    textout_ex(bmp, font, "DATE ", x + 300, y + 335, fg, -1);



    if (rep) {

        set_clip_rect(bmp, x + 9, 0, x + 133, bmp->h - 1);

        textprintf_ex(bmp, font, x + 10, y + 350, fg, -1, "%s", rep->name);

        set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

        textprintf_right_ex(bmp, font, x + 180, y + 350, fg, -1, "%6d", rep->score);

        textprintf_right_ex(bmp, font, x + 230, y + 350, fg, -1, "%4d", rep->floor);

        textprintf_right_ex(bmp, font, x + 280, y + 350, fg, -1, "%3d", rep->combo);

        textprintf_ex(bmp, font, x + 300, y + 350, fg, -1, "%s ", rep->date);

        if (isCustom)

            textprintf_right_ex(bmp, font, x + w - 10, y + 315, mg, -1, "%s", "CUSTOM GAME");

        if (rep->comment[0])

            textprintf_ex(bmp, font, x + 10, y + 370, mg, -1, "%s", rep->comment);

    }

    else {

        if (show_directory)

            textout_ex(bmp, font, "This is a folder. Press ENTER to open it.", x + 10, y + 350, fg, -1);

        else {

            char rbuf[129];

            switch (selected_version) {

            case 1:

                strcpy(rbuf, "You need Icy Tower 1.2 to view this replay.");

                break;

            case 0x82:

                strcpy(rbuf, "You need Icy Tower 1.3 to view this replay.");

                break;

            default:

                strcpy(rbuf, "Replay is broken.");

            }

            textout_ex(bmp, font, rbuf, x + 10, y + 350, makecol(80, 20, 20), -1);

        }

    }

}

int add_itr_file(const char *filename, int attrib, void *param)
{

    int length = strlen(filename) + 10;

    char *name = get_filename(filename);

    int res;



    if (!stricmp(name, "."))

        return 0;

    if (!(attrib & FA_DIREC) && stricmp(get_extension(filename), "itr"))

        return 0;

    itr_file_list[num_itr_files].full_path = malloc(length);

    if (attrib & FA_DIREC) {

            strcpy(itr_file_list[num_itr_files].full_path, filename);

            itr_file_list[num_itr_files].directory = 1;

            if (!strcmp(name, ".."))

                itr_file_list[num_itr_files].parent = 1;

    }

    else {

            res = get_replay_property(filename, 0);

            if (res < 0) {

                if (res == -1 || res == -1000)

                    return 0;

                itr_file_list[num_itr_files].version = -1000 - res;

            }

            strcpy(itr_file_list[num_itr_files].full_path, filename);

            itr_file_list[num_itr_files].directory = 0;

    }

    num_itr_files++;

    return 0;

}

int my_strcmp(const void *c, const void *d)
{
    Treplay_post *a;
    Treplay_post *b;
    int av, bv;

    a = (Treplay_post *)c;
    b = (Treplay_post *)d;
    if (a->directory != b->directory) {
        if (a->directory)
            return -1;
        return 1;
    }
    /* The original forms this as an unsigned interval: modes 2 through 4
     * select a replay property, while every other value sorts by pathname. */
    if (!a->directory && (unsigned int)(sort_method - 2) <= 2U) {
        av = get_replay_property(a->full_path, sort_method);
        bv = get_replay_property(b->full_path, sort_method);
        if (av > bv)
            return -1;
        return 1;
    }
    return stricmp(a->full_path, b->full_path);
}

void update_file_list(char *path)
{
    int i;
    char full_path[1024];

    for (i = 0; i < num_itr_files; i++) {
        free(itr_file_list[i].full_path);
        itr_file_list[i].parent = 0;
        itr_file_list[i].directory = 0;
    }
    num_itr_files = 0;
    sprintf(full_path, "%s/*", path);
    for_each_file_ex(full_path, 0, 0, add_itr_file, 0);
    qsort(itr_file_list, num_itr_files, sizeof(Treplay_post), my_strcmp);
}

/* Recovered from replay.c:701, 0x41d258..0x41dd75.  This keeps the complete
 * selector lifecycle in source: file-list rebuilding, preview ownership,
 * keyboard/controller dispatch, directory/file selection, animated display,
 * and cleanup. */
Treplay *replay_selector(Tcontrol *ctrl, char *path)
{
    Treplay *rep = NULL;
    int curr_file_id = 0;
    int kp;
    int done = 0;
    int i;
    int ctrl_wait = 1000;
    int page_size;
    int offset = 0;
    FONT *old_font = font;
    int need_to_update = 1;
    int ok_to_rename;
    int pageY = 500;
    int targetY;
    BITMAP *bg;

    bg = create_bitmap(SCREEN_W, SCREEN_H);
    blit(screen, bg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    font = data[54].dat;
    page_size = 270 / text_height(font);
    clear_keybuf();

    while (!closeButtonClicked && !done) {
        cycle_count = 0;

        checkMenuFocus();

        if (need_to_update)
            update_file_list(path);

        need_to_update = 0;
        if (curr_file_id > num_itr_files) {
            need_to_update = 1;
            offset = 0;
            curr_file_id = 0;
        }

        if (rep)
            destroy_replay(rep);
        if (num_itr_files > 0)
            rep = load_replay(itr_file_list[curr_file_id].full_path);

        poll_control(ctrl, 1);
        if (is_any(ctrl) && !ctrl_wait) {
            if (is_down(ctrl)) simulate_keypress(KEY_DOWN << 8);
            if (is_up(ctrl)) simulate_keypress(KEY_UP << 8);
            if (is_fire(ctrl)) simulate_keypress(KEY_ENTER << 8);
            ctrl_wait = 20;
        }
        if (!is_any(ctrl)) ctrl_wait = 0;
        if (ctrl_wait > 0) ctrl_wait--;

        if (keypressed()) {
            kp = readkey();
            switch (kp >> 8) {
            case KEY_N:
                sort_method = 1;
                need_to_update = 1;
                break;
            case KEY_S:
                sort_method = 2;
                need_to_update = 1;
                break;
            case KEY_F:
                sort_method = 4;
                need_to_update = 1;
                break;
            case KEY_C:
                sort_method = 3;
                need_to_update = 1;
                break;
            case KEY_F1: {
                char p[1024];
                play_menu_select();
                if (rep)
                    destroy_replay(rep);
                gui_bg_color = makecol(255, 255, 255);
                gui_fg_color = makecol(0, 0, 0);
                install_mouse();
                file_select_ex("Select a new folder and press OK.", path, "itr", 1024, 400, 400);
                remove_mouse();
                replace_filename(p, path, "", sizeof(p));
                strcpy(path, p);
                need_to_update = 1;
                offset = 0;
                curr_file_id = 0;
                rep = NULL;
                break;
            }
            case KEY_F2:
                break;
            case KEY_UP:
                if (curr_file_id > 0) {
                    curr_file_id--;
                    if (curr_file_id < offset)
                        offset--;
                    play_menu_move();
                }
                else {
                    curr_file_id = 0;
                    offset = 0;
                }
                break;
            case KEY_DOWN:
                if (curr_file_id < num_itr_files - 1) {
                    curr_file_id++;
                    if (curr_file_id >= offset + page_size)
                        offset++;
                    play_menu_move();
                }
                else {
                    curr_file_id = num_itr_files - 1;
                    offset = num_itr_files - page_size;
                    if (offset < 0) offset = 0;
                }
                break;
            case KEY_ESC:
                if (rep)
                    destroy_replay(rep);
                play_menu_select();
                done = -1;
                rep = NULL;
                break;
            case KEY_ENTER:
            case KEY_SPACE:
                if (itr_file_list[curr_file_id].directory) {
                    char p[1024];
                    strcpy(path, itr_file_list[curr_file_id].full_path);
                    canonicalize_filename(p, path, sizeof(p));
                    strcpy(path, p);
                    offset = 0;
                    curr_file_id = 0;
                }
                else if (rep)
                    done = 1;
                play_menu_select();
                need_to_update = 1;
                break;
            case KEY_DEL:
                if (rep) {
                    char fname[512];
                    play_menu_select();
                    strcpy(fname, "Really delete replay?");
                    if (my_alert(fname, "WARNING: It will be gone forever.", 1, 0))
                        delete_file(itr_file_list[curr_file_id].full_path);
                    need_to_update = 1;
                }
                break;
            }
        }

        pageY = (int)(0.2 * (25 - pageY) + pageY);
        blit(bg, swap_screen, 0, 0, 0, 0, 640, 480);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_replay_selector(swap_screen, rep, itr_file_list, curr_file_id, offset, page_size, 120, pageY);
        blit_to_screen(swap_screen);
        while (cycle_count <= 0)
            rest(2);
    }

    if (!rep) {
        while (pageY <= 499) {
            cycle_count = 0;
            pageY = (int)(0.2 * (510 - pageY) + pageY);
            blit(bg, swap_screen, 0, 0, 0, 0, 640, 480);
            set_trans_blender(0, 0, 0, (500 - pageY) / 3);
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
            solid_mode();
            draw_replay_selector(swap_screen, NULL, itr_file_list, curr_file_id, offset, page_size, 120, pageY);
            blit_to_screen(swap_screen);
            while (cycle_count <= 0)
                rest(2);
        }
    }

    destroy_bitmap(bg);
    for (i = 0; i < num_itr_files; i++) {
        free(itr_file_list[i].full_path);
        itr_file_list[i].parent = 0;
        itr_file_list[i].directory = 0;
    }
    num_itr_files = 0;
    font = old_font;
    return rep;
}

