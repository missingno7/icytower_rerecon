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

int sort_method;
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
int num_itr_files;
static char replay_header[] = "ITR140";

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
    memcpy(r->header, replay_header, 6);
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
        sum += (r->data[i].key_flags * 5 + r->data[i].cycle_count * 3) * i;
    return sum;
}

int calc_replay_checksum(Treplay *r)
{
    int i;
    unsigned int sum;

    sum = (r->biggest_lost_combo * 17 + r->no_combo_top_floor * 127 + 17) * 2;
    sum += r->floor_shrink * 102 + r->floor_size * 17 + 3702;
    sum += r->start_speed * 163 + r->speed_increase * 23;
    sum += r->gravity * 88 + r->random_seed * 329;
    sum += r->tc_posts * 127 + r->rejump * 13;
    sum += r->score * 17 + 17;
    sum += (r->combo + 1) * 649;
    sum += (r->floor + 1) * 113;
    for (i = 0; i < 5; i++)
        sum += r->ccc[i] * (39 + i * 3) + r->jc[i] * (27 + i * 3);
    for (i = 0; i < 100; i++) {
        sum += r->tc_c_data[i] * ((i + 1) % 13);
        sum += r->tc_q_data[i] * ((i + 6) % 17);
        sum += r->tc_t_data[i] * ((i + 8) % 23);
    }
    for (i = 0; i < 32; i++)
        sum += (r->date[i] + i) * (r->name[i] + i) * (17 + i * 17);
    for (i = 0; i < 42; i++)
        sum += (r->comment[i] + i) * (r->comment[i] + i) * (-3 + i * 3);
    for (i = 0; i < r->size; i++)
        sum += r->data[i].key_flags * 3 * (i % 193 + 1) +
               r->data[i].cycle_count * 7 * (i % 167 + 1);
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
    if (memcmp(r_temp.header, "ITR140", 3)) {
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
    case 3:
        retval = r->combo;
        log2file("%s:combo=%d", filename, retval);
        break;
    case 4:
        retval = r->floor;
        log2file("%s:floor=%d", filename, retval);
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
    if (memcmp(r_temp.header, "ITR140", 6))
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
    /* 462 */
    int fh = text_height(font);
    int i;
    /* 465 */
    int fg = makecol(25, 25, 25);
    /* 466 */
    int mg = makecol(85, 85, 85);
    /* 467 */
    float view_percentage = max_posts ? (float)num_itr_files / max_posts : 1.0f;
    float view_offset = max_posts ? (float)offset / max_posts : 0.0f;
    /* Original DWARF identifies curr_filename as char * at EBP-0x448.
     * The selected-row path stores get_filename(post->full_path) there after
     * rendering the row. */
    char *curr_filename;
    int is_dir;
    int show_directory = 0;
    int selected_version = 0;
    int isCustom = 0;

    /* 467 */
    if (view_percentage > 1.0f)
        view_percentage = 1.0f;
    /* 473 */
    if (data && data[86].dat)
        stretch_sprite(bmp, data[86].dat, x - 15, y - 15, w + 30, h + 30);
    else
        rectfill(bmp, x - 15, y - 15, x + w + 15, y + h + 15, mg);

    /* 476 */
    set_trans_blender(0, 0, 0, 150);
    /* 477 */
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    /* 479 */
    rect(bmp, x + 5, y + 30, x + w - 5, y + 305, fg);
    /* 479 */
    rect(bmp, x + 7, y + 32, x + w - 7, y + 303, fg);
    /* 487 */
    solid_mode();

    /* 490: textout_ex(bmp, data[51].dat, "SELECT REPLAY", x+10, y-12, -1, -1) --
     * slots 0=bmp,4=data[51].dat (0x330/0x10=51),8=text,0xc=x+10,0x10=y-12,
     * 0x14=-1,0x18=-1, all read directly off the movl/mov-before-call chain. */
    textout_ex(bmp, data[51].dat, "SELECT REPLAY", x + 10, y - 12, -1, -1);

    draw_sprite(bmp, data[89].dat, x + 0x136, y + 0x22);
    draw_sprite(bmp, data[112].dat, x + 0x137, y + 0x104);
    switch (sort_method) {
    case 1:
        draw_sprite(bmp, data[115].dat, x + 0x112, y + 0x141);
        break;
    case 2:
        draw_sprite(bmp, data[116].dat, x + 0x112, y + 0x163);
        break;
    case 3:
        draw_sprite(bmp, data[113].dat, x + 0x11e, y + 0x163);
        break;
    case 4:
        draw_sprite(bmp, data[114].dat, x + 0x11e, y + 0x141);
        break;
    }

    /* 509: set_clip_rect(bmp,x+6,0,x+0x122,bmp->h-1) -- the current reconstruction's
     * (x+10,y+35,x+185,y+300) does not match a single one of these four operands. */
    set_clip_rect(bmp, x + 6, 0, x + 290, bmp->h - 1);
    for (i = offset; i < num_itr_files && i < offset + max_posts; i++) {
        char name[1024];
        /* 516: Treplay_post *post = &file_list[i]; cmpb $0,0x5(edx) tests offset 5,
         * which is Treplay_post.parent (src/replay.c:30), not .directory (offset 4). */
        Treplay_post *post = &file_list[i];
        int row = y + 40 + (i - offset) * fh;
        char marker;

        if (post->parent) {
            /* 517: rep movsb copying the literal ".. (parent directory)" (22 bytes)
             * into the buffer later reused as name (same -0x418(%ebp) slot as the
             * warning messages at 591/594/597 below). */
            strcpy(name, ".. (parent directory)");
        } else {
            /* 519: strcpy(name, get_filename(post->full_path)) -- a real buffer,
             * not a bare `char *name` as the current source has it. */
            strcpy(name, get_filename(post->full_path));
        }
        /* 521: is_dir = post->directory (movsbl 0x4(%edx),%esi) -- the named-but-
         * missing local; offset 4, confirmed against the Treplay_post typedef. */
        is_dir = post->directory;
        marker = is_dir ? '}' : '{';
        if (i == selection) {
            /* Original offsets 0x568..0x5ec: alpha-50 mode, the inlined
             * rectfill(vtable+0x3c), solid mode, then selected text. */
            set_trans_blender(0, 0, 0, 50);
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            rectfill(bmp, x + 7, row, x + 0x125, row + fh - 9, fg);
            solid_mode();
            textprintf_ex(bmp, font, x + 8, row, mg, -1, "> %c %s", marker, name);
        } else if (is_dir) {
            /* 525 (predecessor A): color mg when is_dir. */
            textprintf_ex(bmp, font, x + 8, row, mg, -1, "  %c %s", marker, name);
        } else {
            /* 525 (predecessor B): color fg when !is_dir. */
            textprintf_ex(bmp, font, x + 8, row, fg, -1, "  %c %s", marker, name);
        }
        if (i == selection) {
            curr_filename = get_filename(post->full_path);
            show_directory = post->directory;
            selected_version = post->version;
        }
    }
    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

    /* 486/487 (bar): unchanged from the existing reconstruction -- not
     * re-verified against fresh instruction evidence this pass. */
    if (num_itr_files > max_posts && max_posts > 0) {
        int bar_top = y + 40 + (int)(view_offset * 260.0f);
        int bar_height = (int)(view_percentage * 260.0f);
        rectfill(bmp, x + 190, bar_top, x + 196, bar_top + bar_height, fg);
    }

    /* 546 */
    if (rep) {
        /* 547 */
        isCustom = is_custom_replay(rep);
        /* 551: set_clip_rect(bmp,x+6,0,x+390-(isCustom?text_length(font,"CUSTOM GAME  "):0),bmp->h-1) --
         * x+390 from x+0x190 (established for the scrollbar/table region) minus 10;
         * two historical predecessors (one measuring the label, one using 0) merge
         * into this one call. */
        set_clip_rect(bmp, x + 6, 0,
                      x + 390 - (isCustom ? text_length(font, "CUSTOM GAME  ") : 0),
                      bmp->h - 1);
        /* 552: rep->comment falls back to the literal "n/a" when falsy; fg,
         * y+315, x+10. */
        textprintf_ex(bmp, font, x + 10, y + 315, fg, -1, "%s",
                      rep->comment[0] ? rep->comment : "n/a");

        /* 562: a second, explicit `if (rep)` gates the results table below --
         * confirmed by its own independent null test on the rep parameter. */
        if (rep) {
            /* 563: a narrow clip around the NAME column, x+9..x+133. */
            set_clip_rect(bmp, x + 9, 0, x + 133, bmp->h - 1);
            /* 564: textprintf_ex(bmp,font,x+10,y+350,fg,-1,"%s",rep->name) --
             * rep+0xc is rep->name (offsetof(Treplay,name)==12, confirmed
             * against include/recovered/Treplay.h). */
            textprintf_ex(bmp, font, x + 10, y + 350, fg, -1, "%s", rep->name);
            /* 565: full clip reset before the unclipped table. */
            set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);
            /* 555..559: NAME/SCORE/FLOOR/COMBO/DATE column headers, all row y+335. */
            textout_ex(bmp, font, "NAME", x + 10, y + 335, fg, -1);
            textout_right_ex(bmp, font, "SCORE", x + 180, y + 335, fg, -1);
            textout_right_ex(bmp, font, "FLOOR", x + 230, y + 335, fg, -1);
            textout_right_ex(bmp, font, "COMBO", x + 280, y + 335, fg, -1);
            textout_ex(bmp, font, "DATE ", x + 300, y + 335, fg, -1);
            /* 566..569: the value row for `rep` itself, aligned under the headers
             * above, row y+350 (offsets 0x50/0x54/0x58/0x2c confirmed against
             * offsetof(Treplay,score/floor/combo/date) == 80/84/88/44). */
            textprintf_right_ex(bmp, font, x + 180, y + 350, fg, -1, "%6d", rep->score);
            textprintf_right_ex(bmp, font, x + 230, y + 350, fg, -1, "%4d", rep->floor);
            textprintf_right_ex(bmp, font, x + 280, y + 350, fg, -1, "%3d", rep->combo);
            textprintf_ex(bmp, font, x + 300, y + 350, fg, -1, "%s ", rep->date);
            if (isCustom) {
                /* 572/573: right-aligned "CUSTOM GAME" at the same narrowed clip
                 * edge computed at 551, color mg, row y+315 (the comment row's y). */
                textprintf_right_ex(bmp, font,
                                    x + 390 - text_length(font, "CUSTOM GAME  "),
                                    y + 315, mg, -1, "%s", "CUSTOM GAME");
            }
            if (rep->comment[0]) {
                /* 576/577: a second comment display, distinct from 552's (mg not
                 * fg, y+370 not y+315, no "n/a" fallback). */
                textprintf_ex(bmp, font, x + 10, y + 370, mg, -1, "%s", rep->comment);
            }
        }
    }

    /* 583: show_directory gates the folder hint; its own set-site inside the
     * loop was not located this pass (flagged, not guessed). selected_version's
     * set-site (presumably file_list[selection].version, matching the prior
     * reconstruction's guess) was likewise not re-confirmed against fresh
     * instruction evidence this pass -- flagged, not guessed. */
    if (show_directory) {
        /* 585 */
        textout_ex(bmp, font, "This is a folder. Press ENTER to open it.",
                  x + 10, y + 350, fg, -1);
    } else {
        char rbuf[129];
        /* 589/591/594/597: selected_version selects one of three messages into
         * rbuf (a third reuse of the same buffer as the loop rows and the
         * ".. (parent directory)" copy), then 599 prints it in a shared call. */
        if (selected_version == 1)
            strcpy(rbuf, "You need Icy Tower 1.2 to view this replay.");
        else if (selected_version == 0x82)
            strcpy(rbuf, "You need Icy Tower 1.3 to view this replay.");
        else
            strcpy(rbuf, "Replay is broken.");
        /* 599: makecol(0x50,0x14,0x14) = makecol(80,20,20), a color not used
         * anywhere else in this function. */
        textout_ex(bmp, font, rbuf, x + 10, y + 350, makecol(80, 20, 20), -1);
    }
}

