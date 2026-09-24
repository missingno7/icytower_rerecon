#include "recovered/Tcontrol.h"
#include "recovered/Thisc_table.h"
extern void destroy_hisc_table(Thisc_table*);
extern void update_frame(void);
extern void save_config(void);
extern void handle_player_collision_vector(int, int);
extern void handle_player_collision_original(int, int);
extern void handle_player_collision_old(int, int);
extern void handle_player_collision_combo(int, int);
#include "recovered/Tmenu_char_selection.h"
/* Partial historical main.c recovery. Other original entities remain absent. */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <direct.h>
#include <allegro.h>
#include <allegro/platform/aintwin.h>
#include <winalleg.h>
#include "loadpng.h"
#include "beta.h"
#include "control.h"
#include "custom.h"
#include "directories.h"
#include "game_services.h"
#include "timer.h"
#include "particle.h"
#include "map.h"
#include "scroller.h"
#include "recovered_types.h"

/* This exported extension belongs to the separately reconstructed logg CU. */
SAMPLE *logg_load_memory(void *pData, size_t iSize);

/* Declared at original line 92; log2file suppresses output while it is set. */
int itrcheck;
int dropped_file_is_not_a_replay;
int init_ok;
char last_log[1024];
char working_directory[1024];
#include "recovered/Tcommandline.h"
typedef Tcommandline Tcmdline;
Tcmdline cmdline;
double seed;
int hasFocus;
int closeButtonClicked;
int lastMouseB;
int window;
int lastFocus;
int in_replay_menu;
char sfx_file[512];
int collision_type;
int got_joystick;
int scroll_count;
int scroll_delay;
int gdLastJumpDiff;
int gdComboStart;
int last_stripe_y;
int new_personal_best[15];
char *hints[45];
Tbeta *testers;
Tbeta *the_tester;

#include "recovered/Trecord.h"
#include "recovered/Treplay.h"
extern int get_string(BITMAP*, char*, int, int, FONT*, int, int, int, int);
extern void drawSlot(BITMAP*, int, int, char*, char*, int);
Treplay *demo;
int uberChecksum;
Tcontrol ctrl;
extern DATAFILE *data;
DATAFILE *sfx;

void blit_to_screen(BITMAP *bmp);
void checkMenuFocus(void);

BITMAP *swap_screen;
BITMAP *poster;
int bg_stripe_ids[5];
BITMAP *gameover_bmp;

#include "recovered/Toptions.h"

#include "recovered/Tprofile.h"

#include "recovered/Tavailable_profile.h"

#include "recovered/Tmenu_slider.h"

#include "recovered/Tmenu_selection.h"

#include "recovered/Tmenu_floor_selection.h"

/* menu.h layout recovered from the main-CU DWARF inventory. */
#include "recovered/Tmenu.h"

#include "recovered/Tmenu_params.h"

typedef struct FLDAdSpot {
    char *pRemoteImageURL;
    char *pLocalImagePath;
    char *pVisitURL;
    float fFrequency;
} FLDAdSpot;
extern const FLDAdSpot *fldads_get_random_ad(void);

typedef struct Tavatar_profile {
    unsigned char reserved[0x4e4];
    char avatar[1];
} Tavatar_profile;

#include "recovered/Tcharacter.h"

#include "recovered/Tgame_data.h"

#include "recovered/Tgd_jump_sequence.h"
typedef Tgd_jump_sequence Tjump_sequence;

Toptions options;
int start_speeds[6] = { 5, 4, 3, 2, 1, 0 };
char *version_str = "1.5.1";
Tprofile *profile;
Tavailable_profile *profiles;
int numProfiles;
SAMPLE *bg_menu;
SAMPLE *menu_sounds[2];
SAMPLE *jump_sound[3];
SAMPLE *speaker[3];
SAMPLE *sounds[9];
Tcustom custom;
int reward_time;
fixed reward_scale;
BITMAP *reward_bmp;
Tparticle stars[512];
SAMPLE *combo_sound[10];
int num_chars;
Tcharacter *characters;
int curr_char;
Tmenu_char_selection play_char;
Tplayer *ply[1000];
int player_id;
int any11;
int any12;
int any13;
int any21;
int any22;
int any23;
int is_playing_custom_game;
Tmap map;
Tgame_data *gameData;
int checkMusicVoiceID;
int rejump;
Tjump_sequence jumpSequence;
int fast_forward;
int fast_fast_forward;
int gameMusicVoiceID = -1;
SAMPLE *bg_beat;
Tmenu_slider snd_volume_slider = { 0, 0, 250, 25 };
Tmenu_slider msc_volume_slider = { 0, 0, 250, 25 };
Tmenu_selection eyecandy_selection;
Tmenu_floor_selection floors;
Tmenu_selection scroll_speed_selection;
Tmenu_selection floor_size_selection;
Tmenu_selection gravity_selection;
Tmenu_params menu_params;
char replay_directory[1024];
BITMAP *pFLDAdBitmap;
const FLDAdSpot *pFLDAd;
DATAFILE *data;
int rec_pos;
int recording;
int rec_seed;
int hurry_y;
int fall_count;
int clock_angle;
int cycle_loops;
void *hisc_tables[15];
char *hisc_names[15] = {
    "Best Scores", "Best Combos", "Highest Floors", "Biggest Lost Combos",
    "Top Floors, No Combos", "Clock Challenge 1", "Clock Challenge 2",
    "Clock Challenge 3", "Clock Challenge 4", "Clock Challenge 5",
    "Single Jump Sequence", "Double Jump Sequence", "Triple Jump Sequence",
    "Quadruple Jump Sequence", "Quintuple Jump Sequence"
};
char *category_names[15] = {
    "Score", "Best Combo", "Floor", "Lost Combo", "Top Floor, No Combos",
    "Clock Challenge 1", "Clock Challenge 2", "Clock Challenge 3",
    "Clock Challenge 4", "Clock Challenge 5", "Single Jump Sequence",
    "Double Jump Sequence", "Triple Jump Sequence", "Quadruple Jump Sequence",
    "Quintuple Jump Sequence"
};
static int face;
static int count;
char scroller_greetings[156] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x57, 0x65, 0x6c, 0x63, 0x6f, 0x6d, 0x65, 0x20, 0x74, 0x6f, 0x20,
    0x49, 0x63, 0x79, 0x20, 0x54, 0x6f, 0x77, 0x65, 0x72, 0x21,
    0x20, 0x20, 0x20, 0x20, 0x20,
    0x48, 0x65, 0x6c, 0x70, 0x20, 0x48, 0x61, 0x72, 0x6f, 0x6c, 0x64,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x48, 0x6f, 0x6d, 0x65, 0x62, 0x6f,
    0x79, 0x20, 0x74, 0x6f, 0x20, 0x63, 0x6c, 0x69, 0x6d, 0x62, 0x20,
    0x61, 0x73, 0x20, 0x68, 0x69, 0x67, 0x68, 0x20, 0x61, 0x73, 0x20,
    0x70, 0x6f, 0x73, 0x73, 0x69, 0x62, 0x6c, 0x65, 0x21,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x55, 0x73, 0x65, 0x20, 0x61, 0x72, 0x72, 0x6f, 0x77, 0x20, 0x6b,
    0x65, 0x79, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x6d, 0x6f, 0x76, 0x65,
    0x20, 0x61, 0x6e, 0x64, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x62,
    0x61, 0x72, 0x20, 0x74, 0x6f, 0x20, 0x6a, 0x75, 0x6d, 0x70, 0x2e,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x47, 0x6f, 0x6f, 0x64, 0x20, 0x6c, 0x75, 0x63, 0x6b, 0x21, 0x00
};
char init_string[7] = { 0x71, 0x79, 0x75, 0x6a, 0x7d, 0x68, 0x00 };
Tscroller greeting_scroller;
char summary_scroller_message[5120];
Tscroller summary_scroller;

/* Initialized menu data recovered from main.c's DWARF declarations and the
 * original .data bytes.  Links stay symbolic so the ordinary linker owns the
 * final relocations. */
Tmenu ctrl_menu[6] = {
    { "LEFT",   'r', 0,   0,   0x40, &ctrl.key_left },
    { "RIGHT",  'r', 0,   0,   0x40, &ctrl.key_right },
    { "JUMP",   'r', 0,   0,   0x40, &ctrl.key_fire },
    { "PAUSE",  'r', 0,   0,   0x40, &ctrl.key_pause },
    { "ReJump", 'q', 'q', 'q', 0x04, &options.jump_hold },
    { "Back",   'l', 0,   0,   0x80, NULL }
};

Tmenu snd_menu[3] = {
    { "Sound",   0, 'n', 'm', 0x02, &snd_volume_slider },
    { "Music\\\\", 0, 'n', 'm', 0x02, &msc_volume_slider },
    { "Back",   'l', 0,   0,   0x80, NULL }
};

Tmenu gfx_menu[5] = {
    { "Character",   0,   'x', 'y', 0x20, &play_char },
    { "Start floor", 0,   'v', 'w', 0x10, &floors },
    { "Eye Candy",   0,   'p', 'o', 0x08, &eyecandy_selection },
    { "Fullscreen",  'q', 'q', 'q', 0x04, &options.full_screen },
    { "Back",        'l', 0,   0,   0x80, NULL }
};

Tmenu game_menu[1] = {
    { "Back", 'l', 0, 0, 0x80, NULL }
};

Tmenu profile_menu[3] = {
    { "View Profile",   0x83, 0, 0, 0, NULL },
    { "Change Profile", 0x84, 0, 0, 0, NULL },
    { "Back",           'l',  0, 0, 0x80, NULL }
};

Tmenu opt_menu[4] = {
    { "GFX Options",   'g', 0, 0, 0, gfx_menu },
    { "Sound Options", 'g', 0, 0, 0, snd_menu },
    { "Controls",      'g', 0, 0, 0, ctrl_menu },
    { "Back",          'l', 0, 0, 0x80, NULL }
};

Tmenu custom_menu[5] = {
    { "Start Game", 0x85, 0,   0,   0,    NULL },
    { "Speed",      0,    'p', 'o', 0x08, &scroll_speed_selection },
    { "Floors",     0,    'p', 'o', 0x08, &floor_size_selection },
    { "Gravity",    0,    'p', 'o', 0x08, &gravity_selection },
    { "Back",       'l',  0,   0,   0x80, NULL }
};

Tmenu play_menu[3] = {
    { "Classic Game", 'e', 0, 0, 0,    NULL },
    { "Custom Game",  'g', 0, 0, 0,    custom_menu },
    { "Back",         'l', 0, 0, 0x80, NULL }
};

Tmenu main_menu[7] = {
    { "Play Game",   'g', 0, 0, 0,    play_menu },
    { "Instructions", 'h', 0, 0, 0,    NULL },
    { "Profile",     'g', 0, 0, 0,    profile_menu },
    { "High Scores", 'i', 0, 0, 0,    NULL },
    { "Load Replay", 'z', 0, 0, 0,    NULL },
    { "Options",     'g', 0, 0, 0,    opt_menu },
    { "Exit",        'k', 0, 0, 0x80, NULL }
};

Tmenu replay_menu[5] = {
    { "Play Again",    'e', 0, 0, 0,    NULL },
    { "Watch Replay",  '|', 0, 0, 0,    NULL },
    { "Save Replay",   '{', 0, 0, 0,    NULL },
    { "View Profile",  0x83, 0, 0, 0,  NULL },
    { "Main Menu",     'l', 0, 0, 0x80, NULL }
};

extern void save_options(Toptions *o, PACKFILE *fp);
extern void load_options(Toptions *o, PACKFILE *fp);
extern void reset_options(Toptions *o);
extern Thisc_table *make_hisc_table(char*);
extern void reset_hisc_table(Thisc_table*, char*, int, int);
extern int load_hisc_table(Thisc_table*, PACKFILE*);
extern void save_hisc_table(Thisc_table*, PACKFILE*);
extern int qualify_hisc_table(Thisc_table*, int);
extern void sort_hisc_table(Thisc_table*);
extern void enter_hisc_table(Thisc_table*, int, char*);
extern int save_profile(Tprofile *profile);
extern int get_rank_id(Tprofile *profile);
extern char *get_rank(Tprofile *profile);
extern Tprofile *select_profile(Tprofile *current_profile, Tavailable_profile *profiles,
                                int numProfiles, Tcontrol *ctrl);
extern int rebuild_profile_list(Tavailable_profile **profs);
extern Tprofile *load_profile(char *handle);
extern Tprofile *create_profile(char *handle, int overwrite);
extern int handle_menu(Tmenu *menu, Tmenu_params *mp, Tcontrol *ctrl,
                       BITMAP *bmp, void (*callback)(void), int x, int y, int dx);
extern int get_slider_value(Tmenu_slider *s);
extern int get_selection_value(Tmenu_selection *s);
extern void draw_menu(BITMAP *bmp, Tmenu *menu, Tmenu_params *mp,
                      int x, int y, int dx);
extern void reset_menu(Tmenu *menu, Tmenu_params *mp, int selection);
extern void destroy_replay(Treplay *r);
extern Treplay *load_replay(const char *filename);
extern Treplay *replay_selector(Tcontrol *ctrl, char *path);
extern int calc_replay_checksum(Treplay *r);
extern int save_replay(const char *path, const char *file, Treplay *r, int size,
                       int make_new_date);
extern char *get_filename(const char *path);
extern char *get_extension(const char *path);
extern void fldads_start(void);
extern void run_demo(char *file_name);
extern int new_game(void);
extern int play(void);
extern int load_character(const char *filename, int attrib, void *param);
extern void view_scores(Thisc_table**, char**);

void set_current_avatar(void);

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

extern void destroy_game_data(Tgame_data *gd);
extern Tgame_data *create_game_data(void);
extern char *getGameDataXML(Tgame_data *gd);
extern void add_jump_sequence(Tgame_data *gd, Tgd_jump_sequence *js);
extern void add_combo(Tgame_data *gd, Tgd_combo *c);
extern void reset_player(Tplayer *p);
extern Treplay *create_replay(int size);

#ifndef ICYTOWER_SYNTHETIC_LINK

#else
void play_sound(SAMPLE *s, int pitch, int please_pan);
#endif

/* Oracle: main.c:2267, 0x40b6bc..0x40bc43.  Debug keys select the historical
 * presentation experiments; ordinary play always takes the direct path. */
int debug;

/* Partial source recovery of main.c:3405, 0x411a00..0x415e0c.  This retains
 * the oracle's real game-state ownership and phase order while the remaining
 * results/replay branches are being recovered instruction by instruction. */
extern void handle_player_input(Tcontrol*);
extern void update_player(Tplayer *p);
extern int jump_player(Tplayer *p, int force);
extern void play_jump_sound(Tplayer *p);

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

#ifndef ICYTOWER_SYNTHETIC_LINK

#endif

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void line_alert(char *text);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void show_instructions(void);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void show_credits(void);
char *get_version_str(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
int new_rand(void);
inline void new_srand(int s);
inline void syncProfileFromOptions(void);
void syncOptionsFromProfile(void);
int get_gamepad_value(char *dir);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void take_screenshot(BITMAP *bmp);
void open_web_browser(const char *pURL);
void load_new_ad_image(void);
int ok_to_play(void);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void testWindowResolution(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int show_name(char *name, int attribs);
void play_sound(SAMPLE *s, int pitch, int please_pan);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void play_menu_move(void);
void play_menu_select(void);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
void stopGameMusic(void);
void replaceBadCharacters(char *string, char newChar);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
void replay_menu_callback(void);
void main_menu_callback(void);
int do_replay_menu(void);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);
void draw_frame(BITMAP *dst);
int play(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void pwd_garble_string(char *str, int key);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void datafile_callback_slow(DATAFILE *d);
void datafile_callback(DATAFILE *d);
void color_map_callback(int pos);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
void log2file(const char *format, ...);
void end_game(void);
void uninit_game(void);
void save_config(void);
void change_profile(void);
inline void update_reward(void);
void myDeleteFile(char *path, char *file);
void set_current_avatar(void);
void update_frame(void);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
void run_demo(char *file_name);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
BITMAP *loadScrambled(char *fileName);
int check_beta_tester(void);
int check_characters(void);
void startGameMusic(void);
int start_reward(int lev);
void handle_player_collision_original(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
int init_game(int argc, char **argv);

/* Forward declarations; definitions follow in their original source order. */
void log2file(const char *format, ...);
void line_alert(char *text);
char *get_version_str(void);
void myDeleteFile(char *path, char *file);
int my_alert(char *func, char *txt, int choice, int enter_hint);
void load_new_ad_image(void);
Treplay *get_demo(void);
Tcontrol *get_controls(void);
void take_screenshot(BITMAP *bmp);
void pwd_garble_string(char *str, int key);
void play_sound(SAMPLE *s, int pitch, int please_pan);
int new_rand(void);
inline void new_srand(int s);
BITMAP *loadScrambled(char *fileName);
int check_dir(const char *filename, int attrib, void *param);
int load_character(const char *filename, int attrib, void *param);
void set_current_avatar(void);
void for_each_directory(const char *basedir, int (*cb)(const char *filename, int attrib, void *param));
int check_characters(void);
void startGameMusic(void);
void stopGameMusic(void);
void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y);
void draw_progress_bar(void);
void color_map_callback(int pos);
void datafile_callback(DATAFILE *d);
void datafile_callback_slow(DATAFILE *d);
void syncOptionsFromProfile(void);
inline void syncProfileFromOptions(void);
int check_beta_tester(void);
int ok_to_play(void);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
int get_gamepad_value(char *dir);
int show_name(char *name, int attribs);
int add_profile(const char *filename, int attrib, void *param);
int rebuild_profile_list(Tavailable_profile **profs);
void switchedFromProgram(void);
void switchedToProgram(void);
void clickedCloseButton(void);
void open_web_browser(const char *pURL);
int init_game(int argc, char **argv);
void save_config(void);
void uninit_game(void);
void change_profile(void);
void blit_to_screen(BITMAP *bmp);
void draw_reward(BITMAP *bmp);
inline void update_reward(void);
int start_reward(int lev);
void play_jump_sound(Tplayer *p);
void handle_player_input(Tcontrol *control);
void draw_frame(BITMAP *bmp);
void update_frame(void);
void end_game(void);
inline int is_custom_replay(Treplay *r);
int new_game(void);
int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy, int *ix, int *iy);
void handle_player_collision_vector(int lastX, int lastY);
void handle_player_collision_vector_2(int lastX, int lastY);
void handle_player_collision_combo(int lastX, int lastY);
void handle_player_collision_old(int lastX, int lastY);
void handle_player_collision_original(int lastX, int lastY);
void fadeIn(BITMAP *bmp, int speed);
void fadeOut(int speed);
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified, int *qValues, int showQ);
int play(void);
void show_credits(void);
void show_instructions(void);
void play_menu_move(void);
void play_menu_select(void);
void testWindowResolution(void);
void main_menu_callback(void);
void run_demo(char *file_name);
void replay_menu_callback(void);
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f, int pos_x, int pos_y, int colour, int bg_color);
void replaceBadCharacters(char *string, char newChar);
void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color);
int do_replay_menu(void);
void force_create_profile(void);
void startMenuMusic(void);
void stopMenuMusic(void);
void checkMenuFocus(void);
int _mangled_main(int argc, char **argv);

void log2file(const char *format, ...)
{
    static pthread_mutex_t sLogMutex = PTHREAD_MUTEX_INITIALIZER;
    static char logfilename[1024];
    va_list ptr;
    PACKFILE *fp;
    if (itrcheck) return;
    pthread_mutex_lock(&sLogMutex);
    if (!logfilename[0]) get_logfile_path(logfilename, sizeof(logfilename));
    fp = fopen(logfilename, "at");
    if (fp) {
        va_start(ptr, format);
        vfprintf(fp, format, ptr);
        vsprintf(last_log, format, ptr);
        fputc('\n', fp);
        fclose(fp);
    }
    pthread_mutex_unlock(&sLogMutex);
}

void line_alert(char *text)
{
    int color;
    int width;
    int height;

    set_trans_blender(0,0,0,158);
    drawing_mode(DRAW_MODE_TRANS,0,0,0);
    color=makecol(0,0,0);
    width=0;
    height=0;
    if (gfx_driver) {
        height=gfx_driver->h;
        width=gfx_driver->w;
    }
    rectfill(screen,0,0,width,height,color);
    solid_mode();
    draw_sprite(screen,data[88].dat,103,200);
    textprintf_centre_ex(screen,data[51].dat,320,220,-1,-1,"%s",text);
}

char *get_version_str(void)
{
    return "1.5.1";
}

void myDeleteFile(char *path, char *file)
{
    char buf[2048];
    sprintf(buf, "%s%s", path, file);
    delete_file(buf);
}

/* Candidate recovered from the complete 0x40cd68..0x40d452 modal path. */
int my_alert(char *func, char *txt, int choice, int enter_hint)
{
    Tcontrol *menu_ctrl = &menu_params.ctrl;
    int status;
    int done;
    int w;

    w = MAX(text_length(data[51].dat, func ? func : " "),      /* 467 */
            text_length(data[51].dat, txt ? txt : " "));
    gui_fg_color = makecol(0, 0, 0);                           /* 470 */
    gui_bg_color = makecol(255, 255, 255);                     /* 471 */
    set_trans_blender(0, 0, 0, 158);                           /* 473 */
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                    /* 474 */
    rectfill(screen, 0, 0, gfx_driver ? gfx_driver->w : 0,
             gfx_driver ? gfx_driver->h : 0, makecol(0, 0, 0)); /* 475 */
    solid_mode();                                              /* 476 */
    blit(screen, swap_screen, 0, 0, 0, 0, 639, 479); /* 478 */
    acquire_bitmap(screen);
    draw_sprite(screen, data[88].dat, 103, 130);                /* 485 */
    textprintf_centre_ex(screen, data[51].dat, 320, 135, -1, -1, "%s", func); /* 486 */
    if (txt)
        textout_centre_ex(screen, data[54].dat, txt, 320, 180, makecol(0, 0, 0), -1); /* 487 */
    if (enter_hint)                                            /* 488 */
        textout_right_ex(screen, data[54].dat, "(enter to continue)", 520, 200,
                         makecol(80, 80, 80), -1);              /* 489 */
    while (is_any(&ctrl) || is_any(menu_ctrl) || key[KEY_ESC]) { /* 493 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0); rest(2);
    }
    clear_keybuf();                                            /* 500 */
    done = 0;
    status = 0;
    while (!done && !closeButtonClicked) {                     /* 502 */
        cycle_count = 0;                                       /* 503 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0);     /* 504 */
        if (is_left(&ctrl) || is_left(menu_ctrl)) status = -1;  /* 507 */
        if (is_right(&ctrl) || is_right(menu_ctrl)) status = 0; /* 510 */
        if (key[KEY_ESC]) {                                    /* 514 */
            done = -1;
            status = 0;
        }
        if (is_fire(&ctrl) || is_fire(menu_ctrl) || is_enter(menu_ctrl)) done = -1; /* 519 */
        if (choice) {                                          /* 523 */
            vsync();                                           /* 525 */
            draw_sprite(screen, data[status == -1 ? 11 : 10].dat, 240, 220); /* 527 */
            draw_sprite(screen, data[status == -1 ? 8 : 7].dat, 365, 220);   /* 529 */
        }
        if (!cycle_count) rest(2);                             /* 532 */
    }
    while (is_any(&ctrl) || is_any(menu_ctrl) || key[KEY_ESC] || key[KEY_ENTER]) { /* 537 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0); rest(2); /* 538 */
    }
    clear_keybuf();                                            /* 543 */
    blit(swap_screen, screen, 0, 0, 0, 0, 639, 479); /* 545 */
    return status;                                              /* 548 */
}

void load_new_ad_image(void)
{
    const FLDAdSpot *pAd = fldads_get_random_ad();
    if (pAd) {
        log2file("Got ad: %s", pAd->pLocalImagePath);
        if (pFLDAdBitmap) {
            destroy_bitmap(pFLDAdBitmap);
            pFLDAdBitmap = NULL;
        }
        pFLDAdBitmap = load_bitmap(pAd->pLocalImagePath, NULL);
        pFLDAd = pAd;
    }
}

Treplay *get_demo(void)
{
    return demo;
}

Tcontrol *get_controls(void)
{
    return &ctrl;
}

void take_screenshot(BITMAP *bmp)
{
    static int number;
    PALETTE p;
    BITMAP *b;
    char buf[256];
    int found;

    for (;;) {
        sprintf(buf, "screenshots/icytower_%04d.png", number++);
        found = exists(buf);
        if (number > 9999) {
            log2file("*** Too many screenshots in the screenshot folder! Delete some and try again.");
            return;
        }
        if (!found)
            break;
    }
    get_palette(p);
    b = create_sub_bitmap(bmp, 0, 0, bmp->w, bmp->h);
    save_bitmap(buf, b, p);
    destroy_bitmap(b);
    while (key[KEY_F1])
        ;
}

void pwd_garble_string(char *str, int key)
{
    int i;
    int len_i;

    len_i = strlen(str);
    for (i = 0; i < len_i; i++)
        str[i] ^= key - i;
}

void play_sound(SAMPLE *s, int pitch, int please_pan)
{
    int pan;
    int pit;
    if (itrcheck) return;
    if (pitch) pit=new_rand()%300+925;
    else pit=1000;
    if (!s || !options.snd_volume) return;
    if (please_pan) {
        pan=(int)((float)(ply[player_id]->x/640.0f)*192.0f+32.0f);
        any11=pan;
    }
    else pan=128;
    if (fast_forward) pit<<=1;
    if (fast_fast_forward) pit<<=1;
    play_sample(s,options.snd_volume,pan,pit,0);
}

int new_rand(void)
{
    int x;
    seed = seed * 1.4294484665;
    while (seed > 65535.0f) seed -= 65535.0f;
    x = (int)seed;
    return (int)((seed-x) * 65535.0f);
}

inline void new_srand(int s)
{
    seed=s;
}

BITMAP *loadScrambled(char *fileName)
{
    int fileSize;
    char *data;
    FILE *fp;
    char *password;
    int pLen;
    int i;
    int j;
    char *newFile;
    PALETTE pal;
    BITMAP *png;

    fileSize = file_size_ex(fileName);
    data = malloc(fileSize);
    if (!data)
        return NULL;
    fp = fopen(fileName, "rb");
    if (!fp)
        return NULL;
    fread(data, fileSize, 1, fp);
    fclose(fp);
    password = "%2hJd8#9NsM/";
    pLen = strlen(password);
    for (i = 0; i < fileSize; i += pLen)
        for (j = 0; j < pLen; j++)
            data[i + j] ^= password[j];
    newFile = "data/com/temp.dat";
    fp = fopen(newFile, "wb");
    if (!fp)
        return NULL;
    fwrite(data, fileSize, 1, fp);
    fclose(fp);
    png = load_png(newFile, pal);
    delete_file(newFile);
    return png;
}

int check_dir(const char *filename, int attrib, void *param)
{
    char name[1024];
    char *n=get_filename(filename);
    log2file(filename);
    if ((attrib & FA_DIREC) && *n!='.') {
        sprintf(name, "%s/%s.txt", filename, n);
        if (exists(name))
            num_chars++;
    }
    return 0;
}

int load_character(const char *filename, int attrib, void *param)
{
    static int count;
    char *name;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            characters[count].bmp = load_character_bmp(name,
                &characters[count].uses_datafile, characters[count].pal);
            log2file(" %s (%s): %s", name, filename,
                characters[count].bmp ? "ok" : "error");
            if (!characters[count].bmp) {
                num_chars--;
                *allegro_errno = 0;
                return 0;
            }
            strcpy(characters[count].name, name);
            count++;
        }
    }
    return 0;
}

void set_current_avatar(void)
{
    int i;
    for (i=0; i<num_chars; i++) {
        if (!stricmp(characters[i].name, ((Tavatar_profile *)profile)->avatar)) {
            curr_char=i;
            play_char.value=i;
        }
    }
}

void for_each_directory(const char *basedir,
                        int (*cb)(const char *filename, int attrib, void *param))
{
    char dir_and_wildcard[256];
    strncpy(dir_and_wildcard, basedir, sizeof(dir_and_wildcard));
    strcat(dir_and_wildcard, "*");
    for_each_file_ex(dir_and_wildcard, FA_DIREC, 0, cb, NULL);
}

int check_characters(void)
{
    int i;
    char base_char_dir[256];
    size_t base_char_dir_len;
    char additional_char_dir[256];
    int has_additional_char_dir;

    getcwd(base_char_dir, sizeof(base_char_dir));
    base_char_dir_len = strlen(base_char_dir);
    strcpy(base_char_dir + base_char_dir_len, "/characters/");
    has_additional_char_dir = get_custom_characters_dir(
        additional_char_dir, sizeof(additional_char_dir));
    log2file("Searching %s for characters", base_char_dir);
    for_each_directory(base_char_dir, check_dir);
    if (has_additional_char_dir) {
        log2file("Searching %s for characters", additional_char_dir);
        for_each_directory(additional_char_dir, check_dir);
    }
    if (!num_chars)
        return 0;
    characters = malloc(num_chars * sizeof(*characters));
    for_each_directory(base_char_dir, load_character);
    if (has_additional_char_dir)
        for_each_directory(additional_char_dir, load_character);
    if (!num_chars)
        return 0;
    play_char.max = num_chars - 1;
    for (i = 0; i < num_chars; i++)
        characters[i].ok = characters[i].bmp != NULL;
    set_current_avatar();
    return 1;
}