int add_itr_file(const char *filename, int attrib, void *param)
{
    int length;
    char *name;
    int res;

    length = strlen(filename) + 10;
    name = get_filename(filename);
    if (!stricmp(name, "."))
        goto done;
    if (!(attrib & FA_DIREC))
        goto replay_file;
    goto directory;
replay_file:
    if (stricmp(get_extension(filename), "itr"))
        goto done;
    itr_file_list[num_itr_files].full_path = malloc(length);
    res = get_replay_property(filename, 0);
    if (res < 0)
        goto bad_replay;
copy_replay:
    strcpy(itr_file_list[num_itr_files].full_path, filename);
    itr_file_list[num_itr_files].directory = 0;
    num_itr_files++;
done:
    return 0;
directory:
    itr_file_list[num_itr_files].full_path = malloc(length);
    strcpy(itr_file_list[num_itr_files].full_path, filename);
    itr_file_list[num_itr_files].directory = 1;
    if (!strncmp(name, "..", 3))
        itr_file_list[num_itr_files].parent = 1;
    num_itr_files++;
    goto done;
bad_replay:
    if (res == -1 || res == -1000)
        goto done;
    itr_file_list[num_itr_files].version = -1000 - res;
    goto copy_replay;
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
    FONT *old_font = font;
    BITMAP *bg = create_bitmap(SCREEN_W, SCREEN_H);
    Treplay *rep = NULL;
    int curr_file_id = 0;
    int done = 0;
    int ctrl_wait = 1000;
    int page_size;
    int offset = 0;
    int need_to_update = 1;
    int ok_to_rename = 0;
    int pageY = 500;
    int targetY = 0;
    char fname[512];

    if (!bg)
        return NULL;
    blit(screen, bg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    if (data)
        font = data[54].dat;
    page_size = 270 / text_height(font);
    clear_keybuf();

    while (!done) {
        int kp = 0;

        if (need_to_update) {
            if (rep) {
                destroy_replay(rep);
                rep = NULL;
            }
            if (num_itr_files > 0 && curr_file_id >= 0 &&
                curr_file_id < num_itr_files && !itr_file_list[curr_file_id].directory)
                rep = load_replay(itr_file_list[curr_file_id].full_path);
            need_to_update = 0;
        }

        poll_control(ctrl, 1);
        if (is_any(ctrl) && ctrl_wait == 0) {
            if (is_down(ctrl))
                simulate_keypress(KEY_DOWN << 8);
            else if (is_up(ctrl))
                simulate_keypress(KEY_UP << 8);
            else if (is_fire(ctrl))
                simulate_keypress(KEY_ENTER << 8);
        }
        if (ctrl_wait > 0)
            ctrl_wait--;

        if (keypressed())
            kp = readkey() >> 8;
        if (kp) {
            ctrl_wait = 20;
            switch (kp) {
            case KEY_C:
                set_sort_method(3);
                need_to_update = 1;
                break;
            case KEY_F:
                set_sort_method(4);
                need_to_update = 1;
                break;
            case KEY_N:
                set_sort_method(1);
                need_to_update = 1;
                break;
            case KEY_S:
                set_sort_method(2);
                need_to_update = 1;
                break;
            case KEY_UP:
                if (curr_file_id > 0) {
                    curr_file_id--;
                    if (curr_file_id < offset)
                        offset--;
                    play_menu_move();
                    need_to_update = 1;
                }
                break;
            case KEY_DOWN:
                if (curr_file_id + 1 < num_itr_files) {
                    curr_file_id++;
                    if (curr_file_id >= offset + page_size)
                        offset++;
                    play_menu_move();
                    need_to_update = 1;
                }
                break;
            case KEY_ENTER:
                if (curr_file_id >= 0 && curr_file_id < num_itr_files) {
                    Treplay_post *post = &itr_file_list[curr_file_id];
                    if (post->directory) {
                        strcpy(path, post->full_path);
                        canonicalize_filename(fname, path, sizeof(fname));
                        strcpy(path, fname);
                        update_file_list(path);
                        curr_file_id = offset = 0;
                        need_to_update = 1;
                    } else if (rep) {
                        play_menu_select();
                        done = 1;
                    }
                }
                break;
            case KEY_DEL:
                if (curr_file_id >= 0 && curr_file_id < num_itr_files &&
                    !itr_file_list[curr_file_id].directory && rep &&
                    my_alert("Really delete replay?", "WARNING: It will be gone forever.",
                             1, 0)) {
                    delete_file(itr_file_list[curr_file_id].full_path);
                    need_to_update = 1;
                }
                break;
            case KEY_ESC:
                play_menu_select();
                destroy_replay(rep);
                rep = NULL;
                done = -1;
                break;
            case KEY_F1:
                set_sort_method(1);
                need_to_update = 1;
                break;
            case KEY_F2:
                set_sort_method(2);
                need_to_update = 1;
                break;
            case KEY_F3:
                set_sort_method(3);
                need_to_update = 1;
                break;
            case KEY_F4:
                set_sort_method(4);
                need_to_update = 1;
                break;
            case KEY_F5:
                install_mouse();
                file_select_ex("Select a new folder and press OK", path, "itr",
                               sizeof(fname), 400, 400);
                remove_mouse();
                replace_filename(fname, path, "", sizeof(fname));
                strcpy(path, fname);
                update_file_list(path);
                curr_file_id = offset = 0;
                need_to_update = 1;
                ok_to_rename = 1;
                break;
            default:
                break;
            }
        }

        if (need_to_update) {
            update_file_list(path);
            if (curr_file_id >= num_itr_files)
                curr_file_id = num_itr_files - 1;
            if (curr_file_id < 0)
                curr_file_id = 0;
            if (offset > curr_file_id)
                offset = curr_file_id;
        }

        /* 889: pageY interpolates toward the literal 25 at a fixed 0.2 rate --
         * NOT the existing `pageY -= (pageY - targetY) / 3 + 1;` formula this
         * replaces. Evidenced directly (fldl 0.2; fimul; fiadd; fistp),
         * unguarded by any `pageY > targetY` test in this fragment (the test
         * that gated the old formula was not found in this instruction
         * range and may not exist at all -- flagged, not confirmed absent). */
        pageY = (int)(0.2 * (25 - pageY) + pageY);
        /* 892: blit target is swap_screen, not screen -- confirmed by the
         * 0x4dd194 operand at every call in this whole present sequence
         * (blit, draw_replay_selector's bmp arg, blit_to_screen). */
        blit(bg, swap_screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);           /* 892 */
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);                    /* 894 */
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                            /* 895 */
        /* 896: rectfill's bottom-right corner is gfx_driver->w/->h (0x70/0x6c)
         * when a driver is installed, not the SCREEN_W/SCREEN_H constants. */
        rectfill(swap_screen, 0, 0, gfx_driver ? gfx_driver->w : 0,          /* 896 */
                 gfx_driver ? gfx_driver->h : 0, makecol(0, 0, 0));
        solid_mode();                                                        /* 897 */
        /* 899: y is pageY itself, with no "+120" added at the call site --
         * the existing reconstruction's `pageY + 120` does not match this
         * call's own operand (a bare register carrying pageY's value). */
        draw_replay_selector(swap_screen, rep, itr_file_list, curr_file_id,   /* 899 */
                             offset, page_size, 120, pageY);
        blit_to_screen(swap_screen);                                          /* 900 */
        /* 902: a wait loop distinct from the plain checkMenuFocus()/rest(2)
         * pair the current source has -- spins on cycle_count via rest(2),
         * and snapshots curr_file_id into a second local (used as the
         * *second* draw_replay_selector call's `selection` argument below)
         * before the wait. */
        if (cycle_count > 0) {
            int frozen_selection = curr_file_id;                              /* 902 */

            do {
                rest(2);                                                        /* 902 */
            } while (cycle_count <= 0);

            /* 904: skip the second pass entirely when rep is NULL. */
            if (rep) {
                /* 906/907/908: pageY interpolates toward 510 (not targetY) at
                 * a fixed 0.2 rate, only while pageY <= 499; cycle_count is
                 * reset to 0 here (distinct from the reset already implicit
                 * in the wait loop above). */
                if (pageY <= 499) {
                    cycle_count = 0;                                             /* 907 */
                    pageY = (int)(0.2 * (510 - pageY) + pageY);                    /* 908 */
                }
                /* 911..916: a second blit/blend/drawing_mode/rectfill/solid_mode
                 * pass, identical in shape to 892..897 above. */
                blit(bg, swap_screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);             /* 911 */
                set_trans_blender(0, 0, 0, (500 - pageY) / 3);                      /* 913 */
                drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                             /* 914 */
                rectfill(swap_screen, 0, 0, gfx_driver ? gfx_driver->w : 0,           /* 915 */
                         gfx_driver ? gfx_driver->h : 0, makecol(0, 0, 0));
                solid_mode();                                                        /* 916 */
                /* 918: rep forced NULL and selection is the frozen snapshot,
                 * not the live curr_file_id -- confirmed by comparing this
                 * call's argument slots against 899's directly. */
                draw_replay_selector(swap_screen, NULL, itr_file_list,               /* 918 */
                                     frozen_selection, offset, page_size, 120, pageY);
                blit_to_screen(swap_screen);                                          /* 919 */
            }
        }
        checkMenuFocus();
        rest(2);
        (void)ok_to_rename;
    }

    destroy_bitmap(bg);
    font = old_font;
    for (curr_file_id = 0; curr_file_id < num_itr_files; curr_file_id++) {
        free(itr_file_list[curr_file_id].full_path);
        itr_file_list[curr_file_id].parent = 0;
        itr_file_list[curr_file_id].directory = 0;
    }
    num_itr_files = 0;
    return rep;
}