void startGameMusic(void)
{
    gameMusicVoiceID = -1;
    if (!options.msc_volume)
        return;
    if (custom.bg_music) {
        gameMusicVoiceID = play_sample(custom.bg_music, options.msc_volume,
                                       128, 1000, 1);
        return;
    }
    if (custom.bg_midi) {
        set_volume(-1, options.msc_volume);
        play_midi(custom.bg_midi, 1);
        return;
    }
    if (bg_beat)
        gameMusicVoiceID = play_sample(bg_beat, options.msc_volume,
                                       128, 1000, 1);
}

void stopGameMusic(void)
{
    if (gameMusicVoiceID >= 0)
        voice_stop(gameMusicVoiceID);
    if (custom.bg_music)
        stop_sample(custom.bg_music);
    if (custom.bg_midi)
        stop_midi();
}

void load_sound(SAMPLE **dest, char *fname, BITMAP *bmp, int y)
{
    if (bmp)
        textprintf_ex(bmp, font, 0, y, 15, -1, "loading: %s", fname);
    *dest = load_wav(fname);
    if (*dest)
        return;
    alert("load_sound(): file not found", fname, NULL, "OK", NULL, 0, 0);
}

void draw_progress_bar(void)
{
    int size;
    int ypos;
    static int value;
    int maxVal;

    if (itrcheck)
        return;
    size=value*4;
    maxVal=212;
    if (size>maxVal)
        size=maxVal;
    acquire_screen();
    ypos=400;
    rect(screen,108,ypos,532,ypos+10,makecol(150,150,150));
    rectfill(screen,320-size,ypos,320+size,ypos+10,makecol(100,100,100));
    rectfill(screen,0,420,639,430,makecol(255,255,255));
    textout_centre_ex(screen,font,last_log,320,420,makecol(150,150,150),makecol(255,255,255));
    release_screen();
    value++;
}

void color_map_callback(int pos)
{
    if (!(pos & 15))
        draw_progress_bar();
}

void datafile_callback(DATAFILE *d)
{
    draw_progress_bar();
}

void datafile_callback_slow(DATAFILE *d)
{
    static int p;

    if (!(p & 15))
        draw_progress_bar();
    p++;
}

void syncOptionsFromProfile(void)
{
    options.msc_volume = profile->msc_volume;
    options.snd_volume = profile->snd_volume;
    options.jump_hold = profile->jump_hold;
    options.flash = profile->flash;
    snd_volume_slider.value = options.snd_volume;
    msc_volume_slider.value = options.msc_volume;
    eyecandy_selection.value = options.flash;
    set_current_avatar();
    get_profile_dir_for_profile(replay_directory, sizeof(replay_directory),
                                profile->handle);
    strcat(replay_directory, "replays/");
}

inline void syncProfileFromOptions(void)
{
    profile->msc_volume = options.msc_volume;
    profile->snd_volume = options.snd_volume;
    profile->jump_hold = options.jump_hold;
    profile->flash = options.flash;
}

int check_beta_tester(void)
{
    int i;
    Tbeta *b;
    FILE *fp;
    char pwd[16] = "12345678\0";

    fp = fopen("password.txt", "rt");
    if (!fp) {
        allegro_message("password.txt not found");
        return 0;
    }
    fread(pwd, 8, 1, fp);
    fclose(fp);
    garble_string(pwd, 8);
    b = testers;
    the_tester = NULL;
    while (b) {
        if (!strncmp(pwd, b->code, 8))
            the_tester = b;
        b = b->next;
    }
    if (the_tester)
        return 1;
    log2file("no tester match found");
    return 0;
}

int ok_to_play(void)
{
    return 1;
}

SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id)
{
    return logg_load_memory(df[id].dat, df[id].size);
}

int get_gamepad_value(char *dir)
{
    char *action = get_config_string(NULL, dir, "nothing");
    if (!stricmp(action, "up"))
        return 4;
    if (!stricmp(action, "down"))
        return 8;
    if (!stricmp(action, "left"))
        return 1;
    if (!stricmp(action, "right"))
        return 2;
    if (!stricmp(action, "jump"))
        return 16;
    return 0;
}

int show_name(char *name, int attribs)
{
    allegro_message("Caught `%s', attribs %d\n", name, attribs);
    return 0;
}

int add_profile(const char *filename, int attrib, void *param)
{
    char buf[1024];
    char *file;

    file = get_filename(filename);
    if (*file != '.') {
        get_profile_dir_for_profile(buf, sizeof(buf), file);
        sprintf(buf, "%s%s.itp", buf, file);
        if (exists(buf)) {
            numProfiles++;
            if (profiles)
                profiles = realloc(profiles, numProfiles * sizeof(*profiles));
            else
                profiles = malloc(sizeof(*profiles));
            strcpy(profiles[numProfiles - 1].handle, file);
        }
    }
    return 0;
}

int rebuild_profile_list(Tavailable_profile **profs)
{
    char profiledir[1024];

    if (profiles) {
        free(profiles);
        profiles = NULL;
    }
    profiles = malloc(sizeof(*profiles));
    strcpy(profiles[0].handle, "CREATE NEW PROFILE");
    numProfiles = 1;
    get_profiles_dir(profiledir, sizeof(profiledir));
    strcat(profiledir, "/*");
    for_each_file_ex(profiledir, FA_DIREC, 0, add_profile, NULL);
    if (profs)
        *profs = profiles;
    return numProfiles;
}

void switchedFromProgram(void)
{
    hasFocus = 0;
}

void switchedToProgram(void)
{
    hasFocus = 1;
}

void clickedCloseButton(void)
{
    closeButtonClicked = 1;
}

void open_web_browser(const char *pURL)
{
    char cmd[256];
    sprintf(cmd, "url.dll, FileProtocolHandler %s", pURL);
    log2file(" calling '%s'", cmd);
    ShellExecuteA(NULL, "open", "rundll32", cmd, "", 4);
}

/* Partial recovery of main.c:1375, 0x40e7dc..0x40fe78.  The oracle starts
 * with packfile/network state, command-line processing, and the platform
 * subsystems before it exposes the datafile-backed game globals. */
int init_game(int argc, char **argv)
{
    char title[64];
    WSADATA wsaData;
    unsigned short wVersionRequested;
    char cfgfilename[256];
    char profiles_dir[1024];
    char tmpHandle[32];
    char *replay_path;
    PACKFILE *cfg;
    DATAFILE *loader;
    DATAFILE *sfx;
    BITMAP *fldLogo;
    Tgamepad *pad;
    int whiteColor;
    int i;

    tmpHandle[0]=0; /* 1382 */
    init_ok=0; /* 1385 */
    log2file("\nINIT GAME"); /* 1393 */
    packfile_password(NULL); /* 1394 */
    sprintf(title,"Icy Tower v%s","1.5.1"); /* 1395 */
    set_window_title(title); /* 1395 */
    wVersionRequested=MAKEWORD(2,2); /* 1403 */
    if (WSAStartup(wVersionRequested,&wsaData)!=0)
        log2file(" !!! Failed to setup Winsock"); /* 1404 */
    if (LOBYTE(wsaData.wVersion)<2 || HIBYTE(wsaData.wVersion)<2) /* 1407 */
        log2file(" !!! Failed to get proper Winsock version (wanted 2.2, got %d.%d)",
                 LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion)); /* 1408 */
    play_char.max=0; /* 1413 */
    play_char.value=0; /* 1413 */
    play_char.bmp=NULL; /* 1414 */
    eyecandy_selection.value=0; /* 1416 */
    eyecandy_selection.size=3; /* 1417 */
    eyecandy_selection.caption[0]=strdup("Lots"); /* 1418 */
    eyecandy_selection.caption[1]=strdup("Some"); /* 1419 */
    eyecandy_selection.caption[2]=strdup("None"); /* 1420 */
    scroll_speed_selection.value=0; /* 1422 */
    scroll_speed_selection.size=6; /* 1423 */
    scroll_speed_selection.caption[5]=strdup("Normal"); /* 1424 */
    scroll_speed_selection.caption[4]=strdup("Hasty"); /* 1425 */
    scroll_speed_selection.caption[3]=strdup("Fast"); /* 1426 */
    scroll_speed_selection.caption[2]=strdup("Faster"); /* 1427 */
    scroll_speed_selection.caption[1]=strdup("Fastest"); /* 1428 */
    scroll_speed_selection.caption[0]=strdup("Insane"); /* 1429 */
    floor_size_selection.value=0; /* 1431 */
    floor_size_selection.size=5; /* 1432 */
    floor_size_selection.caption[0]=strdup("Wide"); /* 1433 */
    floor_size_selection.caption[1]=strdup("Normal"); /* 1434 */
    floor_size_selection.caption[2]=strdup("Shorter"); /* 1435 */
    floor_size_selection.caption[3]=strdup("Shortest"); /* 1436 */
    floor_size_selection.caption[4]=strdup("Tiny"); /* 1437 */
    floor_size_selection.value=2; /* 1438 */
    gravity_selection.value=0; /* 1440 */
    gravity_selection.size=3; /* 1441 */
    gravity_selection.caption[0]=strdup("Helium"); /* 1442 */
    gravity_selection.caption[1]=strdup("Normal"); /* 1443 */
    gravity_selection.caption[2]=strdup("Heavy"); /* 1444 */
    fldads_start(); /* 1448 */
    if (argc>2) { /* 1489 */
        char *checkFile;
        int check;
        replay_path=NULL;
        check=0;
        i=1;
        do {
            checkFile=argv[i]; /* 1494 */
            if (checkFile[0]!='-')
                replay_path=checkFile;
            if (!stricmp(checkFile,"-check")) check=1; /* 1497 */
            else if (!stricmp(checkFile,"-jumps")) cmdline.jumps=1; /* 1500 */
            else if (!stricmp(checkFile,"-combos")) cmdline.combos=1; /* 1503 */
            else if (!stricmp(checkFile,"-sd")) cmdline.sd=1; /* 1506 */
            else if (!stricmp(checkFile,"-keys")) cmdline.keys=1; /* 1509 */
            else if (!stricmp(checkFile,"-all")) { /* 1512 */
                cmdline.jumps=1; /* 1513 */
                cmdline.combos=1; /* 1514 */
                cmdline.sd=1; /* 1515 */
                cmdline.keys=1; /* 1516 */
            }
            else if (!stricmp(checkFile,"-tiny")) cmdline.tiny=1; /* 1518 */
            i++; /* 1493 */
        } while (i<argc); /* 1493 */
        if (!check) { /* 1524 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1536 */
            allegro_message("<%s>\nis not a vaild option.",argv[1]); /* 1537 */
            log2file("*** Erroneous option (%s)",argv[1]); /* 1538 */
            dropped_file_is_not_a_replay=1; /* 1539 */
            return 0;
        }
        log2file("Loading %s",replay_path); /* 1525 */
        demo=load_replay(replay_path); /* 1526 */
        if (!demo) { /* 1527 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1528 */
            printf("<itrcheck_results status=\"error\">%s</itrcheck_results>\n",
                   get_filename(replay_path)); /* 1529 */
            log2file("*** Failed!"); /* 1530 */
            dropped_file_is_not_a_replay=1; /* 1531 */
            return 0; /* 1532 */
        }
        itrcheck=1; /* 1544 */
        log2file("ITRCHECK activated, checking <%s>",replay_path); /* 1545 */
    } else if (argc==2) { /* 1550 */
        log2file("Loading %s",argv[1]); /* 1551 */
        demo=load_replay(argv[1]); /* 1552 */
        if (!demo) { /* 1553 */
            strcpy(tmpHandle,get_filename(argv[1])); /* 1555 */
            get_extension(tmpHandle)[-1]=0; /* 1556 */
            profile=load_profile(tmpHandle); /* 1558 */
            if (!profile) { /* 1559 */
                tmpHandle[0]=0; /* 1560 */
                set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1561 */
                allegro_message("The file\n<%s>\nis not a vaild Icy Tower profile.",
                               get_filename(argv[1])); /* 1562 */
                log2file("*** Failed!"); /* 1563 */
                dropped_file_is_not_a_replay=1; /* 1564 */
                return 0; /* 1565 */
            }
            free(profile); /* 1567 */
            profile=NULL; /* 1568 */
        }
    }
    log2file("Creating hiscore tables."); /* 1575 */
    for (i=0;i<15;i++) { /* 1576 */
        hisc_tables[i]=make_hisc_table(hisc_names[i]); /* 1577 */
        if (!hisc_tables[i]) { /* 1578 */
            log2file("*** failed."); /* 1579 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1580 */
            allegro_message("Failed reserve memory for highscore table."); /* 1581 */
            return 0;
        }
        reset_hisc_table(hisc_tables[i],"Harold",1000,0); /* 1586 */
    }
    log2file("Initiating controls"); /* 1590 */
    init_control(&ctrl); /* 1591 */
    get_configfile_path(cfgfilename,sizeof(cfgfilename)); /* 1595 */
    log2file("Loading config file"); /* 1597 */
    cfg=pack_fopen(cfgfilename,"rp"); /* 1598 */
    if (cfg) { /* 1599 */
        load_options(&options,cfg); /* 1600 */
        for (i=0;i<15;i++) /* 1601 */
            if (!load_hisc_table(hisc_tables[i],cfg)) /* 1602 */
                reset_hisc_table(hisc_tables[i],"Harold",1000,0); /* 1603 */
        pack_fclose(cfg); /* 1606 */
    } else
    {
        log2file("*** failed."); /* 1609 */
        log2file("Resetting to default config"); /* 1610 */
        reset_options(&options); /* 1611 */
    }
    if (tmpHandle[0]) { /* 1614 */
        log2file("Setting last profile"); /* 1615 */
        strcpy(options.lastProfile,tmpHandle); /* 1616 */
    }
    if (!itrcheck) { /* 1619 */
        options.timesStarted++; /* 1620 */
        log2file("Game started %d times",options.timesStarted); /* 1621 */
    }

    allegro_init(); /* 1623 */
    set_color_depth(32); /* 1624 */
    if (!options.full_screen) { /* 1627 */
        log2file("Setting windowed mode 640x480"); /* 1628 */
        if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0)!=0) { /* 1629 */
            log2file("*** failed."); /* 1633 */
            options.full_screen=-1; /* 1634 */
        } else
            window=1; /* 1630 */
    }
    if (options.full_screen) {
        log2file("Setting fullscreen mode 640x480"); /* 1638 */
        if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0)!=0) { /* 1639 */
            log2file("*** failed."); /* 1643 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1644 */
            allegro_message("Failed to set graphics mode."); /* 1645 */
            return 0; /* 1646 */
        }
        window=0; /* 1640 */
    }
    if (!screen) { /* 1651 */
        log2file("ERROR: screen was not set"); /* 1652 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1653 */
        allegro_message("For some reason, the game failed to go into\ngraphics mode. Try starting the game again.\n\nIf this problem persists,\nplease visit www.freelunchdesign.com."); /* 1654 */
        return 0; /* 1655 */
    }
    log2file("Graphics mode set. (screen = %d)",screen); /* 1668 */
    install_mouse(); /* 1659 */
    enable_hardware_cursor(); /* 1660 */
    select_mouse_cursor(2); /* 1661 */
    if (!options.full_screen) /* 1663 */
        show_mouse(screen); /* 1664 */

    textprintf_centre_ex(screen,font,320,220,makecol(180,180,180),-1,
                         "please wait"); /* 1671 */
    set_color_conversion(COLORCONV_NONE); /* 1674 */
    packfile_password("(c) Free Lunch Design"); /* 1675 */
    loader=load_datafile("data/loading.dat"); /* 1676 */
    log2file("Loading loader."); /* 1677 */
    if (!loader) { /* 1678 */
        log2file(" *** failed"); /* 1679 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1680 */
        allegro_message("Failed to load loader datafile."); /* 1681 */
        return 0; /* 1682 */
    }
    packfile_password(NULL); /* 1684 */
    log2file("Putting FLD Logo on screen"); /* 1687 */
    fldLogo=loader[1].dat; /* 1692 */
    select_palette(loader[0].dat); /* 1696 */
    whiteColor=makecol(255,255,255); /* 1702 */
    clear_to_color(screen,whiteColor); /* 1707 */
    draw_sprite(screen,fldLogo,320-fldLogo->w/2,200-fldLogo->h/2); /* 1711 */
    unload_datafile(loader); /* 1715 */

    log2file("Setting focus modes"); /* 1721 */
    if (options.full_screen)
        set_display_switch_mode(SWITCH_BACKAMNESIA); /* 1723 */
    else
        set_display_switch_mode(SWITCH_BACKGROUND); /* 1726 */
    log2file("Setting focus callbacks"); /* 1728 */
    set_display_switch_callback(SWITCH_IN,switchedToProgram); /* 1729 */
    set_display_switch_callback(SWITCH_OUT,switchedFromProgram); /* 1730 */
    set_close_button_callback(clickedCloseButton); /* 1731 */
    srand((unsigned int)time(NULL)); /* 1733 */
    log2file("Installing timers"); /* 1736 */
    draw_progress_bar(); /* 1737 */
    install_timers(); /* 1738 */
    cycle_count=0; /* 1741 */
    log2file("Installing keyboard"); /* 1743 */
    draw_progress_bar(); /* 1744 */
    install_keyboard(); /* 1745 */
    log2file("Installing sound"); /* 1749 */
    draw_progress_bar(); /* 1750 */
    install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL); /* 1753 */
    log2file("Installing joystick/gamepad"); /* 1756 */
    draw_progress_bar(); /* 1757 */
    got_joystick=(install_joystick(JOY_TYPE_AUTODETECT)==0); /* 1758 */
    if (got_joystick) { /* 1759 */
        ctrl.use_joy=1; /* 1760 */
        log2file(" gamepad has %d buttons",joy[0].num_buttons); /* 1761 */
        if (exists("gamepad.txt")) { /* 1763 */
            log2file(" getting values from gamepad.txt"); /* 1766 */
            set_config_file("gamepad.txt"); /* 1767 */
            pad=get_gamepad(); /* 1765 */
            pad->up=get_gamepad_value("up"); /* 1768 */
            pad->left=get_gamepad_value("left"); /* 1769 */
            pad->right=get_gamepad_value("right"); /* 1770 */
            pad->down=get_gamepad_value("down"); /* 1771 */
            for (i=1;i<=32;i++) { /* 1772 */
                sprintf(cfgfilename,"b%d",i); /* 1774 */
                pad->b[i-1]=get_gamepad_value(cfgfilename); /* 1775 */
            }
        } else {
            log2file(" gamepad.txt is missing, setting defaults"); /* 1780 */
            pad=get_gamepad(); /* 1779 */
            pad->up=4; /* 1781 */
            pad->left=1; /* 1782 */
            pad->right=2; /* 1783 */
            pad->down=8; /* 1784 */
            for (i=0;i<32;i++) /* 1785 */
                pad->b[i]=16; /* 1786 */
        }
    } else
        log2file(" no gamepad or joystick found, play with keyboard only"); /* 1791 */

    log2file("Reserving memory"); /* 1795 */
    draw_progress_bar(); /* 1796 */
    swap_screen=create_bitmap(SCREEN_W,SCREEN_H); /* 1797 */
    if (!swap_screen) { /* 1798 */
        log2file("*** failed."); /* 1799 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1800 */
        allegro_message("Failed reserve memory screen buffers."); /* 1801 */
        return 0; /* 1802 */
    }

    set_color_conversion(0x00ffffff); /* 1820 */
    draw_progress_bar(); /* 1821 */
    pwd_garble_string(init_string,50); /* 1806 */
    log2file("Loading data"); /* 1818 */
    packfile_password(init_string); /* 1822 */
    data=load_datafile_callback("data/data.dat",datafile_callback_slow); /* 1823 */
    if (!data) { /* 1824 */
        log2file("*** failed."); /* 1825 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1826 */
        allegro_message("Failed to load datafile."); /* 1827 */
        return 0; /* 1828 */
    }
    packfile_password(NULL); /* 1830 */
    draw_progress_bar(); /* 1836 */
    log2file("Initiating player"); /* 1835 */
    player_id=rand()%1000; /* 1837 */
    ply[player_id]=malloc(sizeof(*ply[player_id])); /* 1838 */
    if (!ply[player_id]) { /* 1839 */
        log2file("*** failed."); /* 1840 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1841 */
        allegro_message("Failed to allocate memory for player."); /* 1842 */
        return 0; /* 1843 */
    }
    if (!itrcheck) { /* 1846 */
        ((RGB *)data[0].dat)[0].r=0; /* 1849 */
        ((RGB *)data[0].dat)[0].g=0;
        ((RGB *)data[0].dat)[0].b=0;
        gameover_bmp=data[55].dat; /* 1850 */
        log2file("Checking profile directory"); /* 1855 */
        get_profiles_dir(profiles_dir,sizeof(profiles_dir)); /* 1858 */
        if (!file_exists(profiles_dir,FA_DIREC,0)) { /* 1860 */
            log2file("  does not exist, trying to create"); /* 1861 */
            mkdir(profiles_dir); /* 1863 */
        }
        if (!file_exists(profiles_dir,FA_DIREC,0)) { /* 1867 */
            log2file("  *** failed!"); /* 1868 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1869 */
            allegro_message("Failed to create profile directory %s",profiles_dir); /* 1870 */
            return 0; /* 1871 */
        }
        log2file("Checking available profiles"); /* 1876 */
        draw_progress_bar(); /* 1877 */
        rebuild_profile_list(0); /* 1878 */
        log2file("Loading profile"); /* 1880 */
        draw_progress_bar(); /* 1881 */
        log2file(" loading '%s'",options.lastProfile); /* 1882 */
        profile=load_profile(options.lastProfile); /* 1883 */
        if (!profile) { /* 1884 */
            log2file(" profile not found '%s'",options.lastProfile); /* 1885 */
            log2file(" trying to load default profile '%s'","guest"); /* 1887 */
            profile=load_profile("guest"); /* 1888 */
            if (!profile) { /* 1889 */
                log2file(" profile not found '%s'","guest"); /* 1890 */
                profile=create_profile("guest",1); /* 1892 */
                log2file(" created profile '%s'",profile->handle); /* 1893 */
            }
        }
        if (!profile) { /* 1896 */
            log2file("  *** failed!"); /* 1897 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1898 */
            allegro_message("Failed create profile."); /* 1899 */
            return 0; /* 1900 */
        }
        strcpy(options.lastProfile,profile->handle); /* 1903 */
        syncOptionsFromProfile(); /* 1904 */
        log2file("Checking available characters"); /* 1910 */
        draw_progress_bar(); /* 1911 */
        if (!check_characters()) { /* 1912 */
            log2file(" *** no characters available"); /* 1913 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1914 */
            allegro_message("No characters available.\nPlease reinstall game or add custom characters.\nRefer to readme.txt."); /* 1915 */
            return 0;
        }
        select_palette(data[0].dat); /* 1919 */
        log2file("Loading SFX"); /* 1924 */
        draw_progress_bar(); /* 1925 */
        packfile_password(init_string); /* 1927 */
        log2file(" loading sounds"); /* 1926 */
        sfx=load_datafile_callback("data/sfx15.dat",datafile_callback); /* 1928 */
        strcpy(sfx_file,"sfx15.dat"); /* 1935 */
        if (sfx) /* 1929 */
            log2file(" sfx15.dat loaded"); /* 1936 */
        else {
            log2file(" could not load data/sfx15.dat"); /* 1930 */
            log2file("no sound"); /* 1932 */
        }
        packfile_password(NULL); /* 1938 */
        if (sfx) { /* 1940 */
            log2file("Getting sounds from data file"); /* 1941 */
            draw_progress_bar(); /* 1942 */
            combo_sound[0]=getSampleFromOggDatafile(sfx,8); /* 1943 */
            combo_sound[1]=getSampleFromOggDatafile(sfx,18); /* 1944 */
            combo_sound[2]=getSampleFromOggDatafile(sfx,9); /* 1945 */
            combo_sound[3]=getSampleFromOggDatafile(sfx,17); /* 1946 */
            combo_sound[4]=getSampleFromOggDatafile(sfx,21); /* 1947 */
            combo_sound[5]=getSampleFromOggDatafile(sfx,1); /* 1948 */
            combo_sound[6]=getSampleFromOggDatafile(sfx,5); /* 1949 */
            combo_sound[7]=getSampleFromOggDatafile(sfx,6); /* 1950 */
            combo_sound[8]=getSampleFromOggDatafile(sfx,15); /* 1951 */
            combo_sound[9]=getSampleFromOggDatafile(sfx,20); /* 1952 */
            bg_beat=getSampleFromOggDatafile(sfx,2); /* 1953 */
            bg_menu=getSampleFromOggDatafile(sfx,3); /* 1954 */
            speaker[0]=getSampleFromOggDatafile(sfx,10); /* 1955 */
            speaker[1]=getSampleFromOggDatafile(sfx,7); /* 1956 */
            speaker[2]=getSampleFromOggDatafile(sfx,19); /* 1957 */
            sounds[2]=getSampleFromOggDatafile(sfx,0); /* 1958 */
            sounds[4]=getSampleFromOggDatafile(sfx,13); /* 1959 */
            sounds[6]=getSampleFromOggDatafile(sfx,14); /* 1960 */
            sounds[7]=getSampleFromOggDatafile(sfx,4); /* 1961 */
            sounds[8]=getSampleFromOggDatafile(sfx,16); /* 1962 */
            menu_sounds[0]=getSampleFromOggDatafile(sfx,11); /* 1963 */
            menu_sounds[1]=getSampleFromOggDatafile(sfx,12); /* 1964 */
            jump_sound[0]=NULL; /* 1965 */
            jump_sound[1]=NULL; /* 1966 */
            jump_sound[2]=NULL; /* 1967 */
            sounds[0]=NULL; /* 1968 */
            sounds[1]=NULL; /* 1969 */
            sounds[3]=NULL; /* 1970 */
            sounds[5]=NULL; /* 1971 */
            log2file("Releasing ogg datafile."); /* 1972 */
            unload_datafile(sfx); /* 1973 */
            sfx=NULL; /* 1974 */
        } else
            log2file(" no sounds loaded"); /* 1932 */
        log2file("Setting menu values"); /* 1978 */
        snd_volume_slider.value=options.snd_volume; /* 1979 */
        msc_volume_slider.value=options.msc_volume; /* 1980 */
        eyecandy_selection.value=options.flash; /* 1981 */
        gravity_selection.value=options.gravity; /* 1982 */
        floor_size_selection.value=options.floor_size; /* 1983 */
        scroll_speed_selection.value=options.start_speed; /* 1984 */
        floors.max=profile->best_floor>999 ? 9 : profile->best_floor/100; /* 1986 */
        floors.value=profile->start_floor; /* 1987 */
        if (floors.value>floors.max)
            floors.value=floors.max;
    }
    log2file("Cleaning up"); /* 2063 */
    draw_progress_bar(); /* 2064 */
    log2file("Welcome to Icy Tower"); /* 2069 */
    draw_progress_bar(); /* 2070 */
    i=0; /* 2072 */
    while (!keypressed() && cycle_count<=149) { /* 2072 */
        if (!(cycle_count%10) && i!=cycle_count) { /* 2073 */
            draw_progress_bar(); /* 2075 */
            i=cycle_count; /* 2076 */
        }
        rest(2); /* 2078 */
    }
    seed=rand()%2367; /* 2082 */
    fadeOut(16); /* 2083 */
    clear_bitmap(screen); /* 2084 */
    vsync(); /* 2085 */
    clear_keybuf(); /* 2086 */
    init_ok=1; /* 2090 */
    return -1;
}

void save_config(void)
{
    FILE *fp;
    char cfgfilename[256];

    log2file("  saving config and scores");
    get_configfile_path(cfgfilename, sizeof(cfgfilename));
    fp = pack_fopen(cfgfilename, "wp");
    if (fp) {
        int i;
        save_options(&options, fp);
        for (i = 0; i < 15; i++)
            save_hisc_table(hisc_tables[i], fp);
        pack_fclose(fp);
    } else {
        log2file("    *** failed");
    }
}

void uninit_game(void)
{
    int i;

    log2file("\nUNINIT");
    if (init_ok) {
        log2file("Saving config");
        save_config();
        log2file("Saving profile '%s'",profile->handle);
        syncProfileFromOptions();
        save_profile(profile);
    }
    if (testers)
        destroy_all(testers);
    log2file("Freeing sound memory");
    for (i=0;i<10;i++)
        if (combo_sound[i]) destroy_sample(combo_sound[i]);
    for (i=0;i<3;i++)
        if (jump_sound[i]) destroy_sample(jump_sound[i]);
    for (i=0;i<3;i++)
        if (speaker[i]) destroy_sample(speaker[i]);
    if (menu_sounds[0]) destroy_sample(menu_sounds[0]);
    if (menu_sounds[1]) destroy_sample(menu_sounds[1]);
    for (i=0;i<9;i++)
        if (sounds[i]) destroy_sample(sounds[i]);
    if (bg_beat) destroy_sample(bg_beat);
    if (bg_menu) destroy_sample(bg_menu);
    log2file("Freeing custom character memory");
    for (i=0;i<num_chars;i++)
        if (characters[i].bmp) destroy_bitmap(characters[i].bmp);
    free(characters);
    log2file("Unloading datafile");
    if (data) unload_datafile(data);
    log2file("Free buffer memory");
    if (swap_screen) destroy_bitmap(swap_screen);
    log2file("Free highscore tables");
    for (i=0;i<15;i++)
        if (hisc_tables[i]) destroy_hisc_table(hisc_tables[i]);
    log2file("Free player");
    if (ply[player_id]) free(ply[player_id]);
    set_gfx_mode(GFX_TEXT,0,0,0,0);
    log2file("Exiting Allegro");
    allegro_exit();
}

void change_profile(void)
{
    Tprofile *newProfile;

    newProfile = profile;
    if (newProfile) {
        syncProfileFromOptions();
        save_profile(profile);
        newProfile = profile;
    }
    newProfile = select_profile(newProfile, profiles, numProfiles, &ctrl);
    if (newProfile) {
        if (profile) free(profile);
        profile = newProfile;
        strcpy(options.lastProfile, profile->handle);
        syncOptionsFromProfile();
        save_config();
        rebuild_profile_list(0);
    }
}

void blit_to_screen(BITMAP *bmp)
{
    static int blit_mode;
    if (debug) {
        if (key[KEY_F2]) blit_mode = 0;                      /* 2271 */
        if (key[KEY_F3]) blit_mode = 1;                       /* 2272 */
        if (key[KEY_F4]) blit_mode = 2;                       /* 2273 */
        if (key[KEY_F5]) blit_mode = 3;                       /* 2274 */
        if (key[KEY_F6]) blit_mode = 4;                       /* 2275 */
        if (key[KEY_F7]) blit_mode = 5;                       /* 2276 */
        if (key[KEY_F8]) blit_mode = 6;                       /* 2277 */
    }
    acquire_screen();
    if (!blit_mode) {                                          /* 2282 */
        blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);
    }
    else if (blit_mode == 1) {                                 /* 2285 */
        draw_sprite_h_flip(screen, bmp, 0, 0);                 /* 2286 */
    }
    else if (blit_mode == 2) {                                 /* 2288 */
        draw_sprite_v_flip(screen, bmp, 0, 0);                 /* 2289 */
    }
    else if (blit_mode == 3) {                                 /* 2291 */
        int y;
        for (y = 0; y < 480; y++) {                            /* 2293 */
            int x = fixtoi(fixsin(itofix(y + logic_count * 5)) *
                            ply[player_id]->level);              /* 2294 */
            blit(bmp, screen, 0, y, x, y, 640, 1);
        }
    }
    else if (blit_mode == 4) {                                 /* 2297 */
        int y = ply[player_id]->level % 480;                    /* 2298 */
        blit(bmp, screen, 0, 0, 0, y, bmp->w, bmp->h);           /* 2301 */
        blit(bmp, screen, 0, 0, 0, y - 480, bmp->w, bmp->h);     /* 2302 */
    }
    else if (blit_mode == 5) {                                 /* 2304 */
        double dx = ply[player_id]->x - 160.0;                   /* 2305 */
        double dy = ply[player_id]->y - 160.0;                   /* 2306 */
        int x, y;
        if (dx <= 0.0) x = 0;
        else if (dx > 320.0) x = 320;
        else x = (int)dx;
        if (dy <= 0.0) y = 0;
        else if (dy > 240.0) y = 240;
        else y = (int)dy;
        stretch_blit(bmp, screen, x, y, 320, 240, 0, 0, 640, 480); /* 2307 */
    }
    else if (blit_mode == 6) {                                 /* 2309 */
        double dx = ply[player_id]->x - 80.0;                    /* 2310 */
        double dy = ply[player_id]->y - 80.0;                    /* 2311 */
        int x, y;
        if (dx <= 0.0) x = 0;
        else if (dx > 520.0) x = 520;
        else x = (int)dx;
        if (dy <= 0.0) y = 0;
        else if (dy > 360.0) y = 360;
        else y = (int)dy;
        stretch_blit(bmp, screen, x, y, 160, 120, 0, 0, 640, 480); /* 2312 */
    }
    release_screen();
}

/* Partial recovery of main.c:2320, 0x4070fc..0x407341.  The two paths are
 * distinguished by the original eye-candy option: rectangular scaling for
 * flash mode 1 and fixed-point rotation/scaling for mode 0. */
void draw_reward(BITMAP *bmp)
{
    if (options.flash) {
        if (options.flash!=1)
            return;
        stretch_sprite(bmp,reward_bmp,
                       320-(int)(fixtof(reward_scale/2)*reward_bmp->w),
                       360-(int)(fixtof(reward_scale/2)*reward_bmp->h)-fixtoi(reward_scale*reward_bmp->h/2),
                       fixtoi(reward_scale*reward_bmp->w),
                       fixtoi(reward_scale*reward_bmp->h));
    }
    else {
        rotate_scaled_sprite(bmp,reward_bmp,
                             320-(int)(fixtof(reward_scale/2)*reward_bmp->w),
                             360-(int)(fixtof(reward_scale)*120-reward_bmp->h*fixtof(reward_scale/2)),
                             reward_scale<<8,reward_scale);
    }
}

inline void update_reward(void)
{
    if (reward_time > 60)
        reward_scale += 3277;
    if (reward_time <= 9)
        reward_scale -= 6554;
    reward_time--;
}

int start_reward(int lev)
{
    int r;
    int i, p;

    reward_time = 80;
    reward_scale = 0;
    if (lev <= 6) r = 0;
    else if (lev <= 14) r = 1;
    else if (lev <= 24) r = 2;
    else if (lev <= 34) r = 3;
    else if (lev <= 49) r = 4;
    else if (lev <= 69) r = 5;
    else if (lev <= 99) r = 6;
    else if (lev <= 139) r = 7;
    else if (lev > 199) r = 9;
    else r = 8;
    if (!itrcheck) {
        if (!options.flash && r > 2) {
            for (i = 0; i < (r - 2) * 16; i++) {
                p = create_particle(stars, 320, 360);
                stars[p].sy = -((((new_rand() % 500) + 500) << 16) / 100);
                stars[p].sx = ((((new_rand() % 1000) - 500) << 16) * (r - 2)) / 100;
            }
        }
        reward_bmp = data[90 + r].dat;
    }
    play_sound(combo_sound[r], 0, 0);
    return r;
}

void play_jump_sound(Tplayer *p)
{
    if (p->sy < -22.0f)
        play_sound(custom.jump_sound[2], 1, 1);
    else if (p->sy < -15.0f)
        play_sound(custom.jump_sound[1], 1, 1);
    else
        play_sound(custom.jump_sound[0], 1, 1);
}

/* Partial recovery of main.c, 0x40b3e4..0x40b6bc.  This is the oracle's
 * normal-control path; replay control recording remains to be restored. */
void handle_player_input(Tcontrol *control)
{
    int rp;
    unsigned char flags;

    if (!control)
        return;
    if (recording) {
        poll_control(control,0);
        if (ply[player_id]->dead) {
            demo->data[rec_pos+1].key_flags=0x80;
            demo->data[rec_pos+1].cycle_count=0;
            demo->data[rec_pos+2].key_flags=0;
            demo->data[rec_pos+2].cycle_count=0;
        }
        else {
            flags=control->flags&0x93;
            if (demo->data[rec_pos].key_flags&0x80) {
                demo->data[rec_pos+1].key_flags=0x80;
                demo->data[rec_pos+1].cycle_count=0;
                demo->data[rec_pos+2].key_flags=0;
                demo->data[rec_pos+2].cycle_count=0;
            }
            else if (demo->data[rec_pos].key_flags==flags)
                demo->data[rec_pos].cycle_count++;
            else {
                rec_pos++;
                demo->data[rec_pos].key_flags=flags;
                demo->data[rec_pos].cycle_count=0;
            }
        }
    }
    else {
        rp=rec_pos-1;
        if (rp>=0) {
            if (rp>=demo->size)
                control->flags=0;
            else {
                control->flags=demo->data[rp].key_flags;
                if (demo->data[rp].cycle_count>0)
                    demo->data[rp].cycle_count--;
                else
                    rec_pos++;
            }
        }
        else
            rec_pos++;
    }

    if (is_left(control)) {
        if (ply[player_id]->sx>0)
            ply[player_id]->sx*=0.7;
        ply[player_id]->sx-=0.3;
    }
    else if (is_right(control)) {
        if (ply[player_id]->sx<0)
            ply[player_id]->sx*=0.7;
        ply[player_id]->sx+=0.3;
    }
    else
        ply[player_id]->sx*=0.9;

    if (!rejump) {
        if (is_fire(control)) {
            if (!ply[player_id]->jump_key) {
                if (jump_player(ply[player_id],0)) {
                    ply[player_id]->jump_key=-1;
                    play_jump_sound(ply[player_id]);
                    if (profile)
                        profile->total_jumps++;
                }
            }
        }
        if (!is_fire(control))
            ply[player_id]->jump_key=0;
    }
    else {
        if (is_fire(control)) {
            if (jump_player(ply[player_id],0)) {
                play_jump_sound(ply[player_id]);
                if (profile)
                    profile->total_jumps++;
            }
        }
    }
}

/* Partial recovery of main.c:2490, 0x40929c..0x40b3e4.  This keeps the
 * oracle's renderer phases in source: floor plane, animated character,
 * particles, rewards, advertising image, and score/status overlays. */
void draw_frame(BITMAP *bmp)
{
    int x;
    int y;
    int p_im;
    int flip;
    int cx;
    int cy;
    int ls;
    int fo;
    int so;
    int max_bg_id;
    BITMAP *customFrame;
    int oy;
    int ox;


    fo = profile->start_floor * 3 + 0x11;              /* 2498 */
    so = profile->start_floor + 0x65;                  /* 2499 */

    frame_count++;                                     /* 2501 */

    if (ply[player_id]->level <= 0xc8) {                /* 2505 */
        max_bg_id = 2;
    } else if (ply[player_id]->level <= 0x15e) {        /* 2506 */
        max_bg_id = 3;
    } else {
        max_bg_id = (ply[player_id]->level >= 0x259) + 4; /* 2507: branchless in the
                                                             * historical binary (cmp/setge/
                                                             * movzbl/add $0x4), not a
                                                             * separate else-if/else pair */
    }

    {
        /* 2510: row = floor(map.offset / 256), rounding toward -inf for negative offsets
         * (the js/lea 0xff/sar sequence is the historical bias-then-shift idiom). */
        int row = (map.offset < 0) ? ((map.offset + 0xff) >> 8) : (map.offset >> 8);
        int i;

        if (row > last_stripe_y) {                      /* 2510 */
            last_stripe_y++;                             /* 2511 */

            bg_stripe_ids[4] = bg_stripe_ids[3];         /* 2514 */
            bg_stripe_ids[3] = bg_stripe_ids[2];
            bg_stripe_ids[2] = bg_stripe_ids[1];
            bg_stripe_ids[1] = bg_stripe_ids[0];

            /* 2523: only ONE physical `movl $0x0,bg_stripe_ids` exists (offsets 336..352),
             * reached both by the >0x28 branch falling straight through and by the
             * collision-detected branch's own jump (offset 372/380 both target 4093ec,
             * the same address) -- a genuine control-flow merge, not two compiled copies,
             * so the reset is written once and reached from both predecessors. */
            if (new_rand() % 100 > 0x28)                  /* 2517 */
                goto reset_stripe;
            bg_stripe_ids[0] = new_rand() % max_bg_id;     /* 2521 */
            if (bg_stripe_ids[0] != bg_stripe_ids[1] &&    /* 2522 */
                bg_stripe_ids[0] != bg_stripe_ids[2])
                goto skip_reset;
        reset_stripe:
            bg_stripe_ids[0] = 0;                          /* 2523 */
        skip_reset:
            ;
        }

        for (i = 0; i != 4; i++) {                        /* 2529 */
            BITMAP *stripe = data[bg_stripe_ids[i + 1] + 1].dat; /* 2530 */
            blit(stripe, bmp, 0, 0, 0x25,
                 i * 0x80 + (map.offset % 0x100) / 2,
                 stripe->w, stripe->h);
        }
    }

    if (hurry_y + 0x63 <= 0x242 && options.flash != 2) {  /* 2541 */
        BITMAP *hspr = data[67].dat;   /* ? historical index 67 (0x4dd23c + 0x430) not named by
                                         * function_data_refs beyond the generic "data" table */
        draw_sprite(bmp, hspr, 320 - hspr->w / 2, hurry_y); /* 2542 */
    }

    {
        /* 2547: real loop, evidenced by the back-edge test `cmpl $0x1e0,-0x178(%ebp)` (cx)
         * at offset 1580 sitting AFTER the sign-text block (main.c:2562..2575) but tagged
         * to line 2547 -- a rotated for-loop whose test/increment sit at the bottom. `esi`
         * (the per-row Tfloor pointer) is decremented by sizeof(Tfloor)==0x18 each pass
         * (offset 1570 `sub $0x18,%esi`) in lockstep with cx += 0x10 (offset 1573), for
         * 0x1e0/0x10 == 30 rows. Field offsets (empty=0, start_tile=4, end_tile=8, level=12,
         * sign=16, tiles=20) match Tfloor exactly for BOTH the floor body below (2548..2560)
         * and the sign body carried into D2 (2562..2575): they are the SAME loop over the
         * SAME `room` pointer, so this loop opens here and its closing brace is in D2.c. */
        Tfloor *room = &map.room[31]; /* ? starting row; map.room[32], matches the historical sign scan bound */

        for (cx = 0; cx != 0x1e0; cx += 0x10) {      /* 2547 */
            if (!room->empty) {                       /* 2548 */
                int f;   /* DWARF: block-scoped int at -0x180(ebp); no separate `rowy`/`tile`
                          * names are declared by the historical DWARF for this block, so the
                          * tile index reuses this one slot and the row-y term is recomputed
                          * inline (cx + (map.offset & 0xf) - 6) at each use, 2552. Each of the
                          * three draws (2552/2555/2558) is ~20 bytes short of its historical
                          * count for the same reason as 2606: the line table charges each with
                          * a `dec/or $0xfffffff0/inc` abs()-style tail duplicated far away from
                          * its local computation (2552's local copy at 566..623 plus a second
                          * copy at 2212..2217; 2555's at 740..780 plus 1964..1969; 2558's at
                          * 826..876 plus 2224..2229) -- one statement, two compiled copies from
                          * -O2 block layout, not a missing branch. */

                ls = fo + room->tiles * 3;   /* 2549 */
                if (ls > 0x2c) {
                    ls = 0x2c;
                }
                if (room->level > 0x1387) {  /* 2550 */
                    ls += 3;
                }

                f = room->start_tile;     /* 2551 */

                /* left edge tile */
                cy = f * 16 - 5;          /* 2552, stored at -0x174(%ebp) */
                x = cy;                      /* ? DWARF tracks a separate `x` over this same span; mirrored here */
                draw_sprite(bmp, data[ls].dat, x, cx + (map.offset & 0xf) - 6);

                f++;                                  /* 2553 */
                for (; f < room->end_tile; f++) {      /* 2554 */
                    cy = f * 16;                        /* 2555 */
                    x = cy;
                    draw_sprite(bmp, data[ls + 1].dat, x, cx + (map.offset & 0xf) - 6); /* 2555 */
                }

                cy = f * 16;                            /* 2558 */
                x = cy;
                draw_sprite(bmp, data[ls + 2].dat, x, cx + (map.offset & 0xf) - 6);     /* 2558 */

                if (debug && !key[KEY_F2]) {  /* 2560 */
                    textprintf_ex(bmp, font, 0x208, cx + (map.offset & 0xf), 15, -1, "%d",
                                  (room->level - 1) / 10);
                }
            }

            /* lines 2562..2575 continue this same per-row loop (sign text) in D2.c;
             * loop closes and `room` is decremented at the end of D2.c's chunk. */
    /* still inside the D1 per-row loop for 2562..2575, then combo text / rewards for 2589..2651 */
            if (room->sign) {                  /* 2562, same `room` row as the floor draw above */
                int s;
                int sy;
                int sw;
                int c1;
                int c2;

                s = so + room->tiles;          /* 2563 */
                if (s > 0x6e) {
                    s = 0x6e;
                }
                if (room->level > 0x1387) {    /* 2564 */
                    s++;
                }
                sy = cx + (map.offset & 0xf) + 10;  /* 2565 */
                sw = ((BITMAP *)data[s].dat)->w;     /* 2566 */
                cy = room->start_tile + (room->end_tile - room->start_tile) / 2; /* 2567 */
                cy *= 16;
                draw_sprite(bmp, data[s].dat, cy, sy);
                c1 = makecol(255, 255, 255);   /* 2569 */
                c2 = makecol(55, 55, 55);      /* 2570 */
                cy += sw / 2;                  /* 2571 */
                /* shadowed stripe-number text: four gray offsets then one white on top */
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 7, c2, -1, "%d", room->sign); /* 2571 */
                textprintf_centre_ex(bmp, data[54].dat, cy + 2, sy + 6, c2, -1, "%d", room->sign); /* 2572 */
                textprintf_centre_ex(bmp, data[54].dat, cy,     sy + 6, c2, -1, "%d", room->sign); /* 2573 */
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 5, c2, -1, "%d", room->sign); /* 2574 */
                textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 6, c1, -1, "%d", room->sign); /* 2575 */
            }

            room--;                            /* offset 1570 `sub $0x18,%esi`, sizeof(Tfloor) */
        }   /* closes the for (cx = 0; ...) loop opened in D1.c (2547) */
    }       /* closes the `{ Tfloor *room = ...; ... }` scope opened in D1.c */

    for (fo = 0; fo < 512; fo++) {          /* 2580: loop index reuses fo; not otherwise evidenced as the star-loop counter */
        if (stars[fo].intensity) {           /* 2581 */
            draw_sprite(swap_screen, data[stars[fo].color + 0x75].dat, fixtoi(stars[fo].x), fixtoi(stars[fo].y));  /* 2582 */
        }
    }

    /* 2589: status==0 branches to a separate narrow-speed check at original offset 2256.
     * That path reaches the edge sprite directly for -0.02<sx<0.02, or reaches the
     * ordinary p_im=1 speed/frame route outside that band. Other statuses select
     * their own pose bases before the 2594 range test. */
    /* 2590/2591/2595/2597 all show the same shape as 2606's gap: the line table charges each of
     * these comparisons far more than one `fldl/fucompp/fnstsw/test` sequence costs (2590: 42 vs
     * our 9; 2591 has its own separate -3.0/6/7 arm so is not the same statement), because -O2
     * duplicates the fcompp+branch sequence at every predecessor edge that reaches it (status==3's
     * arm at offsets 1819..1852 plus a second copy at 3356..3365; 2595's inner-band test appears
     * at 2561..2608 AND again at 3323..3346; 2597's edge==0/-0.2 test appears at 1869..1924,
     * 2308..2315, 2608..2615 AND 3365..3391) -- one source comparison, several compiled copies from
     * jump-threading. These duplicated instruction fragments do not by themselves prove
     * extra source branches; the remaining function difference still needs investigation. */
    if (ply[player_id]->status) {                       /* 2589 */
        if (ply[player_id]->status == 3) {                /* 2590 */
            if (ply[player_id]->sy > 3.0)              /* 2590: fucompp/fnstsw compare direction inferred, not asserted */
                p_im = 7;
            else
                p_im = 6;                                   /* offset 1852's own `mov $0x6,%esi` */
        }
        else if (ply[player_id]->status == 2) {           /* 2591 */
            if (ply[player_id]->sy > 3.0)               /* 2591 */
                p_im = 7;
            else
                p_im = 6;                                   /* offset 2556's own `mov $0x6,%esi` */
        }
        else if (ply[player_id]->status == 1) {            /* 2592 */
            if (ply[player_id]->sy < -3.0)               /* 2592 */
                p_im = 5;
            else
                p_im = 6;
        }
    } else {
        if (ply[player_id]->sx >= 0.0) {
            if (ply[player_id]->sx < 0.02) {
                ply[player_id]->frame = 0;
                customFrame = custom.frame[0];
                oy = 1 - customFrame->h;
                goto edge_sprite;
            }
        } else if (ply[player_id]->sx > -0.02) {
            ply[player_id]->frame = 0;
            customFrame = custom.frame[0];
            oy = 1 - customFrame->h;
            goto edge_sprite;
        }
        goto p_im_one_path;
    }
    /* 2594: the status-zero route bypasses this pose-range check. */
    if ((unsigned)(p_im - 5) <= 2) {          /* 2594: range test on p_im */
        if (ply[player_id]->sx > -0.01 && ply[player_id]->sx < 0.01)
            p_im = 8;                        /* 2595: strict band includes zero */
    }

    if (p_im != 1) {
        ply[player_id]->frame = 0;
    } else {
p_im_one_path:
        if (ply[player_id]->sx < 0.2 && ply[player_id]->sx > -0.2)
            ply[player_id]->frame = 0;
        if (ply[player_id]->frame > 3)
            ply[player_id]->frame = 0;
        p_im = 1;
    }

    /* 2605: no comparison precedes the custom.frame[0] loads at 2315 and 2615,
     * so the earlier null-guard guess is dropped. */
    /* 2606: original `1 - custom.frame[0]->h` fragments appear at offsets 1924,
     * 2321/2621, 3391, and 6051. The current source emits a different layout;
     * attribution of the remaining deficit to compiler duplication alone is unproved. */
    customFrame = custom.frame[0];                                  /* 2605: default for edge==0 */
    oy = 1 - customFrame->h;                                        /* 2606 */

    /* 2609: offset 2631's `test %esi,%esi; je` enters the edge-sprite path at
     * 2329 when p_im==0. Edge sprite calls jump to 2819, after the ordinary
     * rotate/frame draw. The earlier candidate inverted this condition and
     * drew through both paths. */
    if (!p_im) {                                         /* 2609 */
edge_sprite:
        if (ply[player_id]->edge) {                      /* 2611 */
            customFrame = (logic_count & 8) ? custom.frame[13] : custom.frame[14]; /* 2612/2615 */
            oy = (int)ply[player_id]->y + oy;           /* 2624 */
            if (ply[player_id]->edge == 2) {            /* 2617 */
                ox = (int)ply[player_id]->x - customFrame->w + 0xb;
                draw_sprite_h_flip(bmp, customFrame, ox, oy);
            } else {
                ox = (int)ply[player_id]->x - 0xb;
                draw_sprite(bmp, customFrame, ox, oy);
            }
        }
        /* Only edge==0 enters this block at original offset 3115. Both edge==1
         * and edge==2 sprite calls jump to offset 2819 afterward. */
        else {
            if (map.offset > 0xc8 && ply[player_id]->y > 400.0)
                customFrame = custom.frame[11];         /* 2629 */
            else if (logic_count <= 0xb)
                customFrame = custom.frame[9];          /* 2630 */
            else if (logic_count > 0x18 && logic_count <= 0x24)
                customFrame = custom.frame[10];         /* 2631/2632 */
            ox = -(customFrame->w / 2);                 /* 2636 */
            oy = (int)ply[player_id]->y + oy;
            ox = (int)ply[player_id]->x + ox;
            if (ply[player_id]->sx == 0.0)             /* 2638 */
                draw_sprite(bmp, customFrame, ox, oy);
            else
                draw_sprite_h_flip(bmp, customFrame, ox, oy);
        }
    } else {
        flip = ply[player_id]->rotate;                  /* 2643 */
        if (flip) {
        customFrame = custom.frame[12];                 /* 2644: bypasses the p_im+frame index entirely */
        rotate_sprite(bmp, customFrame, (int)ply[player_id]->x, (int)ply[player_id]->y,
                       ply[player_id]->angle);            /* draw.inl:345, offset 6639..6852;
                                                             x/y args not fully traced -- the 200-byte
                                                             inline body wasn't walked past its w/h loads */
        } else {
        customFrame = custom.frame[p_im + ply[player_id]->frame];   /* 2649/2650: offset 2663 `add 0x3c(%edx),%esi`
                                                            * adds ply->frame onto the still-live p_im (esi,
                                                            * location-list range 2561..2666 covers this add),
                                                            * then offset 2666 indexes custom.frame[] with it --
                                                            * p_im is not dead, it is the pose base index itself,
                                                            * so the 2594 range test that keeps it in [5,8] guards
                                                            * a real array bound, not a provably-redundant compare */
        ox = -(customFrame->w / 2);
        /* 2651: fldl 0x10(%edx)/fldz/fucompp guards the draw, then the fistpl-truncated y and x
         * are added onto the running oy/ox and the draw call issued -- offsets 2686..2786, all one
         * historical source line per function_lines --source-view. */
        /* 2651: sx!=0.0 arm (jne target 0x40b189 = offset 7917) reuses this same
         * ox/oy computation (ecx/edi carry the untruncated oy/ox bases through
         * unchanged, edx stays &ply[player_id] for both the x/y loads and, after
         * the vtable reload, esi stays customFrame) and only swaps the call from
         * *0x44(vtable) (draw_sprite) to *0x50(vtable) (draw_sprite_h_flip,
         * offset 7917..8024), rejoining the same 409d9f continuation either way. */
        oy = (int)ply[player_id]->y + oy;            /* 2651 */
        ox = (int)ply[player_id]->x + ox;            /* 2651 */
        if (ply[player_id]->sx == 0)                  /* 2651 */
            draw_sprite(bmp, customFrame, ox, oy);       /* 2651, draw.inl:238 */
        else
            draw_sprite_h_flip(bmp, customFrame, ox, oy);  /* 2651, draw.inl:280, offset 7917..8024 */
        }
    }


    /* lines 2652..2697: no main.c line-table rows in this offset range (the
     * instructions here -- draw.inl:238/280 inline draw_sprite/draw_sprite_h_flip
     * calls plus reused fragments of main.c:2560/2590-2606/2629-2638 -- are the
     * last unrolled iteration of the scroller/background-layer loop whose source
     * text belongs to D1/D2's declared ranges, not D3's; nothing to add here. */

    /* lines 2698..2699: the scrolling side strips, reconstructed from offsets 2786..3105.
     * The loop base in %esi runs -124, 0, 124, 248, 372 and exits on `cmp $0x1f0` (496, line
     * 2698's `cmp`/`je` test, offsets 2970..2982) with a step of 0x7c (124); the second half's
     * tail jumps back to the first half's entry, so the two physically duplicated halves are one
     * loop body cross-jumped by -O2. Inside it, the `idiv $0x54` REMAINDER of map.offset is
     * multiplied by the double 1.476 at 0x4d6d18 and truncated, giving the scroll offset added to
     * the base (line 2699's own bytes: the divisor setup at 2824..2836 plus the idiv/float-mult/
     * data[100] lookup at 2982..3063 -- the mirror computation for the other half is credited by
     * the line table to the inlined draw.inl:280 body instead, an artifact of the cross-jump, not
     * a separate main.c statement). The sprite is data[100].dat and the two constant arguments in
     * the third slot (x) are 0x235 (565) and 0xffffffc7 (-57), so the strips are vertical, at the
     * right and left screen edges, sharing one y per iteration. Resolved by side-by-side reading
     * of offsets 2786..3110 (aligned_view sbs) against the ORIGINAL's own GFX_VTABLE layout, read
     * from its DWARF rather than guessed: 0x44 draw_sprite, 0x48 draw_256_sprite, 0x4c
     * draw_sprite_v_flip, 0x50 draw_sprite_h_flip, 0x54 draw_sprite_vh_flip. So the call reached
     * with x=0x235 uses 0x44 and 0x48, which is the depth check inside the plain draw_sprite
     * inline (draw.inl:238), and the call reached with x=0xffffffc7 uses 0x50, which is
     * draw_sprite_h_flip: the left strip is the right strip mirrored horizontally. An earlier
     * reading of this block was one vtable slot out and used the vertical flips. */
    for (cy = -124; cy != 496; cy += 124) {                                    /* 2698 */
        cx = cy + (int)((map.offset % 84) * 1.476);                             /* 2699 */
        draw_sprite(bmp, data[100].dat, 565, cx);                                /* 2699 */
        draw_sprite_h_flip(bmp, data[100].dat, -57, cx);                        /* 2699 */
    }

    draw_sprite(bmp, data[16].dat, 22, 100);          /* 2705 */
    /* Machine evidence (offsets 3640..3806 vs 5823..5968, --report comparison.json):
     * the in_combo branch's own tail (after its own blit+draw_sprite) jumps directly
     * into offset 3744, which is INSIDE the reward_time branch's argument setup for
     * textprintf_centre_ex, skipping reward_time's own test (3663) and its own
     * draw_sprite (3677) entirely. A live combo can therefore never also run the
     * reward_time body in the same frame, which is only reachable if the two `if`s
     * are one if/else-if chain, not two independent statements. */
    if (ply[player_id]->in_combo) {                    /* 2706 */
        blit(data[15].dat, bmp, 0, 100 - ply[player_id]->in_combo, 33,   /* 2707 */
             219 - ply[player_id]->in_combo, 16, ply[player_id]->in_combo);
        draw_sprite(bmp, data[14].dat, -8, 210);         /* 2708 */
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d",   /* 2709 */
                              ply[player_id]->acc_level);
    } else if (reward_time) {                            /* 2711 */
        draw_sprite(bmp, data[14].dat, -8, 210);           /* 2712 */
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d",     /* 2713 */
                              ply[player_id]->latest_combo);
    }

    if (hurry_y < 251 || hurry_y > 479) {                 /* 2717 */
        x = 6;
        y = 10;
        cx = 0;
        cy = 0;
    } else {
        x = logic_count % 3 + 5;
        y = (logic_count + 1) % 3 + 8;
        cx = logic_count % 3 - 1;
        cy = (logic_count + 1) % 3 - 1;
    }
    draw_sprite(bmp, data[12].dat, x, y);                  /* 2718 */

    if (hurry_y >= 201 && hurry_y <= 479) {                /* 2719 */
        cx = (logic_count + 2) % 3 - 1;
        cy = (logic_count + 3) % 3 - 1;
    }
    rotate_sprite(bmp, data[13].dat, cx + 34, cy + 28,      /* 2720 */
                  clock_angle ? ftofix((clock_angle % 1500) * 0.1706666) : 0);

    if (reward_time) {                                      /* 2721 */
        draw_reward(swap_screen);                            /* 2722 */
    }

    textprintf_ex(bmp, data[52].dat, 8, 440, -1, -1, "score: %d",   /* 2739 */
                  ply[player_id]->level * 10 + ply[player_id]->score);

    if (!recording) {                                        /* 2742 */
        char myBuf[256];

        if (frame_count & 8) {                                /* 2746 */
            strcpy(myBuf, "REPLAY");                            /* 2747 */
            ls = 630 - text_length(data[53].dat, myBuf);         /* 2748 */
            textprintf_ex(bmp, data[53].dat, ls + 1, 5, makecol(0, 0, 0), -1, "REPLAY");        /* 2749 */
            textprintf_ex(bmp, data[53].dat, ls, 4, makecol(255, 255, 255), -1, "REPLAY");       /* 2750 */
        }

        if (is_playing_custom_game) {                          /* 2754 */
            sprintf(myBuf, "%s Floors", floor_size_selection.caption[demo->floor_size]);  /* 2755 */
            cx = 630 - text_length(data[53].dat, myBuf);          /* 2756 */
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 16, makecol(0, 0, 0), -1);          /* 2757 */
            textout_ex(bmp, data[53].dat, myBuf, cx, 15, makecol(255, 255, 255), -1);         /* 2758 */

            sprintf(myBuf, "%s Speed", scroll_speed_selection.caption[demo->start_speed]);  /* 2760 */
            cx = 630 - text_length(data[53].dat, myBuf);           /* 2761 */
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 26, makecol(0, 0, 0), -1);           /* 2762 */
            textout_ex(bmp, data[53].dat, myBuf, cx, 25, makecol(255, 255, 255), -1);          /* 2763 */

            strcpy(myBuf, gravity_selection.caption[demo->gravity]);   /* 2765 */
            cx = 630 - text_length(data[53].dat, myBuf);              /* 2766 */
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 36, makecol(0, 0, 0), -1);            /* 2767 */
            textout_ex(bmp, data[53].dat, myBuf, cx, 35, makecol(255, 255, 255), -1);           /* 2768 */
        }
        /* DWARF lexical block 124048 (myBuf/myPos/vcr/len/scrollerText) has PC ranges
         * covering both this REPLAY/custom-game text (main.c:2742..2768) and D4's
         * scroller/controller-icon code (main.c up to ~2803, ending right before the
         * unconditional debug F2 overlay): they are one shared `if (!recording) { }`
         * block, not two separate blocks. This region intentionally leaves that
         * block open; D4 declares myPos/len/vcr/scrollerText as siblings of myBuf
         * and closes the brace itself. */

    /* Continues D3's `if (!recording) { ... }` block (DWARF lexical block 124048
     * covers myBuf through here, up to and including the rect() below, as one
     * shared block, not a nested one); the brace is closed just before the
     * unconditional debug F2 overlay. */
        int myPos;
        int len;
        BITMAP *vcr;
        char scrollerText[70];

        vcr = data[127].dat;                            /* 2773 */
        myPos = rec_pos;                                /* 2774 */
        len = demo->size;                               /* 2774 */
        ox = 0x27b - vcr->w;                             /* 2776 */
        y = 0x1db - vcr->h; /* 2777: DWARF: `y`'s slot (reg edi) is live 4279..4341, exactly this
                              * assignment through the dead-check below; the D1-owned `x`
                              * local is NOT live over the matching esi computation here
                              * (its ranges stop at 2234), so that esi temp stays `ox`. */
        draw_sprite(bmp, vcr, ox, y); /* offsets 4282..4315, draw.inl:238 -- the only main.c:2778
                                        * candidate in this range; args are the ox/y just set. */
        if (!ply[player_id]->dead) {                     /* 2779 */
            /* `y` stays live (edi) through 7594..7837 for these three: is_left/is_fire/is_right
             * each build `y + 5` directly in a register (ecx) while the x-argument is spilled
             * through the `cx` stack slot (-0x178) only as a call-argument temporary. */
            if (is_left(&ctrl))                          /* 2780 */
                draw_sprite(bmp, data[128].dat, ox + 0x61, y + 5);
            if (is_fire(&ctrl))                           /* 2781 */
                draw_sprite(bmp, data[130].dat, ox + 0x6b, y + 5);
            if (is_right(&ctrl))                          /* 2782 */
                draw_sprite(bmp, data[129].dat, ox + 0x75, y + 5);
        }
        /* Stack-slot evidence (DW_OP_breg5): cx = -0x178(ebp), cy = -0x174(ebp).
         * offset 4338 "add $0xa,%edi; mov %edi,-0x178(%ebp)" stores y+0xa into cx (edi holds y).
         * offset 4347 "lea 0xa(%esi),%edi; mov %edi,-0x174(%ebp)" stores ox+0xa into cy (esi holds ox),
         * and that same edi is the set_clip_rect x1 argument, i.e. cy, not cx. */
        cx = y + 0xa;                                     /* 2784 */
        cy = ox + 0xa;                                     /* 2785 */
        set_clip_rect(bmp, cy, 0, 0x26f, 0x1df);            /* 2785 */
        /* Machine evidence (offsets 4392..4451 fallthrough vs 6010..6026 jump-in):
         * there is exactly one physical call to sprintf; the empty-comment path only
         * sets up arg3 = "" (esp+0x10) before falling into the shared arg2/arg1/fmt/
         * dest setup and the single call, and the comment path only sets up
         * arg3 = demo->comment before jumping into that same shared setup -- so this
         * is sprintf(scrollerText, "%s%s%s", demo->name, " - ", <arg3>) with a single
         * call site, arg3 selected by the test at 2787, not two separate sprintf
         * statements. */
        sprintf(scrollerText, "%s%s%s", demo->name, " - ",         /* 2787 */
                !demo->comment[0] ? "" : demo->comment);
        /* offset 4479 "mov -0x178(%ebp),%edi; add $0x4,%edi" reloads cx (not cy) for the
         * y-coordinate of every textout_ex below; the x-coordinate keeps using ox. */
        textout_ex(bmp, data[53].dat, demo->name, ox + 0xc - scroll_count / 2,     /* 2788 */
            cx + 4, makecol(150, 150, 160), -1);
        textout_ex(bmp, data[53].dat, demo->name, ox + 0xd - scroll_count / 2,     /* 2789 */
            cx + 4, makecol(200, 200, 210), -1);
        if (demo->comment[0]) {                                                    /* 2790 */
            textout_ex(bmp, data[53].dat, " - ",                                    /* 2791 */
                ox + 0xc - scroll_count / 2 + text_length(data[53].dat, demo->name),
                cx + 4, makecol(200, 200, 210), -1);
            textout_ex(bmp, data[53].dat, demo->comment,                            /* 2792 */
                ox - scroll_count / 2 + 0x1e + text_length(data[53].dat, demo->name),
                cx + 4, makecol(200, 200, 210), -1);
        }
        set_clip_rect(bmp, 0, 0, 0x27f, 0x1df);              /* 2794 */
        if (demo->comment[0]) {                               /* 2796 */
            if (scroll_delay > 0) {                            /* 2797 */
                scroll_delay--;                                 /* 2798 */
            }
            else {
                scroll_count++;                                 /* 2801 */
                if (scroll_count / 2 > text_length(data[53].dat, scrollerText))  /* 2802 */
                    scroll_count = -250;                          /* 2803 */
            }
        }
        /* Original offsets 4808 and 4863 add 0x13 and 0x14 to cx at
         * -0x178(ebp); offsets 4876 and 4853 use cy at -0x174(ebp) for x.
         * The call through GFX_VTABLE +0xbc is rect (draw.inl:112), not rectfill
         * (+0x3c). The old candidate also shifted both y coordinates by 10. */
        rect(bmp, cy, cx + 0x14,                                          /* 2808 */
            cy + (myPos * 117 / len > 0x74 ? 0x74 : myPos * 117 / len),
            cx + 0x13, makecol(50, 200, 50));
    }

    if (debug && key[KEY_F2]) {                                          /* 2812 */
            textprintf_ex(bmp, font, 0, 0, 15, -1, "FPS:%6d / %d", fps, lps);  /* 2813 */
            textprintf_ex(bmp, font, 0, 0xa, 15, -1, "REC:%6d / %d", rec_pos,   /* 2814 */
                demo->size);
            textprintf_ex(bmp, font, 0, 0x14, 15, -1, "    %6d  (%d) ",          /* 2815 */
                demo->data[rec_pos].key_flags, demo->data[rec_pos].cycle_count);
            textprintf_ex(bmp, font, 0xc8, 0, 15, -1, "POS: %d, %d",              /* 2816 */
                (int)ply[player_id]->x, (int)ply[player_id]->y);
            textprintf_ex(bmp, font, 0xc8, 0xa, 15, -1, " dx: %1.2f",              /* 2817 */
                ply[player_id]->sx);
            textprintf_ex(bmp, font, 0xc8, 0x14, 15, -1, "rjp: %d",                 /* 2818 */
                options.jump_hold);
            textprintf_ex(bmp, font, 0x190, 0, 15, -1, "any: %6d %6d %6d",           /* 2819 */
                any11, any12, any13);
            textprintf_ex(bmp, font, 0x190, 0xa, 15, -1, "any: %6d %6d %6d",          /* 2820 */
                any21, any22, any23);
        }
    /* line 2822 tail: fragments at offsets 2246 ("mov $0x2,%edi") and 2556
     * ("mov $0x6,%esi") are also attributed to this line by the line table,
     * but they sit far outside this region's byte range and duplicate
     * register-constant setup that reads as spillover from an earlier
     * region's block layout; no call is associated with them, so nothing
     * is written here beyond the implicit function epilogue, which the
     * closing brace below inherits this annotation for. */
    /* 2822 */
}

void update_frame(void)
{
    Tplayer *p;
    if (reward_time) {
        if (reward_time>60)
            reward_scale+=3277;
        if (reward_time<=9)
            reward_scale-=6554;
        reward_time--;
    }
    p=ply[player_id];
    if (p->dead && p->dead<=299)
        p->dead+=8;
    if (p->edge)
        p->edge_drawn++;
    if (logic_count%10==0)
        p->frame++;
}

void end_game(void)
{
    log2file(" freeing custom data");
    destroy_custom_data(&custom);
}

inline int is_custom_replay(Treplay *r)
{
    return r->floor_shrink != 1 || r->floor_size != 1 ||
           r->start_speed != 5 || r->speed_increase != 1 || r->gravity != 1;
}

/* Recovered from the full 0x40dc9c..0x40e10f control-flow range.  The
 * candidate retains the original state transitions and API boundary; its
 * instruction layout remains under recovery. */
int new_game(void)
{
    int i;

    log2file(" init new game");
    collision_type = 2;
    new_srand(rand() % 0x18ff8);
    rec_pos = 0;
    bg_stripe_ids[4] = 0;
    bg_stripe_ids[3] = 0;
    bg_stripe_ids[2] = 0;
    bg_stripe_ids[1] = 0;
    bg_stripe_ids[0] = 0;
    last_stripe_y = 0;

    if (!itrcheck) {
        floors.max = profile->best_floor > 999 ? 9 : profile->best_floor / 100;
        floors.value = floors.max > profile->start_floor ?
                       profile->start_floor : floors.max;
    }

    jumpSequence.num = 0;
    jumpSequence.dist = 0;
    jumpSequence.start = 0;
    gdLastJumpDiff = 0;
    if (gameData)
        destroy_game_data(gameData);
    gameData = create_game_data();
    if (!gameData)
        log2file("*** failed to allocate memory for gameData, prepare for crash");
    gameData->replay = demo;

    if (demo) {
        log2file(" preparing to show replay");
        recording = 0;
        rejump = demo->rejump;
        rec_seed = demo->random_seed;
        if (is_custom_replay(demo))
            is_playing_custom_game = 1;
        scroll_count = 0;
        scroll_delay = 100;
    } else {
        log2file(" setting up for replay recording");
        recording = 1;
        if (demo)
            destroy_replay(demo);
        demo = create_replay(64000);
        strcpy(demo->name, profile->handle);
        if (is_playing_custom_game) {
            demo->floor_shrink = options.floor_shrink;
            demo->floor_size = options.floor_size;
            demo->start_speed = options.start_speed;
            demo->speed_increase = options.speed_increase;
            demo->gravity = options.gravity;
        } else {
            demo->floor_shrink = 1;
            demo->floor_size = 1;
            demo->start_speed = 5;
            demo->speed_increase = 1;
            demo->gravity = 1;
        }
        rejump = options.jump_hold;
        srand(time(0));
        rec_seed = rand();
        demo->random_seed = rec_seed;
    }

    for (i = 0; i < 15; i++)
        new_personal_best[i] = 0;
    srand(rec_seed);
    log2file(" creating map layout");
    reset_map(&map);
    for (i = 0; i < 30; i++)
        add_floor(&map);
    reset_player(ply[player_id]);
    ply[player_id]->x = 200.0;
    ply[player_id]->y = 431.0;
    ply[player_id]->status = 0;
    ply[player_id]->sx = 0.001;
    reward_time = 0;
    hurry_y = 480;
    if (itrcheck)
        return 1;

    reset_particles(stars);
    log2file(" loading custom character: %s", characters[curr_char].name);
    init_custom(&custom, characters[curr_char].name,
                characters[curr_char].uses_datafile);
    if (!load_frames(&custom))
        return 0;
    load_sounds(&custom);
    log2file(" cc done");
    if (got_joystick)
        ctrl.use_joy = 1;
    return 1;
}

int line_intersect(int ax, int ay, int bx, int by, int cx, int cy, int dx, int dy,
                   int *ix, int *iy)
{
    float r, s, denom;

    denom = (dy - cy) * (bx - ax) + (cx - dx) * (by - ay);
    r = ((dx - cx) * (ay - cy) + (cy - dy) * (ax - cx)) / denom;
    s = ((ay - cy) * (bx - ax) + (ay - by) * (ax - cx)) / denom;
    if (r < 0.0f || s < 0.0f || r > 1.0f || s > 1.0f)
        return 0;
    *ix = ax + (int)(r * (bx - ax) + 0.5);
    *iy = ay + (int)(r * (by - ay) + 0.5);
    return 1;
}

/* Partial recovery of main.c, 0x408d08..0x409137.  The normal path is the
 * oracle's floor-segment intersection; its collision-debug line drawing is
 * intentionally left for the presentation recovery pass. */
void handle_player_collision_vector(int lastX, int lastY)
{
    Tplayer *p;
    int floor_y;
    int floor_x1, floor_x2;
    int left_x, left_y, right_x, right_y;
    int left, right;
    int current_x, current_y;

    p = ply[player_id];
    current_x = (int)p->x;
    current_y = (int)p->y;
    floor_y = -12345678;
    floor_x1 = 0;
    floor_x2 = 0;
    getFloorData(&map, current_y, &floor_y, &floor_x1, &floor_x2);
    if (floor_y == -12345678) {
        getFloorData(&map, lastY, &floor_y, &floor_x1, &floor_x2);
        if (floor_y == -12345678) {
            if (p->status == 2 || p->status == 0)
                p->status = 3;
            return;
        }
    }

    if (debug) {
        if (key[KEY_F2]) {
            int col1 = makecol(255, 0, 0);
            int col2 = makecol(255, 255, 0);
            line(screen, floor_x1, floor_y, floor_x2, floor_y, col1);
            line(screen, current_x - 11, current_y + 1, lastX - 11, lastY, col2);
            line(screen, current_x + 11, current_y + 1, lastX + 11, lastY, col2);
        }
    }
    left = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        current_x - 11, current_y + 1, lastX - 11, lastY, &left_x, &left_y);
    right = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        current_x + 11, current_y + 1, lastX + 11, lastY, &right_x, &right_y);
    if (!left && !right) {
        if (p->status == 2 || p->status == 0)
            p->status = 3;
        return;
    }
    p->edge = left == right ? 0 : (left ? 1 : 2);
    if (p->status != 2 && p->status != 3)
        return;
    if (left && right &&
        (left_x < -10000 || right_x < -10000 || left_x > 10000 || right_x > 10000))
        return;

    play_sound(sounds[8], 1, 1);
    p->status = 0;
    p->sy = 0;
    p->y = floor_y - 1;
    p->x = left ? left_x + 11 : right_x - 11;
    p->rotate = 0;
}

/* Partial recovery of main.c, 0x4088c8..0x408d08.  This variant retries the
 * floor segment four pixels lower before transitioning to falling state. */
void handle_player_collision_vector_2(int lastX, int lastY)
{
    Tplayer *p;
    int floor_y, floor_x1, floor_x2;
    int left_x, left_y, right_x, right_y;
    int left, right;
    int current_x, current_y;
    int plx1, plx2, prx1, prx2;
    int col1, col2;

    p = ply[player_id];
    plx1 = (int)p->x - 11;
    current_x = (int)p->x;
    current_y = (int)p->y + 1;
    plx2 = lastX - 11;
    prx1 = (int)p->x + 11;
    prx2 = lastX + 11;
    col1 = makecol(255, 0, 0);
    col2 = makecol(255, 255, 0);
    floor_y = -12345678;
    floor_x1 = 0;
    floor_x2 = 0;
    getFloorData(&map, (int)p->y, &floor_y, &floor_x1, &floor_x2);
    if (floor_y == -12345678) {
        getFloorData(&map, lastY, &floor_y, &floor_x1, &floor_x2);
        if (floor_y == -12345678) {
            floor_y = 0;
            floor_x1 = 0;
            floor_x2 = 0;
        }
    }

    if (debug) {
        if (key[KEY_F2]) {
            line(screen, floor_x1, floor_y, floor_x2, floor_y, col1);
            line(screen, plx1, current_y, plx2, lastY, col2);
            line(screen, prx1, current_y, prx2, lastY, col2);
        }
    }
    left = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        plx1, current_y, plx2, lastY, &left_x, &left_y);
    right = line_intersect(floor_x1, floor_y, floor_x2, floor_y,
        prx1, current_y, prx2, lastY, &right_x, &right_y);
    if (!left && !right) {
        left = line_intersect(floor_x1, floor_y + 4, floor_x2, floor_y + 4,
            plx1, current_y, plx2, lastY, &left_x, &left_y);
        right = line_intersect(floor_x1, floor_y + 4, floor_x2, floor_y + 4,
            prx1, current_y, prx2, lastY, &right_x, &right_y);
    }
    if (!left && !right) {
        if (p->status == 2 || p->status == 0)
            p->status = 3;
        p->edge = 0;
        return;
    }
    p->edge = left == right ? 0 : (left ? 1 : 2);
    if (p->status != 2 && p->status != 3)
        return;

    play_sound(sounds[8], 1, 1);
    p->status = 0;
    p->sy = 0;
    p->y = floor_y - 1;
    p->x = left ? left_x + 11 : right_x - 11;
    p->rotate = 0;
}

/* Partial recovery of main.c, 0x408358..0x4088c8.  Combo mode uses ordinary
 * solid-foot correction first, then a floor-segment landing intersection. */
void handle_player_collision_combo(int lastX, int lastY)
{
    Tplayer *p;
    int fy1 = -12345678;
    int fx1 = 0, fx2 = 0;
    int ilx, ily, irx, iry;
    int solid1, solid2, left, right;
    int col1, col2;

    p = ply[player_id];
    col1 = makecol(255, 0, 0);
    col2 = makecol(255, 255, 0);
    solid1 = is_solid(&map, (int)p->x - 11, (int)p->y);
    solid2 = is_solid(&map, (int)p->x + 11, (int)p->y);
    any11 = solid1;
    any12 = solid2;
    any23 = 0;
    any22 = 0;
    any21 = 0;
    if (solid1 || solid2) {
        if (p->status == 1 || p->status == 2)
            return;
        if (p->status)
            play_sound(sounds[8], 1, 1);
        p->status = 0;
        p->sy = 0;
        if (solid1) {
            p->y -= solid1 - 9999;
            p->rotate = 0;
            p->edge = solid1 == solid2 ? 0 : 1;
            return;
        }
        p->y -= solid2 - 9999;
        p->rotate = 0;
        p->edge = 2;
        return;
    }

    if (p->status == 2 || p->status == 0)
        p->status = 3;
    getFloorData(&map, (int)p->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) {
            fy1 = 0;
            fx1 = 0;
            fx2 = 0;
        }
    }
    if (debug) {
        if (key[KEY_F2]) {
            line(screen, fx1, fy1, fx2, fy1, col1);
            line(screen, (int)p->x - 11, (int)p->y + 1, lastX - 11, lastY, col2);
            line(screen, (int)p->x + 11, (int)p->y + 1, lastX + 11, lastY, col2);
        }
    }
    left = line_intersect(fx1, fy1, fx2, fy1,
        (int)p->x - 11, (int)p->y + 1, lastX - 11, lastY, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy1,
        (int)p->x + 11, (int)p->y + 1, lastX + 11, lastY, &irx, &iry);
    if (!left && !right) {
        p->edge = 0;
        return;
    }
    p->edge = left == right ? 0 : (left ? 1 : 2);
    if (p->status != 2 && p->status != 3)
        return;

    play_sound(sounds[8], 1, 1);
    p->status = 0;
    p->sy = 0;
    p->y = fy1 - 1;
    p->x = left ? ilx + 11 : irx - 11;
    p->rotate = 0;
}

/* Oracle: main.c, 0x407fd8..0x408358.  The legacy mode first tests the
 * current feet, then sweeps a midpoint when the player moved downward. */
void handle_player_collision_old(int lastX, int lastY)
{
    int x, y, dx, dy;
    int solid1, solid2;

    /* 3246 */
    x = (int)ply[player_id]->x;
    dx = lastX - x;
    if (dx < 0) dx = -dx;
    /* 3247 */
    y = (int)ply[player_id]->y;
    dy = lastY - y;
    if (dy < 0) dy = -dy;
    /* 3248 */
    if ((int)ply[player_id]->x < lastX) x = lastX - dx / 2;
    /* 3249 */
    else x = lastX + dx / 2;
    /* 3250 */
    if ((int)ply[player_id]->y < lastY) y = lastY - dy / 2;
    /* 3251 */
    else y = lastY + dy / 2;

    /* 3253 */
    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    /* 3254 */
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    /* 3255 */
    any11 = solid1;
    /* 3256 */
    any12 = solid2;
    /* 3257 */
    any23 = 0;
    any22 = 0;
    any21 = 0;
    /* 3258 */
    if (solid1 + solid2 == 0) {
        /* 3259 */
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0)
            ply[player_id]->status = 3;
        /* 3270 */
        if (y <= lastY)
            return;
        goto sweep;
    }

resolve:
    /* 3260 */
    if (ply[player_id]->status == 1 || ply[player_id]->status == 2)
        return;
    /* 3261 */
    if (ply[player_id]->status)
        play_sound(sounds[8], 1, 1);
    /* 3262 */
    ply[player_id]->status = 0;
    /* 3263 */
    ply[player_id]->sy = 0;
    /* 3264 */
    if (solid1) {
        ply[player_id]->y -= solid1 - 9999;
        /* 3266 */
        ply[player_id]->rotate = 0;
        /* 3267 */
        if (solid1 == solid2)
            ply[player_id]->edge = 0;
        else
            ply[player_id]->edge = 1;
        return;
    }
    if (solid2) {
        ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        ply[player_id]->edge = 2;
        return;
    }
    ply[player_id]->rotate = 0;
    ply[player_id]->edge = 0;
    return;

sweep:
    /* 3271 */
    solid1 = is_solid(&map, x - 11, y);
    /* 3272 */
    solid2 = is_solid(&map, x + 11, y);
    /* 3273 */
    any21 = solid1;
    /* 3274 */
    any22 = solid2;
    /* 3275 */
    if (solid1 + solid2 == 0) {
        /* 3276 */
        if (ply[player_id]->status == 2 || ply[player_id]->status == 0)
            ply[player_id]->status = 3;
        return;
    }
    /* 3277 */
    if (ply[player_id]->status == 1 || ply[player_id]->status == 2)
        return;
    /* 3278 */
    any23 = 1;
    /* 3279 */
    if (ply[player_id]->status)
        play_sound(sounds[8], 1, 1);
    /* 3280 */
    ply[player_id]->status = 0;
    /* 3281 */
    ply[player_id]->sy = 0;
    /* 3282 */
    if (solid1) {
        ply[player_id]->y -= solid1 - 9999;
        /* 3284 */
        ply[player_id]->rotate = 0;
        /* 3285 */
        if (solid1 == solid2)
            ply[player_id]->edge = 0;
        else
            ply[player_id]->edge = 1;
        return;
    }
    /* 3283 */
    if (solid2) {
        ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        ply[player_id]->edge = 2;
        return;
    }
    /* 3286 */
    ply[player_id]->rotate = 0;
    ply[player_id]->edge = 0;
    return;
}

void handle_player_collision_original(int lastX, int lastY)
{
    int solid1;
    int solid2;

    solid1=is_solid(&map,(int)ply[player_id]->x-11,(int)ply[player_id]->y);
    solid2=is_solid(&map,(int)ply[player_id]->x+11,(int)ply[player_id]->y);
    any11=solid1;
    any12=solid2;
    any23=0;
    any22=0;
    any21=0;
    if (solid1+solid2==0) {
        if (ply[player_id]->status==2 || ply[player_id]->status==0)
            ply[player_id]->status=3;
        return;
    }
    if (ply[player_id]->status==1) return;
    if (ply[player_id]->status==2) return;
    if (ply[player_id]->status)
        play_sound(sounds[8],1,1);
    ply[player_id]->status=0;
    ply[player_id]->sy=0;
    if (solid1) {
        ply[player_id]->y-=solid1-9999;
        ply[player_id]->rotate=0;
        if (solid1==solid2) {
            ply[player_id]->edge=0;
            return;
        }
        ply[player_id]->edge=1;
        return;
    }
    if (solid2) {
        ply[player_id]->y-=solid2-9999;
        ply[player_id]->rotate=0;
        ply[player_id]->edge=2;
        return;
    }
    ply[player_id]->rotate=0;
    ply[player_id]->edge=0;
}

void fadeIn(BITMAP *bmp, int speed)
{
    int a;
    BITMAP *mybmp;

    mybmp=create_bitmap(SCREEN_W,SCREEN_H);
    for (a=255;a>0;a-=speed) {
        cycle_count=0;
        draw_sprite(mybmp,bmp,0,0);
        set_trans_blender(0,0,0,a);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(mybmp,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        blit_to_screen(mybmp);
        while (cycle_count <= 0) rest(2);
    }
    destroy_bitmap(mybmp);
}

void fadeOut(int speed)
{
    int a;
    BITMAP *bmp;

    bmp=create_bitmap(SCREEN_W,SCREEN_H);
    blit(screen,bmp,0,0,0,0,SCREEN_W,SCREEN_H);
    for (a=255;a>0;a-=speed) {
        cycle_count=0;
        draw_sprite(swap_screen,bmp,0,0);
        set_trans_blender(0,0,0,255-a);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        blit_to_screen(swap_screen);
        while (cycle_count<=0) rest(2);
    }
    destroy_bitmap(bmp);
    rectfill(screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
}

/* Partial recovery of main.c:3359, 0x4076c0..0x407a07.  This result panel
 * presents the score, floor, and best-combo categories and their markers. */
void draw_results(BITMAP *bmp, BITMAP *logo, int y, int *qualified,
                  int *qValues, int showQ)
{
    int categories[5] = { 0, 2, 1 };
    int numCats = 3;
    int padding = 30;
    int dist;
    int pos = 0;
    int i;

    draw_sprite(bmp,logo,320-logo->w/2,y);
    for (i=0; i<numCats; i++) {
        textprintf_ex(bmp,data[52].dat,200,y+logo->h+3+pos,-1,-1,"%s:",
                      category_names[categories[i]]);
        textprintf_right_ex(bmp,data[52].dat,440,y+logo->h+3+pos,-1,-1,"%d",
                            qValues[categories[i]]);
        if (showQ) {
            if (new_personal_best[categories[i]]>0 &&
                stricmp(profile->handle,"guest")) {
                draw_sprite(bmp,data[69].dat,476,y+logo->h+13+pos);
                dist = 18;
            }
            else
                dist = -4;
            if (qualified[categories[i]]>0) {
                draw_sprite(bmp,data[68].dat,480+dist,y+logo->h+13+pos);
                textprintf_ex(bmp,data[53].dat,480+dist+7,y+logo->h+18+pos,
                              makecol(0,0,0),-1,"%d",qualified[categories[i]]);
            }
        }
        pos += padding;
    }
}

int play(void)
{
    int playing;
    int old_map_pos;
    int level;
    int diff;
    int i;
    int quit;
    int scroll_acc;
    int scroll;
    int max_scroll;
    int speeds[9] = { 1500, 3000, 4500, 6000, 7500, 9000, 10500, 1800000, 9000000 };
    int next_speed;
    int next_aight;
    int allow_smpl;
    int game_over;
    int falling;
    int shake;
    int flash;
    int step_count;
    int next_floor;
    int play_again;
    int tot_scroll;
    int lastX;
    int lastY;
    int midX;
    int midY;
    int numComboJumps;
    int totComboFloors;
    int startTime;
    int endTime;
    int lastJumpLength;
    int oldUnlockedFloors;
    int current_rank_id;
    Tcontrol rec_ctrl;
    int time_cheat_count;
    clock_t clockTimeStart;
    clock_t clockTimeEnd;
    double clockElapsed;
    double totClockTimes;
    int qpc_start;
    int qpc_end;
    double qpc_elapsed;
    double totQPCTimes;
    int timeTimeStart;
    int timeTimeEnd;
    int timeElapsed;
    double totTimeTimes;
    int musicCounter;
    int lastMusicPos;
    float accMusics;
    int totMusics;
    LARGE_INTEGER li;
    int qpc_freq;
    /* The -0x928 stack slot holds falling in the game loop; original offset 11240
     * reloads it into esi for the results animation, where DWARF names it falling. */

    rec_ctrl = ctrl;                                   /* line 3439 */
    if (!itrcheck) {                                   /* line 3441 (else-branch placed inline by -O2) */
        oldUnlockedFloors = profile->best_floor / 100; /* line 3442 */
        current_rank_id = get_rank_id(profile);        /* line 3443; get_rank_id has no visible prototype
                                                          * in main.c (declared in profile.c taking
                                                          * Tprofile_rank *; that type isn't visible here,
                                                          * so the cast used by other callers is omitted) */
    } else {
        current_rank_id = 0;
        oldUnlockedFloors = 0;
    }
    if (recording) {                                   /* line 3473 */
        demo->tc_posts = 0;                             /* line 3474 */
        for (i = 0; i < 100; i++) {                     /* line 3475 */
            demo->tc_c_data[i] = 0.0f;                  /* line 3476 */
            demo->tc_q_data[i] = 0.0f;                  /* line 3477 */
            demo->tc_t_data[i] = 0.0f;                  /* line 3478 */
            demo->tc_s_data[i] = 0.0f;                  /* line 3479 */
            demo->tc_f_data[i] = 0.0f;                  /* line 3480 */
        }
    }
    log2file(" setting up play data");                 /* line 3485 */
    fall_count = 0;                                     /* line 3486 */
    clock_angle = 0;                                    /* line 3487 */
    map.offset = 0;                                     /* line 3488 */
    fast_forward = 0;                                   /* line 3490 */
    fast_fast_forward = 0;                               /* line 3491 */
    update_frame();                                      /* line 3493 */
    if (!itrcheck) {                                     /* line 3494 */
        draw_frame(swap_screen);                         /* line 3495 */
        fadeIn(swap_screen, 16);                         /* line 3497 */
        play_sound(custom.yo, 0, 0);                     /* line 3498 */
        startGameMusic();                                /* line 3499 */
    }
    if (!itrcheck && bg_beat) {                          /* line 3502-3503 (compiled as its own re-test
                                                            * of itrcheck, redundant with the block above) */
        checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1); /* line 3504 */
    }
    cycle_count = 0;                                     /* line 3510 */
    log2file(" play started");                           /* line 3512 */
    startTime = time(NULL);                              /* line 3513 */
    QueryPerformanceCounter(&li);                        /* line 3519 */
    qpc_start = li.LowPart;                              /* line 3520 */
    totMusics = 0;
    accMusics = 0.0f;
    lastMusicPos = 0;
    musicCounter = 0;
    time_cheat_count = 0;
    lastJumpLength = 0;
    endTime = 0;
    totComboFloors = 0;
    numComboJumps = 0;
    next_floor = -1;
    step_count = 0;
    shake = 0;
    falling = 0;
    game_over = 0;
    allow_smpl = 1;
    next_aight = 50;
    next_speed = 0;
    scroll = -1;
    quit = 0;
    QueryPerformanceFrequency(&li);                      /* line 3522; result unused here (its DWARF
                                                            * live range only starts at the second QPF
                                                            * call inside the loop, line 3610) */
    clockTimeStart = clock();                            /* line 3528 */
    timeTimeStart = time(NULL);                          /* line 3530 */
    playing = 1;                                         /* line 3533: the original enters the loop
                                                            * without testing playing (offset 351 jumps
                                                            * straight to the closeButtonClicked test),
                                                            * so it is known non-zero here; the value is
                                                            * constant-folded away and emits no code */

    while (playing && !closeButtonClicked) {   /* 3536 (3534 tests playing; both tests and both loop
                                                 * entry/back-edge copies compile to this one physical
                                                 * line, so its bytes are credited to the larger 3536) */
        cycle_count = 0;                                 /* line 3540 */
        logic_count++;                                   /* line 3542 */
        step_count++;                                    /* line 3543 */
        fall_count++;                                     /* line 3544 */
        time_cheat_count++;                               /* line 3545 */
        musicCounter++;                                   /* line 3546 */
        if (!itrcheck) {                                  /* line 3549 */
            if (hasFocus != lastFocus) {                  /* 3550 */
                if (hasFocus) {                            /* 3551 */
                    /* line 3552-3559: gaining focus, restart the background track */
                    if (bg_beat) {                          /* 3552 */
                        checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1); /* 3553 */
                    }
                    startGameMusic();                       /* 3559 */
                    totMusics = 0;                          /* 3559 */
                    accMusics = 0.0f;                       /* 3559 */
                    musicCounter = 0;                       /* 3559 */
                } else {
                    /* line 3562-3567: losing focus, stop the background track */
                    if (checkMusicVoiceID >= 0) {           /* 3562 */
                        voice_stop(checkMusicVoiceID);      /* 3563 */
                    }
                    checkMusicVoiceID = -1;                 /* 3565 */
                    stopGameMusic();                        /* 3567 */
                }
            }
            lastFocus = hasFocus;                         /* line 3569 */
        }
        if (!itrcheck) {                                  /* line 3574 (compiled as its own re-test of
                                                             * itrcheck, redundant with the block above) */
            if (checkMusicVoiceID >= 0) {                 /* line 3574 */
                int vgp;   /* DWARF block 132257 [760..886]: vgp (int), a (float), b (float) */
                float a, b;

                vgp = voice_get_position(checkMusicVoiceID); /* line 3575 */
                if (vgp < lastMusicPos) {                 /* line 3576 */
                    musicCounter = 0;
                }
                /* lines 3580-3585: running average of a 44000/vgp "speed" ratio into accMusics,
                 * gated on that ratio being > 0.01 (x87 fucompp/fnstsw/test $0x45 idiom); see report
                 * for the derivation of the comparison direction. The two named DWARF temps a/b hold
                 * the ratio and the scaled increment across lines 3580-3584. */
                a = 44000.0f / vgp;                       /* 3580 */
                if (a > 0.01) {                           /* line 3583: original fldl */
                    b = a / (musicCounter / 50.0f);       /* line 3584: the original computes the intermediate
                                                            * musicCounter/50.0 first (fidivrl -0x938, a REVERSE
                                                            * divide of the int by the ST0-resident 50.0) and then
                                                            * divides 'a' by that -- algebraically a*50.0f/musicCounter,
                                                            * but this operand order is what reproduces fidivrl */
                    accMusics += b;
                    totMusics++;                          /* line 3585 */
                }
                lastMusicPos = vgp;                       /* line 3585 (tail) */
            }
        }
        if (recording && map.offset > 100 && !ply[player_id]->dead) { /* line 3595 */
            if (time_cheat_count == 1000) {                          /* 3597 */
                /* lines 3604-3661: periodic clock()/QueryPerformanceCounter()/time() cross-check,
                 * recorded into the demo replay's time-cheat-detection arrays. DWARF lexical block
                 * 134356 (3604..3661) declares three doubles our source previously never named:
                 * clockSpeed, qpcSpeed, timeSpeed -- the per-method rate used to gate/derive each
                 * tot*Times value below. The exact x87 formulas are a best-effort reconstruction
                 * from the instruction stream (see report); the calls and field targets are
                 * evidenced directly. */
                double clockSpeed;  /* DWARF block 134356 [3604..3661] */
                double qpcSpeed;
                double timeSpeed;

                clockTimeEnd = clock();                             /* line 3604 */
                clockElapsed = clockTimeEnd - clockTimeStart;       /* line 3605 */
                clockSpeed = (50.0 * clockElapsed) / 1000.0;        /* line 3605 (tail); confirmed against
                                                                      * the real toolchain: this direction
                                                                      * (not 1000.0/(50*clockElapsed)) is what
                                                                      * actually emits fdivr %st,%st(1) -- the
                                                                      * reciprocal form silently emits the same
                                                                      * byte count via plain fdiv, which is why
                                                                      * the swap wasn't caught by bytes alone. */
                if (clockSpeed > 0.0) {                             /* 3606 */
                    totClockTimes = 1000.0 / (1000.0 * clockSpeed) / 20.0; /* 3606: reuses the same
                                                                      * 1000.0 already resident from 3605
                                                                      * (GCC folds this to a single flds,
                                                                      * confirmed via the real toolchain) --
                                                                      * this is 1.0/clockSpeed/20.0, a genuine
                                                                      * rate (1/clockElapsed), not a constant. */
                } else {
                    totClockTimes = -0.05;                          /* 3606 (== -1.0/20.0, guard-fail sentinel) */
                }
                QueryPerformanceFrequency(&li);                     /* line 3610 */
                qpc_freq = li.LowPart;                               /* line 3611 */
                QueryPerformanceCounter(&li);                       /* line 3612 */
                qpc_end = li.LowPart;                               /* line 3612 tail */
                qpc_elapsed = qpc_end - qpc_start;                  /* line 3615 */
                qpcSpeed = 20.0 / (50.0 * qpc_elapsed / qpc_freq);  /* line 3615 (tail) */
                totQPCTimes = qpcSpeed;                             /* line 3615 (tail) */
                timeTimeEnd = time(NULL);                           /* line 3623 */
                timeElapsed = timeTimeEnd - timeTimeStart;          /* line 3638 */
                timeSpeed = 20.0 / (50.0 * timeElapsed);            /* line 3638 (tail) */
                totTimeTimes = timeSpeed;                           /* line 3638 (tail) */
                demo->tc_c_data[demo->tc_posts] = totClockTimes;    /* line 3636 */
                demo->tc_q_data[demo->tc_posts] = totQPCTimes;      /* line 3637 */
                demo->tc_t_data[demo->tc_posts] = totTimeTimes;     /* line 3638 (tail) */
                demo->tc_f_data[demo->tc_posts] = ply[player_id]->level; /* line 3639 */
                if (totMusics != 0) {                               /* line 3640 */
                    demo->tc_s_data[demo->tc_posts] = 50.0 * accMusics / totMusics; /* line 3641 */
                }
                if (demo->tc_posts <= 97) {                         /* line 3643 */
                    demo->tc_posts++;                               /* line 3643 */
                }
                clockTimeStart = clock();                           /* line 3654 */
                QueryPerformanceCounter(&li);                       /* line 3656 */
                qpc_start = li.LowPart;                             /* line 3657 */
                timeTimeStart = time(NULL);                         /* line 3661 */
                totMusics = 0;
                accMusics = 0.0f;
                time_cheat_count = 0;
            }
        }
        if (debug) {                                      /* line 3681 */
            /* lines 3682-3691: ten combo-length reward tiers, keyed to the number-row keys */
            if (key[KEY_1]) { if (allow_smpl) start_reward(5); }      /* 3682 */
            if (key[KEY_2]) { if (allow_smpl) start_reward(7); }      /* 3683 */
            if (key[KEY_3]) { if (allow_smpl) start_reward(15); }     /* 3684 */
            if (key[KEY_4]) { if (allow_smpl) start_reward(25); }     /* 3685 */
            if (key[KEY_5]) { if (allow_smpl) start_reward(35); }     /* 3686 */
            if (key[KEY_6]) { if (allow_smpl) start_reward(50); }     /* 3687 */
            if (key[KEY_7]) { if (allow_smpl) start_reward(70); }     /* 3688 */
            if (key[KEY_8]) { if (allow_smpl) start_reward(100); }    /* 3689 */
            if (key[KEY_9]) { if (allow_smpl) start_reward(140); }    /* 3690 */
            if (key[KEY_0]) { if (allow_smpl) start_reward(200); }    /* 3691 */
            /* line 3692 */
            allow_smpl = !(key[KEY_1] || key[KEY_2] || key[KEY_3] ||
                           key[KEY_4] || key[KEY_5] || key[KEY_6] ||
                           key[KEY_7] || key[KEY_8] || key[KEY_9] ||
                           key[KEY_0]);
        }
        midX = (int)ply[player_id]->x;                    /* line 3698 */
        midY = (int)ply[player_id]->y;                    /* line 3699 */

        handle_player_input(&ctrl);                                    /* 3702 */
        update_player(ply[player_id]);                                 /* 3703 */
        if (!itrcheck) {                                                /* 3706 */
            if (ply[player_id]->rotate && ply[player_id]->in_combo && options.flash) {  /* 3707 */
                create_particle(stars, (int)ply[player_id]->x, (int)ply[player_id]->y - 16);  /* 3708 */
            } else {
                for (i = 0; i < 512; i++) {                             /* 3711 */
                    if (stars[i].intensity)                             /* 3711 */
                        update_particle(&stars[i]);                     /* 3711 */
                }
            }
            /* 3717..3732: fall-height "shake" accumulator. old_map_pos captures map.offset
             * before this update (ebx, live only in a register -- DWARF gives it no stack
             * slot, matching "never written" in the skeleton check: it is read once, right
             * back out). The bracket ladder (thresholds 160/140/120/100/80/60/40/20/0) builds
             * scroll_acc (DW_OP_reg1/ecx across this whole span) as a running total, not a
             * mutually-exclusive choice: each of the lower 7 thresholds (jne skip-just-the-
             * add-and-fall-through, evidenced at 3722..3728) adds its delta on top of
             * whichever base the first (140) test picked, and the y>=0 test (fldz/fucompp,
             * 3728) is unconditionally true for a valid y -- kept literal per the evidenced
             * compare, see report. map.offset, y and level are then updated once from
             * old_map_pos + scroll_acc (the single store at 3729/3731/3732), not per bracket. */
            if (ply[player_id]->y < 160.0) {                            /* 3719 */
                old_map_pos = map.offset;                                /* 3717 (evidence: read here) */
                scroll_acc = (ply[player_id]->y >= 140.0) ? 2 : 1;       /* 3721 */
                if (ply[player_id]->y >= 120.0)                         /* 3722 */
                    scroll_acc++;                                       /* 3722 */
                if (ply[player_id]->y >= 100.0)                         /* 3723 */
                    scroll_acc++;                                       /* 3723 */
                if (ply[player_id]->y >= 80.0)                          /* 3724 */
                    scroll_acc++;                                       /* 3724 */
                if (ply[player_id]->y >= 60.0)                          /* 3725 */
                    scroll_acc++;                                       /* 3725 */
                if (ply[player_id]->y >= 40.0)                          /* 3726 */
                    scroll_acc += 2;                                    /* 3726 */
                if (ply[player_id]->y >= 20.0)                          /* 3727 */
                    scroll_acc += 2;                                    /* 3727 */
                if (ply[player_id]->y >= 0.0)                           /* 3728; ? always true for a valid y */
                    scroll_acc += 3;                                    /* 3728 */
                map.offset = old_map_pos + scroll_acc;                  /* 3729 */
                ply[player_id]->y += scroll_acc;                        /* 3731 */
                level = midY + scroll_acc;                              /* 3732: midY's own DWARF location
                                                                          * list (DW_OP_reg7/edi) is live
                                                                          * 3471..3641, i.e. continuously
                                                                          * through the whole scroll_acc
                                                                          * ladder above and up to the final
                                                                          * "lea (%ecx,%edi,1),%edi; mov
                                                                          * %edi,-0x92c(%ebp)" at offset
                                                                          * 3638..3641 -- edi is never
                                                                          * reloaded from level's own slot
                                                                          * first, so this is an overwrite
                                                                          * from midY+scroll_acc, not level's
                                                                          * old value incremented (contrast
                                                                          * the other branch's 3754, which is
                                                                          * "add %esi,-0x92c(%ebp)", a true
                                                                          * increment of the existing level). */
                tot_scroll = scroll_acc;                                /* 3732: shares ecx with scroll_acc through
                                                                          * the collision switch below (evidence:
                                                                          * DW_OP_reg1 live range extends to 3823) */
            }
            if (!ply[player_id]->dead)                                  /* 3736 */
                clock_angle++;                                          /* 3736 */
            /* 3738..3758: proceed only once the shake accumulator has built up and the
             * player is alive; otherwise reset clock_angle/fall_count (only while alive). */
            if (map.offset <= 100 || ply[player_id]->dead) {            /* 3738 */
                if (!ply[player_id]->dead) {                            /* 3757 */
                    clock_angle = 0;                                    /* 3758 */
                    fall_count = 0;                                     /* 3758 */
                }
            } else {
                if (scroll == -1)                                       /* 3739 */
                    scroll = start_speeds[demo->start_speed];           /* 3740 */
                if (scroll) {                                            /* 3742 */
                    map.offset += scroll;                               /* 3751 */
                    tot_scroll += scroll;                               /* 3752 (local_slot_trace: read+add
                                                                          * of the ecx slot shared with
                                                                          * scroll_acc/tot_scroll above) */
                    ply[player_id]->y += scroll;                        /* 3753 */
                    level += scroll;                                    /* 3754 */
                } else if (step_count & 1) {                            /* 3743 */
                    map.offset++;                                       /* 3744 */
                    tot_scroll++;                                       /* 3745 (same slot, mirrors 3752) */
                    ply[player_id]->y += 1.0;                           /* 3746 */
                    level++;                                            /* 3747 */
                }
            }
            any13 = map.offset;                                         /* 3763 */
            if ((unsigned)(hurry_y + 99) <= 578u)                        /* 3765 */
                hurry_y -= 2;                                           /* 3765 */
            if (demo->speed_increase) {                                 /* 3766 */
                /* 3767 */
                if (!ply[player_id]->dead &&
                    speeds[next_speed] < fall_count &&
                    scroll > 4) {
                    ply[player_id]->ccc[next_speed] = ply[player_id]->level;  /* 3768 */
                    next_speed++;                                       /* 3770 */
                    scroll++;                                           /* 3771 */
                    hurry_y = 477;                                      /* 3772 */
                    play_sound(speaker[0], 0, 0);                       /* 3773 */
                    play_sound(sounds[4], 0, 0);                        /* 3774 */
                }
            }
            if (scroll == 5) {                                          /* 3778 */
                fall_count -= 45;                                       /* 3779 */
                if (!ply[player_id]->dead)                              /* 3780 */
                    clock_angle -= 45;                                  /* 3780 */
            }
            /* old_map_pos is loaded into %ebx once at 3717 ("mov 0x4f8e18,%ebx") and is
             * never redefined before this point, so it still holds the pre-update
             * map.offset snapshot here; %eax is a fresh read of map.offset (offset
             * 1729/4266, "mov 0x4f8e18,%eax"). Fragments 1717..1753 do the signed
             * mod-16 (and $0x8000000f + js/dec/or/inc fixup) on both, then
             * "cmp %eax,%ebx; jle 0xb88" skips add_floor to the cold code at
             * offset 2952, which is line 3787's "cmp $0xf,%ecx; jg 0x6d9" (%ecx is
             * scroll_acc, per its DW_OP_reg1 location over 1611..1765): jg jumps
             * forward into the call at 1753, and falling through duplicates line
             * 3814's test, i.e. skips the call. So the two tests are an OR: the
             * mod-16 wrap check runs first, and only when it is false does the
             * scroll_acc>15 check get evaluated (matching the jle/jg short-circuit
             * order below). */
            if (old_map_pos % 16 > map.offset % 16                        /* 3783 */
                || scroll_acc > 15)                                        /* 3787 */
                add_floor(&map);                                           /* 3789 */
        }

        /* lastY shares level's stack slot (-0x92c(%ebp)/-2348 in the DWARF dump); no
         * separate store to that address exists between the level updates above and
         * the switch below, so the switch's second argument is simply level's
         * current value carried over under a different DWARF name. */
        lastY = level;                                                  /* 3814: evidence: shared slot, no distinct write found */

        switch (collision_type) {                                       /* 3814 */
        case 3:
            handle_player_collision_original(midX, lastY);               /* 3815 */
            break;
        case 2:
            handle_player_collision_old(midX, lastY);                    /* 3817 */
            break;
        case 1:
            handle_player_collision_vector(midX, lastY);                 /* 3819 */
            break;
        case 0:
            handle_player_collision_vector_2(midX, lastY);               /* 3821 */
            break;
        case 4:
            handle_player_collision_combo(midX, lastY);                  /* 3823 */
            break;
        default:
            allegro_message("unknown collision type");                  /* 3826 */
            break;
        }

        if (ply[player_id]->rotate)                                     /* 3833 */
            ply[player_id]->angle += 0x80000;                           /* 3833 */
        if (ply[player_id]->in_combo) {                                 /* 3837 */
            ply[player_id]->in_combo--;                                 /* 3838 */
            if (!ply[player_id]->in_combo && ply[player_id]->acc_jumps > 1) {  /* 3839 */
                int rewResult;
                Tgd_combo c;

                ply[player_id]->score += ply[player_id]->acc_level * ply[player_id]->acc_level;  /* 3841 */
                rewResult = start_reward(ply[player_id]->acc_level);     /* 3842 */
                if (recording && !is_playing_custom_game)                /* 3843 */
                    profile->rewards[rewResult]++;                       /* 3843 */
                totComboFloors += ply[player_id]->acc_level;             /* 3844 */
                numComboJumps++;                                         /* 3845 */
                c.length = ply[player_id]->acc_level;                    /* 3848 */
                c.start = gdComboStart;                                  /* 3849 */
                c.end = gdComboStart + ply[player_id]->acc_level;        /* 3850 */
                add_combo(gameData, &c);                                 /* 3851 */
                ply[player_id]->latest_combo = ply[player_id]->acc_level;  /* 3853 */
                if (ply[player_id]->acc_level > ply[player_id]->best_combo)  /* 3854 */
                    ply[player_id]->best_combo = ply[player_id]->acc_level;  /* 3855 */
            }
        }

        if (ply[player_id]->status) {                                   /* 3862 */
            level = (get_level(&map, (int)ply[player_id]->y) - 1) / 10;  /* 3864 */
            diff = level - ply[player_id]->level;                       /* 3869 */
            if (diff != 0) {                                             /* 3870 */
                if (diff == gdLastJumpDiff) {                            /* 3871 */
                    jumpSequence.num++;                                  /* 3882 */
                } else {
                    jumpSequence.dist = gdLastJumpDiff;                  /* 3874 */
                    add_jump_sequence(gameData, &jumpSequence);          /* 3875 */
                    jumpSequence.num = 1;                                /* 3878 */
                    jumpSequence.start = ply[player_id]->level;          /* 3879 */
                }
                gdLastJumpDiff = diff;                                  /* 3885 */
            }
            if (level >= ply[player_id]->level) {                       /* 3891 */
                diff = level - ply[player_id]->level;                   /* 3893 */
                if (diff != lastJumpLength)                             /* 3896 */
                    lastJumpLength = 0;                                 /* 3897 */
                for (i = 0; i < 5; i++) {                                /* 3897 */
                    if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])   /* 3900 */
                        ply[player_id]->jcTop[i] = ply[player_id]->jc[i];  /* 3901 */
                    ply[player_id]->jc[i] = 0;                             /* 3904 */
                }
                if (diff > 0) {                                          /* 3911 */
                    if (diff <= 5)                                       /* 3912 */
                        ply[player_id]->jc[diff - 1]++;                  /* 3913 */
                    if (diff != 1) {                                     /* 3919: dec+je on diff (offset
                                                                            * 6071/6072) -- the diff==1 case
                                                                            * jumps straight to offset 15650,
                                                                            * bypassing this whole block. */
                        if (ply[player_id]->in_combo) {                  /* 3920 */
                            ply[player_id]->acc_level += diff;           /* 3921 */
                            ply[player_id]->acc_jumps++;                 /* 3922 */
                        } else {
                            ply[player_id]->acc_level = diff;            /* 3926 */
                            ply[player_id]->acc_jumps = 1;               /* 3927 */
                        }
                        ply[player_id]->in_combo = 100;                  /* 3923 */
                        lastJumpLength = diff;                          /* 3932: shared tail for both arms
                                                                            * above -- offset 6120..6150,
                                                                            * reached by fallthrough from the
                                                                            * in_combo arm (offset 6113..6120)
                                                                            * and by "jmp 4131e8" from the
                                                                            * else arm (offset 7570), which
                                                                            * targets that same offset 6120.
                                                                            * local_slot_trace confirms this
                                                                            * is the ONLY write of diff into
                                                                            * lastJumpLength's slot. */
                    } else {
                        lastJumpLength = 1;                              /* 3928: slot trace shows a literal
                                                                            * $0x1 store at offset 15650,
                                                                            * reached only via 3919's diff==1
                                                                            * jump (offset 6072 je 415722 =
                                                                            * offset 15650) -- unconditional
                                                                            * on diff==1, before the in_combo
                                                                            * test below. */
                        if (ply[player_id]->in_combo)                   /* 3932: in_combo test at offset
                                                                            * 3897 (cmpl $0x0,0x40(%eax)),
                                                                            * reached here via the jmp at
                                                                            * offset 15660. */
                            ply[player_id]->in_combo = 1;                /* 3933: store, evidenced after
                                                                            * the test at offset 3903 */
                    }
                }
                /* 3910..3923 reloads player_id/ply[player_id] for this next statement's test,
                 * not a re-test of the line-3932 condition. */
                if (!ply[player_id]->in_combo)                           /* 3936 */
                    gdComboStart = level;                                /* 3937 */
            }

            if (ply[player_id]->in_combo) {                              /* 3943 */
                ply[player_id]->in_combo = 1;                            /* 3943 (same DWARF row, offset
                                                                            * 4346, as the test at 4339) */
                for (i = 0; i < 5; i++) {                                /* 3945 */
                    if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])   /* 3948 */
                        ply[player_id]->jcTop[i] = ply[player_id]->jc[i];  /* 3949 */
                    ply[player_id]->jc[i] = 0;                             /* 3952 */
                }
                lastJumpLength = 0;                                     /* 3945: slot trace shows a second
                                                                            * write to lastJumpLength's slot
                                                                            * here (offset 4454), missing from
                                                                            * this block until now. */
                ply[player_id]->level = level;                          /* 3962 */
                if (!numComboJumps &&                                    /* 3967 */
                    ply[player_id]->no_combo_top_floor < ply[player_id]->level)
                    ply[player_id]->no_combo_top_floor = gdComboStart;   /* 3969 */
            }
        }

        /* 3976..3999 and on into W3's 4000..4004: ONE `if` whose body crosses the region
         * boundary, so W2 deliberately leaves its brace open and W3 closes it and writes the
         * else arm.  Evidence: both `jne`s of line 3976's own fragment, at offsets 2035 and
         * 2046, jump to offset 2940, which is `mov $0xffffffff,%esi; jmp 412300`, and
         * 0x412300 is offset 2304, the first instruction of line 4010.  So the outer test has
         * only the two terms; `in_combo && acc_jumps > 1` is an inner `if` guarding the single
         * 3979 store (its own `je`/`jle` at 2068 and 2074 target offset 2094, line 3981, not
         * 2940); the body runs on through `add_jump_sequence` and the 4003/4004 block; 4010 is
         * the merge point; and the else arm is `playing = -1`.  `esi` is `playing`: play's
         * DWARF location list puts `playing` in esi from offset 2945, five bytes after that
         * store, while `falling`'s own ranges do not start until offset 11246.  Line 3977 is
         * the same variable, not `flash`: `cmpl $0x1,itrcheck; sbb %esi,%esi` yields -1 when
         * itrcheck is 0 and 0 otherwise. */
        if (ply[player_id]->y < 540.0 && !ply[player_id]->dead) {        /* 3976 */
            playing = (itrcheck < 1) ? -1 : 0;                           /* 3977 */
            if (ply[player_id]->in_combo && ply[player_id]->acc_jumps > 1)   /* 3978 */
                ply[player_id]->biggest_lost_combo = ply[player_id]->acc_level;  /* 3979 */
            ply[player_id]->in_combo = 0;                                /* 3981 */
            ply[player_id]->dead = 1;                                    /* 3982 */
            play_sound(custom.falling, 0, 1);                            /* 3983 */
            endTime = time(0);                                           /* 3985 */
            for (i = 0; i < 5; i++) {                                    /* 3988 */
                if (ply[player_id]->jc[i] <= ply[player_id]->jcTop[i])       /* 3991 */
                    ply[player_id]->jcTop[i] = ply[player_id]->jc[i];        /* 3992 */
                ply[player_id]->jc[i] = 0;                                   /* 3995 */
            }
            jumpSequence.dist = gdLastJumpDiff;                          /* 3999 */
        /* brace intentionally left open: W3 closes it after the 4003/4004 block */

            add_jump_sequence(gameData, &jumpSequence);                             /* 4000 */
                /* falling lives in the function-level slot ebp-0x928 that the line-3520 reset
                 * block zeroes, so it is declared with play's other locals, not here: as a block
                 * local re-initialised every iteration GCC could prove it never passed 250 and
                 * deleted the whole 4016..4019 body.  Both arms fall through unconditionally into
                 * the y<900 combo body below (traced from the tail-duplicated machine code at
                 * offsets 2266..2304 / 5797..5848: the "lastJumpLength = 0; falling = 1;" pair
                 * is machine-duplicated into BOTH arms -- the no_combo_top_floor update is the
                 * only part actually gated). */

                if (numComboJumps) {                                               /* 4003 */
                    lastJumpLength = 0;                                             /* 4003 */
                    falling = 1;                                                 /* 4003 */
                } else {
                    if (ply[player_id]->no_combo_top_floor < ply[player_id]->level) /* 4003 */
                        ply[player_id]->no_combo_top_floor = ply[player_id]->level; /* 4004 */
                    lastJumpLength = 0;                                             /* 4004 */
                    falling = 1;                                                 /* 4004 */
                }
            /* The `if (y < 540.0 && !dead)` opened in W2 at line 3976 closes here: both of its
             * `jne`s jump to offset 2940, `mov $0xffffffff,%esi; jmp 412300`, and 0x412300 is
             * offset 2304, the first instruction of line 4010.  So `add_jump_sequence` and the
             * 4003/4004 block are inside that `if`, the else arm is this single store, and 4010 is
             * where the two paths merge.  With this edge in place `falling` is no longer 1 on
             * every path into 4015, which is what let GCC prove 4016's `> 250` test false and
             * delete 4016..4019 outright. */
        } else {
            playing = -1;                                                       /* 4004 */
        }
        if (ply[player_id]->y < 900.0 && !game_over) {                      /* 4010 */
            play_sound(speaker[1], 0, 0);                                   /* 4012 */
            game_over = 2;                                                  /* 4012 */
        }
        if (falling)                                                     /* 4015 */
            falling++;                                                   /* 4015 */
        if (falling > 250 && falling <= ply[player_id]->level * 5) {   /* 4016 */
            play_sound(sounds[6], 1, 0);                                    /* 4017 */
            if (custom.falling)                                            /* 4018 */
                stop_sample(custom.falling);                               /* 4019 */
            /* 4022 is INSIDE this block, not after it: line 4016's own `jle` at offset
             * 2373 jumps to offset 2453, past both of these stores, while 4018's `je` at
             * 2411 jumps to 2421, the first of them.  With the reset conditional the
             * counter accumulates across frames, which is what makes the `> 250` test
             * reachable at all; with it unconditional the counter is 0 or 2 on every path
             * and GCC deletes 4016..4022 outright. */
            ply[player_id]->shake = 0x18;                                   /* 4022 */
            falling = 0;                                                 /* 4022 */
        }
        if (next_aight > ply[player_id]->level) {                           /* 4027 */
            play_sound(sounds[2], 0, 0);                                    /* 4028 */
        }
        if (!options.flash) {                                                  /* 4029 */
            /* midX = next_aight / 2 is evaluated as the loop bound: the shr/add/sar division
             * (rounding toward zero) is credited to main.c:4029 itself (the compiler folds it
             * into the for-init/condition), while the spill store to midX's own stack slot
             * (DWARF -0x940(%ebp)) lands on main.c:4031, the loop body's first real statement
             * (source-view 4000..4055: fragments 3987..4024 tagged 4029, 4024..4036 tagged
             * 4031) -- so the assignment and the loop share one combined-init statement. */
            for (i = 0, midX = next_aight / 2;                                 /* 4029 */
                 i < midX;                                                     /* 4124 */
                 i++) {                                                        /* 4123 */
                int p;                                                          /* 4031 block-local */
                p = create_particle(stars, (new_rand() % 600) + 20, 480);       /* 4030 */
                stars[p].sy = -(((new_rand() % 200) << 16) / 5);                /* 4031 */
            }
        }
        if (next_aight > 999)                                                  /* 4033 */
            next_aight += 500;                                                 /* 4034 */
        else
            next_aight += 50;                                                  /* 4037 */
        if (ply[player_id]->edge == 0)                                        /* 4042 */
            ply[player_id]->edge_drawn = 0;                                   /* 4042 */
        if (ply[player_id]->edge_drawn != 0) {                                 /* 4043 */
            if (ply[player_id]->edge_drawn == 11 && ply[player_id]->status == 0) /* 4044 */
                play_sound(custom.edge, 1, 1);                                 /* 4044 tail */
            if (ply[player_id]->edge_drawn == 50)                              /* 4045 */
                ply[player_id]->edge_drawn = 0;                                /* 4046 */
        }
        if (!debug) {                                                          /* 4049 */
            if (recording && ply[player_id]->dead > 100)                       /* 4056 */
                playing = 0;                                                  /* 4056 */
        } else if (ply[player_id]->dead <= 99) {                               /* 4050 */
            /* esi is confirmed as `playing` here (its DWARF range covers offsets 2860..2903,
             * exactly this store). Re-measured after `playing` gained its first real assignments
             * this round (the y<540/dead edge and the KEY_SPACE/KEY_RIGHT restructure): 4050 now
             * measures 29 of 22 historical bytes and 4056 32 of 44 -- together 61 of 66, so the
             * two `playing = 0;` epilogues (this one and 4056's) are no longer degenerate; the
             * remaining spread is consistent with the two still sharing code the historical
             * binary kept separate, not a missing statement. */
            playing = 0;                                                     /* 4050 */
        }
        if (!itrcheck && key[KEY_F1]) {                                        /* 4062 */
            int pauseTime, addTime; /* DWARF block 132550 [4560..4780]: pauseTime, addTime */
            pauseTime = time(NULL);                                            /* 4063 */
            take_screenshot(swap_screen);                                      /* 4064 */
            while (key[KEY_F1]) { }                                            /* 4065: self-target waits for release */
            addTime = time(NULL) - pauseTime;                                  /* 4066 */
            if (addTime > 0)                                                   /* 4067 */
                startTime += addTime;                                          /* 4068 */
            if (checkMusicVoiceID >= 0)                                       /* 4075 */
                musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); /* 4077 */
            clockTimeStart = clock();                                          /* 4083 */
            QueryPerformanceCounter(&li);                                      /* 4085 */
            qpc_start = li.LowPart;                                            /* 4086 */
            timeTimeStart = time(NULL);                                        /* 4090 */
            lastMusicPos = 0;                                                  /* 4077: folded into the
                                                                                   musicCounter statement's own
                                                                                   fragment (offsets 4697..4709,
                                                                                   right after the fistpl), not a
                                                                                   separate line-table row */
            accMusics = 0.0;                                                   /* 4077 */
        }
        if (ply[player_id]->shake) {                                          /* 4094 */
            ply[player_id]->shake--;                                          /* 4095 */
            shake = new_rand() % 8;                                           /* 4097 */
        }
        update_frame();                                                       /* 4100 */
        if (!quit && closeButtonClicked) {                                    /* 4104 */
            quit = 1;                                                        /* 4104 */
            playing = 0;                                                     /* 4104 */
        }
        if (recording) {                                                      /* 4109 */
            if (key[KEY_ESC]) {                                               /* 4110 */
                if (ply[player_id]->dead) {                                   /* 4111 */
                    log2file("  player quit after dying");                    /* 4112 */
                    playing = 0;                                              /* 4112 */
                } else {
                    /* REGION W3a: ESC pause screen, lines 4117..4182 */
                    int pauseTime, fc, ca, addTime; /* block-scoped DWARF locals (block 132596), not in the 52-local skeleton */

                    pauseTime = time(NULL);                                   /* 4117 */
                    fc = fall_count;                                          /* 4118 */
                    ca = clock_angle;                                         /* 4119 */
                    log2file("  game paused with esc");                      /* 4120 */
                    for (i = 0; i < 640; i += 2) {                            /* 4122 */
                        vline(swap_screen, i, 0, 480, 0);                     /* draw.inl:46 */
                        hline(swap_screen, 0, i, 640, 0);                     /* draw.inl:54 */
                    }
                    textout_centre_ex(swap_screen, data[50].dat,               /* 4126: GCC attributes a call's
                                                                                  bytes to its opening line, so the
                                                                                  annotation moves here (was on the
                                                                                  closing line, which left the
                                                                                  line_budget tool crediting each
                                                                                  call's bytes to the call above it
                                                                                  and showing 4128 as 0 bytes even
                                                                                  though the call is present). */
                                       "DO YOU REALLY WANT TO EXIT?", 320, 160, -1, -1);
                    textout_centre_ex(swap_screen, data[52].dat,               /* 4127 */
                                       "Press any key to resume", 320, 210, -1, -1);
                    textout_centre_ex(swap_screen, data[52].dat,               /* 4128 */
                                       "Press ESC to exit", 320, 240, -1, -1);
                    blit_to_screen(swap_screen);                              /* 4129 */
                    play_sound(custom.wazup, 0, 1);                           /* 4130 */
                    /* Two-loop wait, re-derived from the assembly (source-view 4117..4182):
                     * outer loop's entry jmp lands on its is_any/is_pause/ESC compound test
                     * (offsets 1717.. no -- offsets 5357/5316 etc, see function_lines source-view),
                     * a plain bottom-tested `while (cond) body`; the inner "key still held" loop's
                     * entry jmp lands on keypressed() alone (offset 5490), meaning keypressed() is
                     * the sole loop condition and the is_any/is_pause/closeButtonClicked/key[ESC]
                     * checks are an if-break inside its body -- GCC then thread the break's three
                     * different truth cases into different entry points of the loop that follows
                     * (closeButtonClicked jumps straight past that loop's own redundant
                     * closeButtonClicked test; key[ESC] jumps into its is_pause call for the same
                     * reason), which is why the reconstruction only needs three plain loops. */
                    poll_control(&ctrl, 0);                                    /* 4132 */
                    while (is_any(&ctrl) || is_pause(&ctrl)                    /* 4133 */
                           || (!closeButtonClicked && key[KEY_ESC])) {         /* 4133 */
                        poll_control(&ctrl, 0);                                /* 4134 */
                        rest(2);                                               /* 4135 */
                    }
                    clear_keybuf();                                            /* 4137 */
                    while (!keypressed()) {                                    /* 4138 */
                        if (is_any(&ctrl) || is_pause(&ctrl) ||
                            closeButtonClicked || key[KEY_ESC])                /* 4138 */
                            break;                                            /* 4138 */
                        poll_control(&ctrl, 0);                                /* 4139 */
                        rest(2);                                               /* 4140 */
                    }
                    while (!closeButtonClicked && is_pause(&ctrl)) {           /* 4142 */
                        poll_control(&ctrl, 0);                                /* 4143 */
                        rest(2);                                               /* 4144 */
                    }
                    if (key[KEY_ESC]) {                                       /* 4146 */
                        log2file("  game quit from esc pause");               /* 4150 */
                        profile->games_quit++;                                /* 4151 */
                        endTime = time(NULL);                                 /* 4152 */
                        quit = 1;                                             /* 4152 */
                        playing = 0;                                          /* 4152 */
                    }
                    clear_keybuf();                                           /* 4154 */
                    fall_count = fc;                                          /* 4156 */
                    clock_angle = ca;                                         /* 4157 */
                    log2file("  game unpaused");                              /* 4158 */
                    addTime = time(NULL) - pauseTime;                         /* 4159 */
                    if (addTime > 0)                                          /* 4160 */
                        startTime += addTime;                                 /* 4161 */
                    if (checkMusicVoiceID >= 0)                               /* 4168 */
                        musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); /* 4170 */
                    clockTimeStart = clock();                                 /* 4175 */
                    QueryPerformanceCounter(&li);                             /* 4177 */
                    qpc_start = li.LowPart;                                   /* 4178 */
                    timeTimeStart = time(NULL);                               /* 4182 */
                    lastMusicPos = 0;                                         /* 4170: folded into the
                                                                                  musicCounter statement's own
                                                                                  fragment, same shape as 4077 */
                    accMusics = 0.0;                                          /* 4170 */
                }
            }
            if (is_pause(&ctrl) && ply[player_id]->dead == 0) {               /* 4186 */
                /* REGION W3b: pause-key screen, lines 4187..4245 (near-identical to W3a) */
                int pauseTime, fc, ca, addTime; /* block-scoped DWARF locals (block 132838), not in the 52-local skeleton */

                pauseTime = time(NULL);                                       /* 4187 */
                fc = fall_count;                                              /* 4188 */
                ca = clock_angle;                                             /* 4189 */
                log2file("  game paused with pause key");                     /* 4190 */
                for (i = 0; i < 640; i += 2) {                                /* 4192 */
                    vline(swap_screen, i, 0, 480, 0);                         /* draw.inl:46 */
                    hline(swap_screen, 0, i, 640, 0);                         /* draw.inl:54 */
                }
                textout_centre_ex(swap_screen, data[50].dat,                  /* 4196 (annotation kept on the
                                                                                  opening line; see the note above
                                                                                  the ESC screen's identical calls) */
                                   "Game Paused", 320, 160, -1, -1);
                textout_centre_ex(swap_screen, data[52].dat,                  /* 4197 */
                                   "Press any key to resume", 320, 210, -1, -1);
                blit_to_screen(swap_screen);                                  /* 4198 */
                play_sound(custom.wazup, 0, 1);                               /* 4199 */
                /* Same three-loop shape as the ESC screen (W3a above), but this screen's
                 * outer wait has no closeButtonClicked term (source-view 4186..4245 never
                 * loads it before is_any/is_pause/ESC), and its final loop tests
                 * is_pause() || key[KEY_ESC] directly (continues on either, offset
                 * 7018/7027 both jump back to the poll_control/rest body) rather than the
                 * negated-AND the ESC screen's closing loop uses. */
                poll_control(&ctrl, 0);                                       /* 4200 */
                while (is_any(&ctrl) || is_pause(&ctrl)) {                    /* 4201 */
                    poll_control(&ctrl, 0);                                   /* 4202 */
                    rest(2);                                                  /* 4203 */
                }
                clear_keybuf();                                               /* 4206 */
                while (!keypressed()) {                                       /* 4207 */
                    if (is_any(&ctrl) || is_pause(&ctrl) || key[KEY_ESC])      /* 4207 */
                        break;                                                /* 4207 */
                    poll_control(&ctrl, 0);                                   /* 4208 */
                    rest(2);                                                  /* 4209 */
                }
                poll_control(&ctrl, 0);                                       /* 4212 */
                while (is_pause(&ctrl) || key[KEY_ESC]) {                     /* 4213 */
                    poll_control(&ctrl, 0);                                   /* 4214 */
                    rest(2);                                                  /* 4215 */
                }
                fall_count = fc;                                              /* 4219 */
                clock_angle = ca;                                             /* 4220 */
                log2file("  game unpaused");                                  /* 4221 */
                addTime = time(NULL) - pauseTime;                             /* 4222 */
                if (addTime > 0)                                              /* 4223 */
                    startTime += addTime;                                     /* 4224 */
                if (checkMusicVoiceID >= 0)                                   /* 4231 */
                    musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); /* 4233 */
                clockTimeStart = clock();                                     /* 4238 */
                QueryPerformanceCounter(&li);                                 /* 4240 */
                qpc_start = li.LowPart;                                       /* 4241 */
                timeTimeStart = time(NULL);                                   /* 4245 */
                lastMusicPos = 0;                                             /* 4233: folded into the
                                                                                  musicCounter statement's own
                                                                                  fragment, same shape as 4077 */
                accMusics = 0.0;                                              /* 4233 */
            }
            if (!itrcheck) {                                                 /* 4249 */
                poll_control(&rec_ctrl, 0);                                   /* 4251 */
                if (ply[player_id]->dead) {                                   /* 4253 */
                    log2file("  replay ended after death");                   /* 4255 */
                    playing = 0;                                              /* 4255 */
                }
                if (key[KEY_ESC]) {                                           /* 4264 */
                    log2file("  quit from replay");                           /* 4265 */
                    quit = 1;                                                 /* 4265 */
                    playing = 0;                                              /* 4265 */
                }
                if (key[KEY_SPACE]) {                                        /* 4271 */
                    if (ply[player_id]->dead == 0) {
                        log2file("  replay paused");                          /* 4272 */
                        while (key[KEY_SPACE])                                /* 4273: debounce-wait loop */
                            poll_control(&rec_ctrl, 1);
                        while (!key[KEY_SPACE] && !key[KEY_RIGHT] &&          /* 4274: pause-wait loop */
                               !key[KEY_ESC] && !key[KEY_UP]) {               /* 4274: pause-wait loop */
                            poll_control(&rec_ctrl, 1);                       /* 4275 */
                            if (key[KEY_F1])                                  /* 4276 */
                                take_screenshot(swap_screen);                 /* 4277 */
                            /* main.c:4278's own jne loops back to its OWN fragment's start
                             * (offset 10425 == 0x4142b9 - 0x411a00), with nothing else between --
                             * a bare debounce spin on key[KEY_F1] alone, no poll_control() call
                             * (unlike the KEY_SPACE debounce loops at 4273/4281), that was simply
                             * missing from this block. */
                            while (key[KEY_F1])                                /* 4278 */
                                ;
                        }
                        while (key[KEY_SPACE])                                /* 4281: debounce-wait loop */
                            poll_control(&rec_ctrl, 1);
                        fast_forward = 0;                                     /* 4282 */
                        fast_fast_forward = 0;                                /* 4283 */
                        log2file("  replay unpaused");                       /* 4284 */
                    }
                }
                /* NOT an else: main.c:4271's own je (SPACE not pressed) lands at offset 5965,
                 * exactly the reload that starts main.c:4284's tail; the dead!=0 fallthrough at
                 * offset 3262 lands exactly at 4287's own first fragment; and 4284's own tail
                 * falls straight into 4287's SECOND fragment (offsets 5971..5984) after finishing
                 * the pause/unpause sequence. All three paths -- space not pressed, space pressed
                 * while dead, and space pressed-and-unpaused -- converge on the same KEY_RIGHT
                 * test, so 4287..4310 run unconditionally after the block above, not only when it
                 * was skipped. */
                if (key[KEY_RIGHT]) {                                    /* 4287 */
                    fast_forward++;                                       /* 4288 */
                    fast_fast_forward = 0;                                /* 4289 */
                } else {
                    /* main.c:4287's own je (KEY_RIGHT false) and the post-unpause path's second
                     * KEY_RIGHT test both land at offset 5984, `movl $0,fast_forward`, tagged
                     * main.c:4292 -- a real reset shared by both paths, not just the debounce
                     * tail, missing from this branch until now. */
                    fast_forward = 0;                                     /* 4292 */
                    if (key[KEY_UP]) {                                    /* 4295 */
                        if (ply[player_id]->dead == 0 &&
                            ply[player_id]->level < demo->floor - 10) {       /* 4296 */
                            fast_fast_forward++;                              /* 4297 */
                            fast_forward = 0;                                 /* 4298 */
                            next_floor = ((ply[player_id]->level + 100) / 100) * 100; /* 4299 */
                            if (next_floor > demo->floor - 10)                /* 4301 */
                                next_floor = demo->floor - 10;                /* 4301 */
                        }
                    }
                }
                /* source-view 4270..4320: the level>=next_floor test (fragments 3390..3412)
                 * falls straight into the reset; the level<next_floor case instead jumps to
                 * a second, out-of-line test of ply[player_id]->dead (fragments 7984..7999)
                 * that also reaches the reset when dead != 0 -- one condition, two tested
                 * terms ORed, not just the level compare. */
                if (ply[player_id]->level >= next_floor ||
                    ply[player_id]->dead) {                              /* 4309 */
                    fast_fast_forward = 0;                                /* 4310 */
                    next_floor = -1;
                }
            }
        }
        if (!itrcheck) {                                                     /* 4319 */
            static int someCounter;
            int ffstep; /* DWARF block 132354 [2740..2804 6150..6448]: someCounter, ffstep, drew, skipDrawing */

            someCounter++;                                                    /* 4324 */
            ffstep = fast_forward ? 4 : 1;                                   /* 4327 */
            if (fast_fast_forward)                                            /* 4330 */
                ffstep = 32;                                                  /* 4330 */
            if (!quit && someCounter % ffstep == 0) {                        /* 4337 */
                draw_frame(swap_screen);                                      /* 4338 */
                if (ply[player_id]->shake) {                                  /* 4346 */
                    acquire_screen();                                          /* gfx.inl:221/203 */
                    blit(swap_screen, swap_screen, 0, shake, 0, 0,             /* 4348: args from
                                                                                    fragments 6207..6271 (source-view
                                                                                    4340 4352) -- src=dst=swap_screen,
                                                                                    src_x=0, src_y=shake (ebp-0x96c),
                                                                                    dst_x=dst_y=0, w/h read back from
                                                                                    swap_screen's own struct fields
                                                                                    (mov (%eax),%edx / mov 0x4(%eax));
                                                                                    annotation moved to the opening
                                                                                    line for the same reason as 4126
                                                                                    above (GCC attributes a call's
                                                                                    bytes to where it opens). */
                         swap_screen->w, swap_screen->h);
                    blit_to_screen(swap_screen);                              /* 4349 */
                    release_screen();                                         /* gfx.inl:227/212 */
                } else {
                    blit_to_screen(swap_screen);                              /* 4353 */
                }
                if (!debug) {                                                 /* 4356 */
                    while (cycle_count == 0)                                  /* 4357 */
                        rest(2);                                              /* 4357 */
                } else if (key[KEY_TAB] && key[KEY_LSHIFT]) {                  /* 4360 */
                    while (cycle_count <= 7) { }                              /* 4361: busy wait */
                } else {
                    while (cycle_count == 0)
                        rest(2);                                              /* 4363 */
                }
            }
        }
        if (!itrcheck)                                                       /* 4369 */
            rest(2);
    }

    /* lines 4374..4426: recording gates a small profile play-time update vs. the full
     * gameData stats snapshot + itrcheck-gated XML dump. */
    if (recording) {                                                           /* 4374 */
        diff = endTime - startTime;                                            /* 4375 */
        if (diff > 0)                                                          /* 4376 */
            profile->seconds_spent_playing += diff;                           /* 4377 */
    } else {
        gameData->score = ply[player_id]->level * 10 + ply[player_id]->score;  /* 4389 */
        gameData->floor = ply[player_id]->level;                               /* 4390 */
        gameData->combo = ply[player_id]->best_combo;                          /* 4391 */
        gameData->no_combo_top_floor = ply[player_id]->no_combo_top_floor;     /* 4392 */
        gameData->biggest_lost_combo = ply[player_id]->biggest_lost_combo;     /* 4393 */
        for (i = 0; i < 5; i++)                                                /* 4395 */
            gameData->ccc[i] = ply[player_id]->ccc[i];                        /* 4395 */
        for (i = 0; i < 5; i++)                                                /* 4398 */
            gameData->jc[i] = ply[player_id]->jcTop[i];                       /* 4398 */
        {
            int keys_pressed[7];
            int key_flag[7] = { 16, 1, 2, 4, 8, 32, 128 };                     /* 4403 */
            int last_keys[7];
            int k;

            for (k = 0; k < 7; k++)                                            /* 4402 */
                keys_pressed[k] = time_cheat_count;                            /* 4402 */
            if (demo->size > 0) {                                              /* 4406 */
                for (k = 0; k < 7; k++)                                        /* 4404: rep stos reuses eax
                                                                                    * without reloading it from
                                                                                    * 4402's time_cheat_count
                                                                                    * read (offset 7805, no mov
                                                                                    * before it) -- last_keys is
                                                                                    * seeded with time_cheat_count,
                                                                                    * not a literal 0. */
                    last_keys[k] = time_cheat_count;                           /* 4404 */
                for (i = 0; i < demo->size; i++) {                             /* 4406 */
                    int flags = demo->data[i].key_flags;                       /* 4406 */
                    for (k = 0; k < 7; k++) {                                  /* 4408 */
                        int f = key_flag[k] & flags;                           /* 4409 */
                        if (!last_keys[k] && f)                                /* 4409 */
                            keys_pressed[k]++;                                 /* 4410 */
                        last_keys[k] = f;                                      /* 4412 */
                    }
                }
            }
            gameData->jump = keys_pressed[0];                                  /* 4416 */
            gameData->left = keys_pressed[1];                                  /* 4417 */
            gameData->right = keys_pressed[2];                                 /* 4418 */
        }
        if (itrcheck) {                                                       /* 4421 */
            char *xmlStr = getGameDataXML(gameData);                          /* 4422 */
            printf("%s", xmlStr);                                             /* 4423 */
            free(xmlStr);                                                     /* 4424 */
        }
        if (itrcheck)                                                         /* 4426 */
            return 0;
    }

    /* lines 4456..4458 */
    log2file(" play ended");                                                  /* 4456 */
    fast_forward = 0;                                                         /* 4457 */
    fast_fast_forward = 0;                                                    /* 4458 */

    /* lines 4500..4643: demo/profile stat snapshot (recording && !quit only), then
     * syncProfileFromOptions()/save_profile() unconditionally, then the quit/closeButtonClicked
     * guard around highscore qualification. Traced from three save_profile() call sites all
     * tagged historical line 4641 (offsets 8235, 9307, 10956): the !recording predecessor
     * (offset 8179) calls sync+save BEFORE ever testing quit (offset 8240's test comes after
     * the call, not before it); the recording&&quit predecessor jumps straight past the whole
     * replay-file block to its own sync+save copy (offset 10900, target of the "jne 414494"
     * at offset 8315); the recording&&!quit predecessor falls through the replay-file block
     * into a third sync+save copy (offset 9251) whose *own* trailing test reads
     * closeButtonClicked directly (offset 9312, "cmpl $0x0,closeButtonClicked") rather than
     * quit -- because on that path quit was already resolved false by the earlier test at
     * offset 8308. That is only consistent with sync+save being unconditional statements
     * textually AFTER this whole if/else, not folded into either arm or gated by !quit. */
    if (recording) {                                                          /* 4500 */
        if (!quit) {                                                          /* 4500 */
            demo->score = ply[player_id]->level * 10 + ply[player_id]->score;  /* 4503 */
            demo->floor = ply[player_id]->level;                               /* 4504 */
            demo->combo = ply[player_id]->best_combo;                         /* 4505 */
            demo->rejump = options.jump_hold;                                  /* 4506 */
            demo->no_combo_top_floor = ply[player_id]->no_combo_top_floor;     /* 4507 */
            demo->biggest_lost_combo = ply[player_id]->biggest_lost_combo;     /* 4508 */
            for (i = 0; i < 5; i++)                                            /* 4510 */
                demo->ccc[i] = ply[player_id]->ccc[i];                        /* 4510 */
            for (i = 0; i < 5; i++)                                            /* 4513 */
                demo->jc[i] = ply[player_id]->jcTop[i];                       /* 4513 */

            if (!is_playing_custom_game) {                                    /* 4519 */
                profile->games_played++;                                      /* 4520 */
                profile->total_floors += demo->floor;                         /* 4522 */
                profile->total_score += demo->score;                          /* 4523 */
                profile->total_combos += numComboJumps;                       /* 4524 */
                profile->total_combo_floors += totComboFloors;                /* 4525 */
                for (i = 0; i < 5; i++) {                                      /* 4526 */
                    if (demo->ccc[i] > 0) {                                    /* 4527 */
                        profile->cccNum[i]++;                                  /* 4528 */
                        profile->cccTotal[i] += demo->ccc[i];                  /* 4529 */
                    }
                }
            } else {
                profile->custom_games_played++;                               /* 4534 */
            }

            /* lines 4541..4624: replay directory + per-category replay files */
            if (!file_exists(replay_directory, -1, NULL))                     /* 4541 */
                mkdir(replay_directory);                                      /* 4543 */

            if (!is_playing_custom_game) {                                    /* 4550 */
                int rank;

                if (demo->floor > profile->best_floor) {                      /* 4551 */
                    profile->best_floor = demo->floor;                        /* 4552 */
                    myDeleteFile(replay_directory, profile->best_replay_names[2]);  /* 4553 */
                    sprintf(profile->best_replay_names[2], "%s_best_floor_%d.itr",  /* 4554 */
                            profile->handle, demo->floor);
                    save_replay(replay_directory, profile->best_replay_names[2], demo,  /* 4555 */
                                rec_pos + 2, 1);
                    new_personal_best[2] = 1;                                  /* 4556 */
                }
                if (demo->combo > profile->best_combo) {                      /* 4559 */
                    profile->best_combo = demo->combo;                        /* 4560 */
                    myDeleteFile(replay_directory, profile->best_replay_names[1]);  /* 4561 */
                    sprintf(profile->best_replay_names[1], "%s_best_combo_%d.itr",  /* 4562 */
                            profile->handle, demo->combo);
                    save_replay(replay_directory, profile->best_replay_names[1], demo,  /* 4563 */
                                rec_pos + 2, 1);
                    new_personal_best[1] = 1;                                  /* 4564 */
                }
                if (demo->score > profile->best_score) {                      /* 4567 */
                    profile->best_score = demo->score;                        /* 4568 */
                    myDeleteFile(replay_directory, profile->best_replay_names[0]);  /* 4569 */
                    sprintf(profile->best_replay_names[0], "%s_best_score_%d.itr",  /* 4570 */
                            profile->handle, demo->score);
                    save_replay(replay_directory, profile->best_replay_names[0], demo,  /* 4571 */
                                rec_pos + 2, 1);
                    new_personal_best[0] = 1;                                  /* 4572 */
                }
                if (ply[player_id]->no_combo_top_floor > profile->no_combo_top_floor) {  /* 4575 */
                    profile->no_combo_top_floor = ply[player_id]->no_combo_top_floor;  /* 4576 */
                    myDeleteFile(replay_directory, profile->best_replay_names[4]);  /* 4577 */
                    sprintf(profile->best_replay_names[4], "%s_best_no_combo_%d.itr",  /* 4578 */
                            profile->handle, demo->no_combo_top_floor);
                    save_replay(replay_directory, profile->best_replay_names[4], demo,  /* 4579 */
                                rec_pos + 2, 1);
                    new_personal_best[4] = 1;                                  /* 4580 */
                }
                if (ply[player_id]->biggest_lost_combo > profile->biggest_lost_combo) {  /* 4583 */
                    profile->biggest_lost_combo = ply[player_id]->biggest_lost_combo;  /* 4584 */
                    myDeleteFile(replay_directory, profile->best_replay_names[3]);  /* 4585 */
                    sprintf(profile->best_replay_names[3], "%s_best_lost_combo_%d.itr",  /* 4586 */
                            profile->handle, demo->biggest_lost_combo);
                    save_replay(replay_directory, profile->best_replay_names[3], demo,  /* 4587 */
                                rec_pos + 2, 1);
                    new_personal_best[3] = 1;                                  /* 4588 */
                }
                for (rank = 1; rank < 6; rank++) {                             /* 4591 */
                    if (ply[player_id]->ccc[rank - 1] > profile->ccc[rank - 1]) {  /* 4592 */
                        profile->ccc[rank - 1] = ply[player_id]->ccc[rank - 1];  /* 4593 */
                        myDeleteFile(replay_directory, profile->best_replay_names[4 + rank]);  /* 4594 */
                        sprintf(profile->best_replay_names[4 + rank], "%s_best_cc%d_%d.itr",  /* 4595 */
                                profile->handle, rank, ply[player_id]->ccc[rank - 1]);
                        save_replay(replay_directory, profile->best_replay_names[4 + rank],  /* 4596 */
                                    demo, rec_pos + 2, 1);
                        new_personal_best[4 + rank] = 1;                       /* 4597 */
                    }
                }
                for (rank = 1; rank < 6; rank++) {                             /* 4601 */
                    if (ply[player_id]->jcTop[rank - 1] > profile->jc[rank - 1]) {  /* 4602 */
                        profile->jc[rank - 1] = ply[player_id]->jcTop[rank - 1];  /* 4603 */
                        myDeleteFile(replay_directory, profile->best_replay_names[9 + rank]);  /* 4604 */
                        sprintf(profile->best_replay_names[9 + rank], "%s_best_jj%d_%d.itr",  /* 4605 */
                                profile->handle, rank, ply[player_id]->jcTop[rank - 1]);
                        save_replay(replay_directory, profile->best_replay_names[9 + rank],  /* 4606 */
                                    demo, rec_pos + 2, 1);
                        new_personal_best[9 + rank] = 1;                       /* 4607 */
                    }
                }
            }

            if (save_replay(replay_directory, "last_game.itr", demo, rec_pos + 2, 1) < 0) {  /* 4613 */
                my_alert("Failed to save replay.", "(last_game.itr)", 0, 1);  /* 4614 */
                uberChecksum = 0;                                             /* 4615 */
            } else {
                char fbuf[2048];
                Treplay *rr;

                sprintf(fbuf, "%slast_game.itr", replay_directory);          /* 4620 */
                rr = load_replay(fbuf);                                      /* 4621 */
                if (rr) {                                                    /* 4622 */
                    uberChecksum = calc_replay_checksum(demo);               /* 4623 */
                    destroy_replay(rr);                                     /* 4624 */
                }
            }
        }
    }

    /* lines 4641..4643: unconditional, reached from all three predecessors above */
    syncProfileFromOptions();                                                /* 4641 */
    save_profile(profile);                                                   /* 4641 */

    {
        /* DWARF block 133269 begins after the replay filename buffer's scope. */
        float hy;
        int gotHigh;
        int qualify[15];
        int qualifyValue[15];
        int gameover_bmp_id;

    /* lines 4643..4683: highscore qualification, guarded by quit && closeButtonClicked */
    if (!quit) {                                                             /* 4643 */
        if (!closeButtonClicked) {                                           /* 4643 */
            int rank;   /* qualify, qualifyValue, gotHigh and gameover_bmp_id live in the enclosing
                         * DWARF block 133269, which opens here and runs into REGION W5 */

            for (i = 0; i < 15; i++)                                         /* 4650 */
                qualify[i] = 0;                                             /* 4650 */
            qualifyValue[0] = ply[player_id]->level * 10 + ply[player_id]->score;  /* 4652 */
            qualifyValue[2] = ply[player_id]->level;                         /* 4653 */
            qualifyValue[1] = ply[player_id]->best_combo;                    /* 4654 */
            qualifyValue[3] = ply[player_id]->biggest_lost_combo;            /* 4655 */
            qualifyValue[4] = ply[player_id]->no_combo_top_floor;            /* 4656 */
            for (i = 0; i < 5; i++) {                                        /* 4657 */
                qualifyValue[5 + i] = ply[player_id]->ccc[i];                /* 4658 */
                qualifyValue[10 + i] = ply[player_id]->jcTop[i];             /* 4659 */
            }
            quit = 0;                                                       /* 4657 */
            gotHigh = 0;                                                    /* 4657 */
            for (rank = 0; rank < 15; rank++) {                              /* 4662 */
                qualify[rank] = qualify_hisc_table(hisc_tables[rank], qualifyValue[rank]);  /* 4663 */
                gotHigh += qualify[rank];                                    /* 4664 */
            }
            quit = gotHigh;                                                 /* 4662: local_slot_trace shows
                                                                                * THREE writes to quit's slot
                                                                                * in this stretch (9446/4657
                                                                                * const 0, 9503/4662 a VALUE,
                                                                                * 9522/4668 const 0 again) and
                                                                                * seven reads after them. At
                                                                                * offset 9503 ("mov %esi,
                                                                                * -0x93c(%ebp)") esi is the
                                                                                * gotHigh accumulator, still
                                                                                * live from "add %eax,%esi"
                                                                                * at offset 9495 (line 4664) --
                                                                                * quit is reused from here on
                                                                                * to carry gotHigh's value
                                                                                * through the rest of the
                                                                                * function; the 4670/4673 tests
                                                                                * below read quit, not gotHigh,
                                                                                * from this point on. */

            if (recording) {                                                /* 4668 */
                gameover_bmp_id = (quit > 0) ? 0x3e : 0x37;                  /* 4670: disasm reads -0x93c
                                                                                * (quit's slot) here, not
                                                                                * gotHigh's esi. */
            } else {
                quit = 0;                                                   /* 4668: second const-0 write to
                                                                                * quit's slot (offset 9522),
                                                                                * distinct from gotHigh (whose
                                                                                * own DW_OP_reg6/esi location
                                                                                * list keeps it separately live
                                                                                * over this same span). */
                gotHigh = 0;                                                /* 4668 */
                gameover_bmp_id = 0x37;                                     /* 4668 */
            }
            if (is_playing_custom_game)                                     /* 4671 */
                gameover_bmp_id = 0x37;                                     /* 4671 */

            if (quit) {                                                     /* 4673: disasm reads -0x93c
                                                                                * (quit) again here, not
                                                                                * gotHigh. */
                if (!is_playing_custom_game) {                              /* 4673 */
                    log2file(" player qualified for highscore");            /* 4674 */
                    play_sound(sounds[7], 0, 0);                            /* 4675 */
                }
            } else {
                log2file(" player did not qualify for highscore");          /* 4678 */
                play_sound(speaker[1], 0, 0);                               /* 4679 */
            }

            /* 4683: the debug path skips the results animation below. */
        }
    }

    {
        /* hy, gotHigh, qualify and qualifyValue are declared in the enclosing DWARF block 133269,
         * which opens in REGION W4 at main.c:4650 and runs to the end of the function. */
        if (!debug) {       /* original 9619 branches around the results work */
        hy = 480.0f;       /* 4704: panel starts off-screen at 480 and eases up toward 130.0f */
        int alpha_pos;      /* letter-navigation cursor; first write is at 4821 (rank-up reset) */
        char *initials = NULL;

        {
            /* DWARF and instructions separate four name-entry variables: alpha_pos
             * at stack -0x930 indexes letters[], pos in esi indexes buf[], done at
             * stack -0x938 controls the loop, and skip_keys in edi delays repeat
             * input. The previous candidate conflated these states. scrollerY is
             * not read from the earlier results loop; its DWARF register range
             * starts at offset 12095 in this inner block. */
            int scrollerY;   /* ticker-bar Y offset; %ebx from the 4821 reset (-20) through the
                               * 4831-4838 easing -- was wrongly conflated with alpha_pos before. */
            char letters[31] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ .\244";
            int len;
            char buf[8] = { '.', 0, '.', 0, '.', 0, 0, 0 };  /* 4689 */
            int done;
            int skip_keys;
            int pos;
            int isGuest;
            int new_rank_id;
            int rank_bmp_id;
            int rank_y;
            int k;
            char guestName[4];
            char postName[32];

            len = strlen(letters) - 1;                                              /* 4688 */
            isGuest = !stricmp(profile->handle, "guest");                           /* 4692 */

            /* qualify, qualifyValue and gotHigh are NOT recomputed here: they are declared in
             * the enclosing DWARF block 133269, which opens in REGION W4 at main.c:4650 and
             * does not close until the end of the function, so W4's own computation (main.c
             * 4650-4664, and the recording/is_playing_custom_game handling through 4671) is
             * still in scope and still holds. A prior pass at this region assumed the opposite
             * ("this region cannot see the values") and inserted an unannotated fourteen-
             * statement duplicate of W4's block here; function_lines.py --source-view 4650 4687
             * shows the original computes qualify[]/qualifyValue[]/gotHigh exactly ONCE, entirely
             * within W4's own address range (offsets 9327-9497), with nothing resembling it
             * again before 4687. The duplicate also disagreed with W4's real logic (W4 only
             * zeroes gotHigh when !recording, never when is_playing_custom_game while recording
             * -- the deleted duplicate zeroed it in both cases). Removed. */

            /* results screen: slide the results panel in and wait for the
             * highscore chime / fade timer (4695..4737). */
            for (;;) {
                cycle_count = 0;                                                     /* 4696 */
                ply[player_id]->dead -= 16;                                          /* 4697 */
                hy = hy + (130.0f - hy) * 0.1;                                       /* 4699 */
                update_frame();                                                      /* 4700 */
                for (i = 0; i < 512; i++)                                            /* 4701 */
                    if (stars[i].intensity)
                        update_particle(&stars[i]);
                if ((unsigned)(hurry_y + 99) <= 578u)                                 /* 4702 */
                    hurry_y -= 2;
                draw_frame(swap_screen);                                             /* 4703 */
                /* 4704 */
                draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify,
                             qualifyValue,
                             is_playing_custom_game ? 0 : (recording != 0));
                if (isGuest && gotHigh && !is_playing_custom_game && !recording) { /* 4705 */
                    /* 4706 */ textout_centre_ex(swap_screen, data[52].dat, "Enter your initials",
                                       320, (int)(hy * 2.0 + 80.0), -1, -1);
                }
                if (falling >= 1)                                                   /* 4709: cmp/sbb */
                    falling++;
                if (falling <= ply[player_id]->level * 5 && falling <= 250) {     /* 4710 */
                    play_sound(sounds[6], 0, 1);                                      /* 4711 */
                    if (custom.falling)                                               /* 4712 */
                        stop_sample(custom.falling);
                    ply[player_id]->shake = 24;                                       /* 4714 */
                }
                if (ply[player_id]->shake) {                                          /* 4716 */
                    acquire_screen();                                                /* 4716: inlined */
                    /* 4718 */ blit(swap_screen, screen, 0, new_rand() % 8, 0, 0,
                         swap_screen->w, swap_screen->h);
                    release_screen();                                                /* 4718: inlined */
                    ply[player_id]->shake--;                                          /* 4720 */
                }
                blit_to_screen(swap_screen);                                          /* 4722 */
                if (key[KEY_F1]) {                                                    /* 4725 */
                    take_screenshot(swap_screen);                                     /* 4726 */
                    while (key[KEY_F1]) { }  /* 4727: wait for release at 13091..13105 */
                }
                if (key[KEY_TAB] && key[KEY_LSHIFT]) {                                /* 4731 */
                    do {
                        rest(2);                                                      /* 4734 */
                    } while (cycle_count == 0);
                }
                if (hy <= 140.0f)  /* sole original 140.0f comparison at offset 11863 */
                    break;
            }
            ply[player_id]->dead = 0;                                                 /* 4737 */
            clear_keybuf();                                                           /* 4738 */

            /* summary scroller message: custom/guest/personal-record tip (4742..4775). */
            if (!recording)
                summary_scroller_message[0] = 0;                                      /* 4743 */
            else {
                if (is_playing_custom_game) {
                    /* 4746: annotate the statement's FIRST physical line, not just the last --
                     * an unannotated opening line inherits the previous statement's historical
                     * line (4743) and misattributes this call's bytes to it. */
                    memcpy(summary_scroller_message,               /* 4746 */
                           "Custom mode is crazy fun but does not add to your profile. "
                           "Play Classic Mode to compete in the highscore lists and "
                           "climb in rank!", 0x82);                                    /* 4746 */
                } else {
                    if (gotHigh)                                                         /* 4749 */
                        memcpy(summary_scroller_message, "New personal records!    ", 0x1a); /* 4753 */
                    if (isGuest) {                                                       /* 4770 */
                        strcpy(summary_scroller_message,
                               "You're playing in guest mode. Start a profile and "
                               "record your progress!");
                    } else {
                        strcpy(summary_scroller_message, hints[new_rand() % 45]);        /* 4775 */
                    }
                }
            }

            /* name entry loop: scroller/rank banner setup and per-frame draw + input
             * (4781..4923). */
            /* 4781 */ init_scroller(&summary_scroller, data[54].dat, summary_scroller_message,
                           640, 30, -1);
            scroll_scroller(&summary_scroller, -150);                                  /* 4782 */
            new_rank_id = get_rank_id(profile);                                        /* 4787 */
            rank_bmp_id = new_rank_id + 0x4a;
            alpha_pos = 0;
            rank_y = 0x244;
            scrollerY = -20;
            skip_keys = 0;
            pos = 0;
            done = 20;

            for (;;) {
                if (done == 0)                          /* 4792: cmp $0, done slot -0x938 */
                    break;
                if (closeButtonClicked)                                                /* 4793 */
                    break;
                cycle_count = 0;                                                       /* 4797 */
                step_count++;                                                          /* 4798 */
                update_frame();                                                        /* 4800 */
                if (key[KEY_F1]) {                                                     /* 4802 */
                    take_screenshot(swap_screen);                                      /* 4803 */
                    while (key[KEY_F1]) { }                                            /* 4804 */
                }
                if ((unsigned)(hurry_y + 99) <= 578u)                                   /* 4808 */
                    hurry_y -= 2;
                draw_frame(swap_screen);                                               /* 4809 */
                /* 4810 */ draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify,
                             qualifyValue, is_playing_custom_game ? 0 : (recording != 0));
                if (isGuest && gotHigh && !is_playing_custom_game && !recording) {  /* 4811 */
                    /* 4812 */ textout_centre_ex(swap_screen, data[52].dat, "Enter your initials",
                                       320, (int)(hy * 2.0 + 80.0), -1, -1);
                    if (pos != 0 || (step_count & 4))                                   /* 4814 */
                        textout_centre_ex(swap_screen, data[52].dat, &buf[0],
                                           300, (int)(hy * 2.0 + 120.0), -1, -1);
                    if (pos != 1 || (step_count & 4))                                   /* 4815 */
                        textout_centre_ex(swap_screen, data[52].dat, &buf[2],
                                           320, (int)(hy * 2.0 + 120.0), -1, -1);
                    if (pos != 2 || (step_count & 4))                                   /* 4816 */
                        textout_centre_ex(swap_screen, data[52].dat, &buf[4],
                                           340, (int)(hy * 2.0 + 120.0), -1, -1);
                    if (pos == 3 && (step_count & 4))                                   /* 4817 */
                        textout_centre_ex(swap_screen, data[52].dat, "%",
                                           360, (int)(hy * 2.0 + 120.0), -1, -1);
                }
                if (new_rank_id != current_rank_id) {                                   /* 4820 */
                    /* 4821: banner state is initialized before the loop, as in
                     * original offsets 12034..12075. The rank-change branch
                     * starts with the bitmap lookup at offset 12351. */
                    draw_sprite(swap_screen, data[rank_bmp_id].dat, 20, rank_y);         /* 4821 (inlined) */
                    /* 4822 */ textout_ex(swap_screen, data[52].dat, "rank up!",
                               20, rank_y + 0x46, -1, -1);
                    /* 4823 */ rank_y = (int)((320 - rank_y) * 0.1 + rank_y);
                    /* current_rank_id = new_rank_id was here in an earlier pass, but
                     * local_slot_trace for current_rank_id shows exactly 3 accesses in the
                     * whole function -- init at 3441, a write at 3443 (both before region
                     * W1a), and the single read at 4820 -- with NO write anywhere inside this
                     * loop. Removed as invented: nothing in the trace supports it, and per
                     * large-body-reconstruction-pitfalls this makes new_rank_id!=current_rank_id
                     * loop-invariant-true after the first rank-up, i.e. the block re-triggers
                     * every frame, which is consistent with the evidence even if it looks like
                     * a game bug. */
                    /* Tried: moving this draw_sprite/textout_ex/easing out of the if-block to
                     * run unconditionally every frame (on the theory that evidence/census/
                     * line-mappings.json shows exactly two draw.inl:238 sites in the whole
                     * function, both here, so rank_y should have two readers). Measured worse
                     * on every axis: play grew 16556->16678 bytes and this line's own delta
                     * went from +70 (4820) to +168, so reverted. The second draw.inl:238 site
                     * (offset 0x35b3, historical) is real but is NOT reached by simply hoisting
                     * this draw out of the if -- its actual source position is still unknown. */
                }
                if (summary_scroller_message[0]) {                                       /* 4827 */
                    scroll_scroller(&summary_scroller, -2);                              /* 4828 */
                    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                              /* 4829 */
                    set_trans_blender(0, 0, 0, 110);                                     /* 4830 */
                    /* 4831-4833: original computes these through draw.inl's inlined rectfill,
                     * with y1=scrollerY and y2=scrollerY+20/+18/+16 (confirmed via
                     * `lea 0x14(%ebx),%eax` / `lea 0x12(%ebx),%eax` / `lea 0x10(%ebx),%eax`
                     * at offsets 13301/13375/13449, all inside scrollerY's DWARF-register
                     * range) -- not the constants this used to hardcode. */
                    rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 20, makecol(0, 0, 0)); /* 4831 */
                    rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 18, makecol(0, 0, 0)); /* 4832 */
                    rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 16, makecol(0, 0, 0)); /* 4833 */
                    solid_mode();                                                         /* 4834 */
                    /* 4835/4836: the draw_scroller 4th argument is 0xc(%esp)=%ebx at offsets
                     * 13519/13584, both inside scrollerY's register range -- not alpha_pos. */
                    draw_scroller(&summary_scroller, swap_screen, 1, scrollerY,          /* 4835 */
                                   makecol(150, 150, 150));
                    if (!draw_scroller(&summary_scroller, swap_screen, 0, scrollerY,     /* 4836 */
                                        makecol(200, 200, 200)))
                        restart_scroller(&summary_scroller);
                }
                /* 4838: this easing computation (ebx=ebx+(int)(-ebx*0.1)) sits at offset
                 * 13633-13707, inside scrollerY's DWARF register range (13150-13795), not
                 * alpha_pos's (alpha_pos's own local_slot_trace has no access at 4838 at all).
                 * scrollerTargetY has no DWARF location (eliminated), consistent with the
                 * target being the constant 0 folded into this formula, the same way
                 * hyTarget==130.0f and rankTargetY==320 are already folded into hy/rank_y. */
                scrollerY = scrollerY + (int)(-scrollerY * 0.1);  /* 4838 */

                if (falling <= ply[player_id]->level * 5 && falling <= 250) {         /* 4844 */
                    play_sound(sounds[6], 0, 1);                                          /* 4845 */
                    if (custom.falling)                                                   /* 4846 */
                        stop_sample(custom.falling);
                    ply[player_id]->shake = 24;                                           /* 4850 */
                    falling = 0;
                }
                if (ply[player_id]->shake) {                                              /* 4852 */
                    acquire_screen();                                                    /* 4852: inlined */
                    /* 4855 */ blit(swap_screen, screen, 0, new_rand() % 8, 0, 0,
                         swap_screen->w, swap_screen->h);
                    release_screen();                                                    /* 4855: inlined */
                    ply[player_id]->shake--;                                              /* 4857 */
                }
                blit_to_screen(swap_screen);                                              /* 4860 */

                if (isGuest && gotHigh && !is_playing_custom_game && recording) {      /* 4863 */
                    poll_control(&ctrl, 0);                                                /* 4864 */
                    if (keypressed()) {                                                     /* 4865 */
                        if (done == 20) {                                                    /* 4865 */
                            k = readkey() & 0xff;                                            /* 4866 */
                            k -= 0x20;
                            if (k == -24)                                                    /* 4867 */
                                k = (signed char)0xa4;
                            else if (k == 14)                                               /* 4868 */
                                k = '.';
                            else if (k == 1)                                                /* 4869 */
                                k = '!';
                            if (k != 0x20) {                                                /* 4870 */
                                for (i = 0; i < len; i++) {                                 /* 4871 */
                                    char typed = letters[i];
                                    if ((signed char)typed == k) {                          /* 4872 */
                                        buf[pos * 2] = typed;                               /* 4875 */
                                        pos++;                                              /* 4876 */
                                        alpha_pos = i;
                                        skip_keys = 100;
                                        if (pos == 3)                                      /* 4877 */
                                            done = 19;
                                    }
                                }
                            }
                        }
                    }
                    if (skip_keys) {
                        skip_keys--;
                    } else {
                    if (is_right(&ctrl)) {                                                 /* 4884 */
                        alpha_pos++;                                                        /* 4885 */
                        skip_keys = 8;
                        if (alpha_pos > len)                                                /* 4886 */
                            alpha_pos = 0;
                    }
                    if (is_left(&ctrl)) {                                                  /* 4889 */
                        skip_keys = 8;
                        alpha_pos--;                                                        /* 4891 */
                        if (alpha_pos < 0)
                            alpha_pos = len;
                    }
                    if (is_fire(&ctrl)) {                                                  /* 4894 */
                        if (letters[alpha_pos] == (char)0xa4) {                            /* 4895: blank slot confirmed */
                            if (pos != 0) {                                                 /* 4895 */
                                buf[pos * 2] = '.';                                         /* 4896 */
                                pos--;                                                      /* 4897 */
                            }
                            skip_keys = 100;
                        } else if (pos <= 1) {                                              /* 4899 */
                            pos++;
                            skip_keys = 100;
                        } else {
                            if (done == 20) {                                               /* 4900 */
                                pos++;                                                      /* 4902 */
                                done = 19;
                            }
                            skip_keys = 100;
                        }
                    }
                    if (key[KEY_DEL] || key[KEY_BACKSPACE]) {                               /* 4906 */
                        buf[pos * 2] = '.';                                                 /* 4907 */
                        skip_keys = 7;
                        if (pos != 0)                                                       /* 4908 */
                            pos--;
                    } else if (skip_keys > 0) {
                        skip_keys--;                                                        /* 4912 */
                    }
                    }
                    if (!is_any(&ctrl) && !key[KEY_DEL] && !key[KEY_BACKSPACE])             /* 4913 */
                        skip_keys = 0;
                    if (pos <= 2)
                        buf[pos * 2] = letters[alpha_pos];                                  /* 4915 */
                }
                if (done != 20)                                                             /* 4918 */
                    done--;  /* original 12918..12930, reached from both paths */
                poll_control(&ctrl, 0);                                                     /* 4920 */
                if (isGuest && gotHigh && is_playing_custom_game) {                         /* 4921 */
                    if (keypressed() || is_fire(&ctrl)) {                                   /* 4922 */
                        if (done == 20)                                                     /* 4923 */
                            done = 14;  /* original 12956..13003, 13732 */
                    }
                }
                if (key[KEY_LSHIFT] && key[KEY_TAB]) {           /* 4929: operand order swapped
                     * from the 4731 occurrence -- function_lines.py source-view shows 4929 testing
                     * key[KEY_LSHIFT] first (4731 tests key[KEY_TAB] first), so the two blocks are
                     * not byte-identical in the original and the compiler does not fold them. */
                    do {
                        rest(2);                                                             /* 4932 */
                    } while (cycle_count == 0);
                    continue;
                }
            }

            /* highscore entry: commit the typed initials into every qualified table
             * (4940..4951). */
            if (!is_playing_custom_game && recording) {                                  /* 4940 */
                guestName[0] = buf[0];                                                       /* 4941 */
                guestName[1] = buf[2];
                guestName[2] = buf[4];
                guestName[3] = 0;
                if (isGuest) {                                                               /* 4943 */
                    initials = guestName;
                } else {
                    strcpy(postName, profile->handle);
                    initials = postName;
                }
                for (k = 0; k < 15; k++) {                                                   /* 4948 */
                    if (qualify[k] > 0) {                                                    /* 4949 */
                        enter_hisc_table(hisc_tables[k], qualifyValue[k], initials);          /* 4950 */
                        sort_hisc_table(hisc_tables[k]);                                      /* 4951 */
                    }
                }
            }

        } /* original debug route rejoins before the unlock check */

            /* post-game "new start floor unlocked" message (4963..4987). */
            if (recording && !debug && !is_playing_custom_game) {                        /* 4963 */
                int f = ply[player_id]->level / 100;                                          /* 4964 */
                if (f > oldUnlockedFloors && f <= 9) {                                        /* 4966 */
                    fadeOut(16);                                                              /* 4968 */
                    blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);                   /* 4971 */
                    set_trans_blender(0, 0, 0, 158);                                          /* 4974 */
                    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                                   /* 4975 */
                    {                                                                           /* 4976 */
                        int black = makecol(0, 0, 0);
                        int width = 0;
                        int height = 0;
                        GFX_DRIVER *driver = gfx_driver;
                        if (driver) {
                            width = driver->w;
                            height = driver->h;
                        }
                        rectfill(swap_screen, 0, 0, width, height, black);
                    }
                    solid_mode();                                                             /* 4977 */
                    draw_sprite(swap_screen, data[58].dat,
                                320 - ((BITMAP *)data[58].dat)->w / 2, 20);                    /* 4980 (inlined) */
                    /* 4981 */ textout_centre_ex(swap_screen, data[54].dat, "A new start floor",
                                       320, 0x12c, -1, -1);
                    /* 4982 */ textout_centre_ex(swap_screen, data[54].dat, "has been unlocked!",
                                       320, 0x15e, -1, -1);
                    /* 4983 */ textout_centre_ex(swap_screen, data[54].dat,
                                       "(Get it in the options menu)",
                                       320, 0x1b8, -1, -1);
                    play_sound(sounds[2], 0, 0);                                                /* 4984 */
                    fadeIn(swap_screen, 16);                                                   /* 4985 */
                    while (key[KEY_ESC] || key[KEY_ENTER] || key[KEY_SPACE]) {  /* 4986 */
                    }
                    while (!key[KEY_ESC] && !key[KEY_ENTER] && !key[KEY_SPACE]) {  /* 4986 */
                    }
                }
            }
        }

        /* epilogue: save config, stop music, offer the replay menu (4994..5021). */
        save_config();                                                                        /* 4994 */
        stopGameMusic();                                                                       /* 4997 */
        if (checkMusicVoiceID >= 0)                                                            /* 4998 */
            voice_stop(checkMusicVoiceID);                                                     /* 4999 */

        play_again = 0;                                                                        /* 5002 */
        if (recording) {                                                                        /* 5002 */
            int wasDebug = debug;  /* original checks debug before do_replay_menu */
            if (!wasDebug) {                                                                    /* 5002 */
                in_replay_menu = 1;                                                              /* 5003 */
                play_again = do_replay_menu();                                                   /* 5004 */
                in_replay_menu = 0;                                                              /* 5005 */
            }
            if (wasDebug || recording)                                                           /* 5011 */
                play_sound(speaker[2], 0, 0);
            stopGameMusic();                                                                     /* 5013 */
            if (checkMusicVoiceID >= 0)                                                           /* 5014 */
                voice_stop(checkMusicVoiceID);                                                    /* 5015 */
            clear_bitmap(screen);                                                                 /* inlined */
        }
    }

    }   /* end of DWARF block 133269, opened in REGION W4 */

    return play_again;
}

void show_credits(void)
{
    double vol;
    double vol_step;
    int gc;
    BITMAP *logoBMP;

    vol=options.msc_volume;
    vol_step=vol/150.0f;
    clear_bitmap(screen);
    logoBMP=data[125].dat;
    blit(data[126].dat,screen,0,0,0,0,640,480);
    draw_sprite(screen,logoBMP,320-logoBMP->w/2,10);
    textout_centre_ex(screen,data[50].dat,"Thanks for playing!",320,280,-1,-1);
    textout_centre_ex(screen,data[52].dat,"DESIGN & CODING: Johan Peitz",320,360,-1,-1);
    textout_centre_ex(screen,data[52].dat,"GRAPHICS: Emanuel Garnheim",320,390,-1,-1);
    fadeIn(screen,16);
    closeButtonClicked=0;
    cycle_count=0;
    while (!key[KEY_ESC] && cycle_count<=149) {
        gc=cycle_count;
        checkMenuFocus();
        if (bg_menu)
            adjust_sample(bg_menu,(int)vol,128,1000,1);
        while (gc==cycle_count)
            rest(2);
        if (closeButtonClicked)
            break;
        vol-=vol_step;
    }
    fadeOut(16);
}

void show_instructions(void)
{
    int done;

    blit(data[126].dat,swap_screen,0,0,0,0,640,480);
    masked_blit(data[70].dat,swap_screen,0,0,0,0,640,480);
    while (is_any(&ctrl))
        poll_control(&ctrl,0);
    fadeIn(swap_screen,16);
    done=0;
    while (!closeButtonClicked && !done) {
        cycle_count=0;
        checkMenuFocus();
        poll_control(&ctrl,0);
        done=is_fire(&ctrl)!=0;
        if (key[KEY_ESC] || key[KEY_ENTER])
            done=1;
        while (!cycle_count)
            rest(2);
    }
    fadeOut(16);
}

void play_menu_move(void)
{
    play_sound(menu_sounds[1], 0, 0);
}

void play_menu_select(void)
{
    play_sound(menu_sounds[0], 0, 0);
}

void testWindowResolution(void)
{
    if (window) {
        if (!options.full_screen)
            return;
        {
            PALETTE pal;
            log2file("Switching to fullscreen (640x480)");
            get_palette(pal);
            show_mouse(NULL);
            set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0);
            set_palette(pal);
            window=0;
            set_display_switch_mode(SWITCH_BACKAMNESIA);
            set_display_switch_callback(SWITCH_IN,switchedToProgram);
            set_display_switch_callback(SWITCH_OUT,switchedFromProgram);
            if (window)
                return;
        }
    }
    if (options.full_screen)
        return;
    {
        PALETTE pal;
        log2file("Switching to window (640x480)");
        get_palette(pal);
        set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0);
        set_palette(pal);
        window=1;
        set_display_switch_mode(SWITCH_BACKGROUND);
        set_display_switch_callback(SWITCH_IN,switchedToProgram);
        set_display_switch_callback(SWITCH_OUT,switchedFromProgram);
        show_mouse(screen);
    }
}

/* Source recovery of main.c:5136, 0x4100f8..0x410f95.  This retains the
 * oracle's complete menu-frame lifecycle while the exact historical drawing
 * expansion is still classified DIFFER. */
void main_menu_callback(void)
{
    int old_msc;
    const int scroller_step = -1;
    BITMAP *head_bmp;
    BITMAP *head_shadow;
    BITMAP *head;
    int headX;
    int headY;
    char welcomeMessage[512];
    int mouseInAd;
    int i;

    count++;                                                        /* 5142 */
    if (new_rand() % 198 == 1) {                                     /* 5143 */
        face++;                                                      /* 5144 */
        if (face == 3)
            face = 0;
    }

    if (key[KEY_F1]) {                                               /* 5147 */
        take_screenshot(swap_screen);                                /* 5148 */
        while (key[KEY_F1])                                          /* 5149 */
            rest(2);                                                 /* 5161 */
    }
    testWindowResolution();                                          /* 5152 */

    if (pFLDAd) {                                                    /* 5156 */
        mouseInAd = mouse_x < pFLDAdBitmap->w && gfx_driver &&        /* 5158 */
                    mouse_y > gfx_driver->h - pFLDAdBitmap->h;

        if (key[KEY_F5]) {                                           /* 5160 */
            while (key[KEY_F5])
                rest(2);
            options.full_screen = 0;                                 /* 5165 */
            testWindowResolution();                                  /* 5166 */
            open_web_browser((char *)pFLDAd->pVisitURL);              /* 5169 */
            my_alert("Icy Tower", "Your web browser has been opened.", 0, 1); /* 5170 */
        } else if (mouseInAd) {
            if ((mouse_b & 1) && !(lastMouseB & 1)) {
                testWindowResolution();
                open_web_browser((char *)pFLDAd->pVisitURL);
                my_alert("Icy Tower", "Your web browser has been opened.", 0, 1);
            }
        }
        lastMouseB = mouse_b;                                        /* 5172 */
        if (mouseInAd)
            _win_hcursor = LoadCursorA(NULL, IDC_HAND);
        else
            _win_hcursor = LoadCursorA(NULL, IDC_ARROW);
    } else {
        lastMouseB = mouse_b;
    }

    if (data && data[126].dat)                                       /* 5187 */
        blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);
    if (data && data[71].dat)                                        /* 5193 */
        draw_sprite(swap_screen, data[71].dat, 330, 280);
    if (pFLDAdBitmap) {                                               /* 5200 */
        set_alpha_blender();                                         /* 5201 */
        draw_trans_sprite(swap_screen, pFLDAdBitmap, 0, 280);         /* 5202 */
    }

    head = data[58 + face].dat;
    head_shadow = data[61].dat;
    head_bmp = create_bitmap(640, 320);
    clear_to_color(head_bmp, makecol(255, 0, 255));
    headX = fixsin(itofix(count * 3) + 10);
    headY = fixtoi(fixsin(itofix(count * 5)) * 10) + 16;
    rotate_sprite(head_bmp, head_shadow, 402, headY, headX * 5);
    set_trans_blender(0, 0, 0, 110);
    draw_trans_sprite(swap_screen, head_bmp, 0, 0);
    destroy_bitmap(head_bmp);
    headX = fixsin(itofix(count * 3) + 10);
    headY = fixtoi(fixsin(itofix(count * 5)) * 10) + 12;
    rotate_sprite(swap_screen, head, 400, headY, headX * 5);

    scroll_scroller(&greeting_scroller, scroller_step);               /* 5220 */
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                           /* 5221 */
    set_trans_blender(0, 0, 0, 110);                                  /* 5222 */
    rectfill(swap_screen, 0, 0, 639, 20, makecol(0, 0, 0));           /* 5223 */
    rectfill(swap_screen, 0, 0, 639, 18, makecol(0, 0, 0));           /* 5224 */
    rectfill(swap_screen, 0, 0, 639, 16, makecol(0, 0, 0));           /* 5225 */
    solid_mode();                                                     /* 5226 */
    draw_scroller(&greeting_scroller, swap_screen, 1, 0, makecol(150, 150, 150)); /* 5227 */
    if (!draw_scroller(&greeting_scroller, swap_screen, 0, 0,        /* 5228 */
                       makecol(200, 200, 200)))
        restart_scroller(&greeting_scroller);

    textprintf_ex(swap_screen, data[54].dat, 5, 3, makecol(100, 21, 20), -1,
                  "v%s %s", "1.5.1", debug ? " FUN MODE" : "");
    textprintf_ex(swap_screen, data[54].dat, 4, 2, makecol(162, 90, 51), -1,
                  "v%s %s", "1.5.1", debug ? " FUN MODE" : "");
    if (stricmp(profile->handle, "guest")) {
        sprintf(welcomeMessage, "Welcome, %%s! %s",
                get_rank_id(profile) ? "Your rank is %s." : "");
        textprintf_right_ex(swap_screen, data[54].dat, 638, 3,
                            makecol(50, 50, 50), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
        textprintf_right_ex(swap_screen, data[54].dat, 637, 3,
                            makecol(50, 50, 50), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
        textprintf_right_ex(swap_screen, data[54].dat, 637, 2,
                            makecol(200, 200, 200), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
        textprintf_right_ex(swap_screen, data[54].dat, 636, 2,
                            makecol(255, 255, 255), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
    } else {
        strcpy(welcomeMessage,
               "Welcome to Icy Tower! Start a profile in the profile menu.");
        textprintf_right_ex(swap_screen, data[54].dat, 638, 3,
                            makecol(50, 50, 50), -1, "%s", welcomeMessage);
        textprintf_right_ex(swap_screen, data[54].dat, 637, 3,
                            makecol(50, 50, 50), -1, "%s", welcomeMessage);
        textprintf_right_ex(swap_screen, data[54].dat, 637, 2,
                            makecol(200, 200, 200), -1, "%s", welcomeMessage);
        textprintf_right_ex(swap_screen, data[54].dat, 636, 2,
                            makecol(255, 255, 255), -1, "%s", welcomeMessage);
    }

    options.snd_volume = get_slider_value(&snd_volume_slider);        /* 5255 */
    options.msc_volume = get_slider_value(&msc_volume_slider);        /* 5256 */

    if (characters[play_char.value].ok) {                             /* 5259 */
        for (i = 0; i < 256; i++) {                                   /* 5261 */
            play_char.pal[i].r = characters[play_char.value].pal[i].r; /* 5262 */
            play_char.pal[i].g = characters[play_char.value].pal[i].g; /* 5263 */
            play_char.pal[i].b = characters[play_char.value].pal[i].b; /* 5264 */
        }
    }
    play_char.bmp = characters[play_char.value].bmp;                  /* 5266 */
    curr_char = play_char.value;                                      /* 5267 */
    strcpy(profile->last_avatar, characters[play_char.value].name);   /* 5268 */

    floors.max = profile->best_floor > 999 ? 9 : profile->best_floor / 100; /* 5275 */
    profile->start_floor = floors.value < floors.max ? floors.value : floors.max; /* 5276 */
    menu_params.fo = floors.value * 3 + 17;                           /* 5277 */

    options.flash = get_selection_value(&eyecandy_selection);         /* 5282 */
    options.floor_shrink = floors.value;
    options.start_speed = get_selection_value(&scroll_speed_selection); /* 5285 */
    options.floor_size = get_selection_value(&floor_size_selection);  /* 5284 */
    options.gravity = get_selection_value(&gravity_selection);        /* 5283 */
    if (bg_menu)                     /* 5288 */
        adjust_sample(bg_menu, options.msc_volume, 128, 1000, 1);

    syncProfileFromOptions();                                         /* 972 */
}

void run_demo(char *file_name)
{
    int fo;

    if (file_name) {
        if (demo)
            destroy_replay(demo);
        demo = load_replay(file_name);
    }
    if (demo) {
        if (!itrcheck) {
            fo = profile->start_floor;
            profile->start_floor = 0;
        }
        else
            fo = 0;
        if (new_game()) {
            play();
            end_game();
        }
        if (!itrcheck) {
            profile->start_floor = fo;
            floors.value = fo;
        }
    }
}

/* Partial recovery of main.c:5337, 0x4073f8..0x4076c0.  The replay menu
 * uses a striped datafile backdrop and overlays the optional summary scroll. */
void replay_menu_callback(void)
{
    int i;

    for (i=0; i<640; i+=2) {
        vline(swap_screen,i,0,480,0);
        hline(swap_screen,0,i,640,0);
    }
    draw_sprite(swap_screen,data[87].dat,120,140);
    if (!summary_scroller_message[0])
        return;
    scroll_scroller(&summary_scroller,-2);
    drawing_mode(DRAW_MODE_TRANS,0,0,0);
    set_trans_blender(0,0,0,110);
    rectfill(swap_screen,0,0,639,20,makecol(0,0,0));
    rectfill(swap_screen,0,0,639,18,makecol(0,0,0));
    rectfill(swap_screen,0,0,639,16,makecol(0,0,0));
    solid_mode();
    draw_scroller(&summary_scroller,swap_screen,1,0,makecol(150,150,150));
    if (!draw_scroller(&summary_scroller,swap_screen,0,0,makecol(200,200,200)))
        restart_scroller(&summary_scroller);
}

/* Oracle: main.c:5367, 0x40bc44..0x40bf59.  The editor owns only its
 * temporary backing bitmap; callers retain the supplied string and screen. */
int get_string(BITMAP *bmp, char *string, int w, int max_chars, FONT *f,
               int pos_x, int pos_y, int colour, int bg_color)
{
    BITMAP *block = create_bitmap(w, text_height(f) + 2);
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789.!_";
    int i = strlen(string);
    int tick;
    int c;

    if (!block)
        return -1;
    blit(bmp, block, pos_x - 1, pos_y - 1, 0, 0, block->w, block->h);
    while (key[KEY_ENTER] || key[KEY_SPACE])
        ;
    clear_keybuf();
    tick = 0;
    for (;;) {
        if (closeButtonClicked) {
            destroy_bitmap(block);
            return 0;
        }
        tick++;
        cycle_count = 0;
        checkMenuFocus();
        string[i] = (tick & 8) ? '|' : ' ';
        string[i + 1] = 0;
        vsync();
        blit(block, bmp, 0, 0, pos_x - 1, pos_y - 1, block->w, block->h);
        if (bg_color >= 0)
            rectfill(bmp, pos_x, pos_y, pos_x + block->w - 1,
                     pos_y + block->h - 3, bg_color);
        textout_ex(bmp, f, string, pos_x + 2, pos_y, colour, -1);
        blit_to_screen(bmp);
        if (keypressed()) {
            c = readkey();
            switch (c >> 8) {
        case KEY_ESC:
            string[i] = 0;
            destroy_bitmap(block);
            return -1;
        case KEY_TAB:
        case KEY_UP:
        case KEY_DOWN:
            string[i] = 0;
            destroy_bitmap(block);
            return -2;
        case KEY_ENTER:
            string[i] = 0;
            destroy_bitmap(block);
            return 0;
        case KEY_BACKSPACE:
            i--;
            if (i < 0)
                i = 0;
            break;
        default:
            if (i < max_chars - 2 && strchr(letters, c) &&
                ((c >> 8) != KEY_SPACE || i) &&
                text_length(f, string) < w - 9)
                string[i++] = (char)c;
            }
        }
        while (!cycle_count)
            rest(2);
    }
}

void replaceBadCharacters(char *string, char newChar)
{
    int i;
    char letters[63] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (i = 0; i < (int)strlen(string); i++)
        if (!strchr(letters, string[i]))
            string[i] = newChar;
}

void drawSlot(BITMAP *dst, int x, int y, char *title, char *text, int color)
{
    textout_ex(dst, data[54].dat, title, x, y - 16, makecol(0, 0, 0), -1);
    rectfill(dst, x - 1, y - 1, x + 340, y + 18, makecol(255, 255, 255));
    rect(dst, x - 1, y - 1, x + 340, y + 18, makecol(80, 80, 80));
    textout_ex(dst, data[54].dat, text, x + 2, y, color, -1);
}

/* Source recovery of main.c:5474, 0x410f98..0x4119fd. */
int do_replay_menu(void)
{
    int ret = -1;
    int play_again = 0;
    int isGuest = !stricmp("guest",profile->handle);

    log2file(" replay_menu launched");
    while (!closeButtonClicked && ret!='l') {
        ret=handle_menu(replay_menu,&menu_params,&ctrl,swap_screen,
                        replay_menu_callback,180,160,0);
        if (ret=='e') {
            log2file("  play again selected");
            play_again=1;
            ret='l';
        }
        else if (ret=='|') {
            char lastGameFile[2048];
            log2file("  view replay selected");
            fadeOut(16);
            sprintf(lastGameFile,"%slast_game.itr",replay_directory);
            run_demo(lastGameFile);
        }
        else if (ret=='{') {
            char fname[512];
            char pname[512];
            char comment[512];
            char fpath[512];
            char buffer[1024];
            int status = !isGuest;
            int action;
            log2file("  save replay selected");
            memset(fname,' ',511);
            fname[0]=0;
            memset(pname,' ',511);
            strcpy(pname,isGuest ? "" : profile->handle);
            memset(comment,' ',511);
            comment[0]=0;
            status=!isGuest;
            while (!closeButtonClicked && status!='*') {
                stretch_sprite(swap_screen,data[86].dat,120,140,380,200);
                textout_ex(swap_screen,data[51].dat,"SAVE REPLAY",140,150,-1,-1);
                textout_ex(swap_screen,data[54].dat,"(enter to advance)",320,312,
                           makecol(80,80,80),-1);
                drawSlot(swap_screen,140,210,"Your name:",pname,
                         makecol(50,50,50));
                drawSlot(swap_screen,140,250,"Filename:",fname,
                         makecol(50,50,50));
                drawSlot(swap_screen,140,290,"Comment: (optional)",comment,
                         makecol(50,50,50));
                blit_to_screen(swap_screen);
                if (status==0) {
                    action=get_string(swap_screen,pname,340,512,data[54].dat,
                                     140,210,makecol(0,0,0),makecol(255,255,255));
                    replaceBadCharacters(pname,'_');
                    action++;
                    if (!action)
                        status='*';
                    else
                        status=1;
                    /* 5537 */
                    drawSlot(swap_screen,140,210,"Your name:",pname,
                             makecol(50,50,50));
                    /* 5538 */
                    drawSlot(swap_screen,140,250,"Filename:",fname,
                             makecol(50,50,50));
                    /* 5539 */
                    drawSlot(swap_screen,140,290,"Comment: (optional)",comment,
                             makecol(50,50,50));
                }
                else if (status==1) {
                    if (!fname[0] && pname[0]) {
                        sprintf(fname,"%s_%d_%d_%d",pname,demo->score,
                                demo->floor,demo->combo);
                        replaceBadCharacters(fname,'_');
                    }
                    action=get_string(swap_screen,fname,340,512,data[54].dat,
                                   140,250,makecol(0,0,0),makecol(255,255,255));
                    replaceBadCharacters(fname,'_');
                    if (action == -1)
                        status='*';
                    else
                        status=2;
                    /* 5581 */
                    drawSlot(swap_screen,140,210,"Your name:",pname,
                             makecol(50,50,50));
                    /* 5582 */
                    drawSlot(swap_screen,140,250,"Filename:",fname,
                             makecol(50,50,50));
                    /* 5583 */
                    drawSlot(swap_screen,140,290,"Comment: (optional)",comment,
                             makecol(50,50,50));
                    /* 5584 */
                    blit_to_screen(swap_screen);
                }
                else if (status==2) {
                    action=get_string(swap_screen,comment,340,42,data[54].dat,
                                           140,290,makecol(0,0,0),makecol(255,255,255));
                    if (action == -1)
                        status='*';
                    else if (action == -2)
                        status=!isGuest;
                    else
                        status=3;
                    /* 5581 */
                    drawSlot(swap_screen,140,210,"Your name:",pname,
                             makecol(50,50,50));
                    /* 5582 */
                    drawSlot(swap_screen,140,250,"Filename:",fname,
                             makecol(50,50,50));
                    /* 5583 */
                    drawSlot(swap_screen,140,290,"Comment: (optional)",comment,
                             makecol(50,50,50));
                    /* 5584 */
                    blit_to_screen(swap_screen);
                }
                else if (status==3) {
                    char lastGameFile[2048];
                    int thisChecksum;
                    sprintf(lastGameFile,"%slast_game.itr",replay_directory);
                    if (!pname[0]) {
                        status=0;
                        continue;
                    }
                    if (!fname[0]) {
                        status=1;
                        continue;
                    }
                    if (demo)
                        destroy_replay(demo);
                    demo=load_replay(lastGameFile);
                    if (!demo) {
                        my_alert("Failed to save replay.",
                                 "Temporary file not found.",0,1);
                        status=!isGuest;
                        continue;
                    }
                    thisChecksum=calc_replay_checksum(demo);
                    if (thisChecksum!=uberChecksum) {
                        my_alert("Failed to save replay.",
                                 "Temporary file mismatch.",0,1);
                        status=!isGuest;
                        continue;
                    }
                    strncpy(demo->name,pname,30);
                    strcpy(demo->comment,comment);
                    replace_extension(buffer,fname,"itr",512);
                    sprintf(fpath,"%s%s",replay_directory,buffer);
                    if (exists(fpath) &&
                        !my_alert("The file exists.","Do you want to overwrite it?",1,0)) {
                        status=1;
                        continue;
                    }
                    if (save_replay(replay_directory,buffer,demo,demo->size+2,1)<0) {
                        my_alert("Failed to save replay.",fpath,0,1);
                        status=1;
                    }
                    else {
                        my_alert("Replay saved.",0,0,1);
                        status='*';
                    }
                }
            }
        }
    }
    return play_again;
}

/* Partial recovery of main.c:5650, 0x40d454..0x40da56.  This mandatory
 * first-run flow captures the display, creates or loads a profile, and
 * returns with the profile list and remembered handle synchronized. */
void force_create_profile(void)
{
    BITMAP *bg;
    int y;
    int ok;
    char new_name[32];

    bg=create_bitmap(SCREEN_W,SCREEN_H);
    blit(screen,bg,0,0,0,0,SCREEN_W,SCREEN_H);
    memset(new_name,0,sizeof(new_name));
    for (;;) {
        int res;
        char buf[129];

        checkMenuFocus();
        blit(bg,screen,0,0,0,0,SCREEN_W,SCREEN_H);
        set_trans_blender(0,0,0,158);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        draw_sprite(screen,data[87].dat,100,120);
        textout_ex(screen,data[51].dat,"Welcome to Icy Tower",130,127,-1,-1);
        textout_ex(screen,data[54].dat,"Yo, wazup? In Icy Tower, all your highscores",130,160,0,-1);
        textout_ex(screen,data[54].dat,"and progress will be stored in a personal profile.",130,175,0,-1);
        textout_ex(screen,data[54].dat,"AWESOME!",130,190,0,-1);
        textout_ex(screen,data[54].dat,"Please enter a name for your profile:",130,220,0,-1);
        textout_right_ex(screen,data[54].dat,"...and press enter.",430,260,0,-1);
        rectfill(screen,129,240,430,258,makecol(255,255,255));
        rect(screen,129,240,430,258,makecol(80,80,80));
        blit_to_screen(screen);
        res=get_string(screen,new_name,300,32,data[54].dat,130,240,makecol(0,0,0),-1);
        if (res>=-1) {
            if (res==-1) {
            my_alert("You can create a profile later in the OPTIONS menu.","Oh Well...",0,1);
            profile=load_profile("guest");
            if (!profile)
                profile=create_profile("guest",1);
            syncOptionsFromProfile();
            break;
            }
            if (!new_name[0])
                continue;
            replaceBadCharacters(new_name,'_');
            profile=create_profile(new_name,0);
            if (!profile) {
                my_alert("That profile name is taken.","Ooops!",0,1);
                continue;
            }
            sprintf(buf,"Welcome %s!",profile->handle);
            my_alert(buf,"Your profile has been created!",0,1);
            break;
        }
    }
    destroy_bitmap(bg);
    strcpy(options.lastProfile,profile->handle);
    rebuild_profile_list(0);
}

void startMenuMusic(void)
{
    if (bg_menu)
        play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
}

void stopMenuMusic(void)
{
    if (bg_menu)
        stop_sample(bg_menu);
}

void checkMenuFocus(void)
{
    if (in_replay_menu)
        return;
    if (lastFocus==hasFocus)
        return;
    if (hasFocus)
        startMenuMusic();
    else
        stopMenuMusic();
    lastFocus=hasFocus;
}

/* Source recovery of main.c:5761, 0x415f10..0x4166a2.  This retains the
 * startup, main-menu action dispatch, game/replay transitions, and orderly
 * shutdown recovered from the original control-flow branches. */
int _mangled_main(int argc, char **argv)
{
    char full_path[1024];
    char logfilename[256];
    FILE *f;
    int i;
    int ret;
    int must_fade;
    int play_again;
    int redraw_menu;
    HMODULE hDebugLibrary;

    hDebugLibrary = LoadLibraryA("exchndl.dll");
    if (!hDebugLibrary)
        printf("No exception handler present, RPTs will not be generated");
    allegro_init();
    register_png_file_type();
    get_executable_name(full_path, sizeof(full_path));
    /* The third argument is the EMPTY string, not "data": the original stores
     * 0x4d4bb3 here (main.c:5792, offset 111) and the bytes at that address in
     * assets/icytower15.exe are a lone NUL. replace_filename then yields the
     * executable's own directory, which is what the following chdir enters and
     * where profiles/, gamepad.txt and the data/ folder all live. With "data"
     * the game chdirs one level too deep and nothing it needs is found. */
    replace_filename(working_directory, full_path, "",
                     sizeof(working_directory));
    chdir(working_directory);
    memset(logfilename, 0, sizeof(logfilename));
    get_logfile_path(logfilename, sizeof(logfilename));
    f = fopen(logfilename, "wt");
    if (f) {
        fprintf(f, "Icy Tower v%s - log file\n----------------------------\n",
                "1.5.1");
        fclose(f);
    }
    for (i = 0; i < argc; i++)
        if (!stricmp(argv[i], "-check"))
            itrcheck = 1;
    log2file("Game started with the following commands:");
    for (i = 0; i < argc; i++)
        log2file("    %s", argv[i]);
    log2file("Working directory is:\n    %s", working_directory);
    if (!init_game(argc,argv)) {
        if (!dropped_file_is_not_a_replay) {
            log2file("* Failed to initialize the game *");
            allegro_message("Failed to initialize the game.");
        }
        log2file("Cleaning up Allegro");
        uninit_game();
        log2file("Done...");
        return 1;
    }

    if (itrcheck && demo) {
        log2file("Running replay.");
        run_demo(NULL);
        if (itrcheck) {
            log2file("Exiting Allegro");
            allegro_exit();
            log2file("\nDone...");
            exit(0);
        }
    }
    if (itrcheck)
        load_new_ad_image();
    init_scroller(&greeting_scroller, data[54].dat, scroller_greetings,
                  640, 30, -1);
    menu_params.font=data[51].dat;
    menu_params.bullet=data[72].dat;
    menu_params.pos=0;
    menu_params.data=data;
    init_control(&menu_params.ctrl);
    reset_menu(main_menu,&menu_params,0);
    startMenuMusic();
    clear_keybuf();

    must_fade=1;
    if (options.timesStarted==1 && !stricmp("guest",options.lastProfile)) {
        main_menu_callback();
        draw_menu(swap_screen,main_menu,&menu_params,355,285,0);
        fadeIn(swap_screen,16);
        force_create_profile();
        syncOptionsFromProfile();
        must_fade=0;
    }
    redraw_menu=1;
    while (!closeButtonClicked) {
        if (redraw_menu) {
            main_menu_callback();
            draw_menu(swap_screen,main_menu,&menu_params,355,285,0);
            if (must_fade)
                fadeIn(swap_screen,16);
            else
                blit_to_screen(swap_screen);
            redraw_menu=0;
        }
        ret=handle_menu(main_menu,&menu_params,&ctrl,swap_screen,
                        main_menu_callback,355,285,0);

        if (ret=='e' || ret==0x85) {
            in_replay_menu=(ret!='e');
            fadeOut(16);
            stopMenuMusic();
            if (demo) {
                destroy_replay(demo);
                demo=NULL;
            }
            do {
                play_again=0;
                if (new_game()) {
                    play_again=play();
                    end_game();
                    fadeOut(16);
                } else
                    fadeOut(16);
            } while (play_again && !closeButtonClicked);
            if (bg_menu)
                play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
            must_fade=1;
        }
        else if (ret=='i') {
            view_scores(hisc_tables,hisc_names);
            must_fade=0;
        }
        else if (ret=='h') {
            fadeOut(16);
            show_instructions();
            must_fade=1;
        }
        else if (ret=='z') {
            if (demo) {
                destroy_replay(demo);
                demo=NULL;
            }
            for (;;) {
                demo=replay_selector(&ctrl,replay_directory);
                if (!demo) {
                    must_fade=0;
                    break;
                }
                fadeOut(16);
                stopMenuMusic();
                run_demo(NULL);
                fadeOut(16);
                main_menu_callback();
                draw_menu(swap_screen,main_menu,&menu_params,355,285,0);
                fadeIn(swap_screen,32);
                if (closeButtonClicked)
                    break;
                if (bg_menu)
                    if (options.msc_volume)
                        play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
            }
        }
        else if (ret=='k')
            break;
        rest(2);
        if (closeButtonClicked || ret=='k')
            break;
        redraw_menu=1;
    }
    fadeOut(16);
    show_credits();
    stopMenuMusic();
    uninit_game();
    return 0;
}

END_OF_MAIN()
