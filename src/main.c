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
#include <allegro.h>
#include "port/game/port_game.h"
#include "port/config/port_config.h"
#include "port/sim/snapshot.h"
/* port: render snapshots around draw_frame (port/game/snapshot_capture.c) */
void port_snapshot_pre(void);
void port_snapshot_post(void);
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
char last_log[1024];
char working_directory[1024];
#include "recovered/Tcommandline.h"
typedef Tcommandline Tcmdline;
double seed;
int hasFocus = 1;
int lastFocus = 1;
int window = -1;
char sfx_file[512];
int got_joystick;
int scroll_count;
int scroll_delay;
int last_stripe_y;

#include "recovered/Trecord.h"
#include "recovered/Treplay.h"
extern int get_string(BITMAP*, char*, int, int, FONT*, int, int, int, int);
extern void drawSlot(BITMAP*, int, int, char*, char*, int);
Tcontrol ctrl;
extern DATAFILE *data;

void blit_to_screen(BITMAP *bmp);
void checkMenuFocus(void);

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
int collision_type = 0;
Tbeta *testers = 0;
Tbeta *the_tester = 0;
Tcommandline cmdline = {0};
int debug = 0;
int init_ok = 0;
int itrcheck = 0;
int dropped_file_is_not_a_replay = 0;
int any11 = 0;
int any12 = 0;
int any13 = 0;
int any21 = 0;
int any22 = 0;
int any23 = 0;
int is_playing_custom_game = 0;
int gdLastJumpDiff = 0;
int gdComboStart = 0;
BITMAP *swap_screen = 0;
BITMAP *poster = 0;
int bg_stripe_ids[5] = {0};
Thisc_table *hisc_tables[15] = {0};
int new_personal_best[15] = {0};
DATAFILE *data = 0;
DATAFILE *sfx = 0;
int fall_count = 0;
int clock_angle = 0;
int cycle_loops = 0;
Treplay *demo = 0;
int fast_forward = 0;
int fast_fast_forward = 0;
int uberChecksum = 0;
Tgame_data *gameData = 0;
int closeButtonClicked = 0;
int lastMouseB = 0;
int num_chars = 0;
int curr_char = 0;
Tavailable_profile *profiles = 0;
int numProfiles = 0;
Tprofile *profile = 0;
SAMPLE *combo_sound[10] = {0};
SAMPLE *bg_beat = 0;
SAMPLE *bg_menu = 0;
SAMPLE *jump_sound[3] = {0};
SAMPLE *speaker[3] = {0};
SAMPLE *menu_sounds[2] = {0};
SAMPLE *sounds[9] = {0};
Tmenu_floor_selection floors = {0};
BITMAP *pFLDAdBitmap = 0;
const FLDAdSpot *pFLDAd = 0;
int in_replay_menu = 0;

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
char *hints[45] = {
    "How much is left to reach the next rank?",
    "Check out your profile to see what you need to do to reach the next rank!",
    "In your profile you can see all your records! Check it out!",
    "How many times did you jump? Check out your profile!",
    "Tell your friends about Icy Tower. The more the merrier!",
    "Is that really your best?",
    "Was that really your best?",
    "You can do better than that!",
    "You can do better! One more time!",
    "Just one more time! Please?",
    "Make combo jumps for lots of score!",
    "Bounce on the walls to maintain your speed!",
    "AGAIN!",
    "Play again!",
    "Didn't you see that coming?",
    "Awww... Try again!",
    "That was close! You'll make it next time!",
    "Did you beat your high score yet?",
    "Calm down, it's not as hard as it seems.",
    "Come on, concentrate!",
    "Come on, focus!",
    "Better luck next time!",
    "Harold the Homeboy really likes cheese!",
    "There is always room for improvement! Once more!",
    "Icy Tower is also available for iPhone and iPod Touch! That's so exciting!",
    "Icy Tower is available on mobile phones! Have you tried it yet?",
    "Don't miss Icy Tower for iPhone and iPod Touch! Available now, yay!",
    "Now you can play Icy Tower on your iPhone phone as well, check it out now!",
    "Wanna play Icy Tower anywhere? Check out Icy Tower for mobile phones!",
    "Everyone is talking about Icy Tower for iPhone, you should get it too!",
    "Get Icy Tower for your phone and play anywhere, anytime!",
    "Awesome news: Icy Tower for mobile phones is out now!",
    "Challenge your friends in Icy Tower on Facebook!",
    "Now you can play anytime you want to! Icy Tower on your iPhone!",
    "Maybe you should try playing with a different character?",
    "Did you try playing with a different character?",
    "Icy Tower is a game from Free Lunch Design. More awesome gamea on our website!",
    "You are playing Icy Tower, the game that everyone loves!",
    "Everybody loves Icy Tower, you too!",
    "Play Icy Tower on Facebook! Compete with your friends!",
    "Have you tried Icy Tower on Facebook yet?",
    "Icy Tower is also available on Facebook. Try it out!",
    "Play Icy Tower on Facebook!",
    "Play Icy Tower with your friends on Facebook!",
    "Join all the Icy Tower fans on Facebook!"
};
Toptions options;
Tcustom custom;
int reward_time;
fixed reward_scale;
BITMAP *reward_bmp;
Tparticle stars[512];
Tcharacter *characters;
Tmenu_char_selection play_char;
Tplayer *ply[1000];
int player_id;
Tmap map;
int checkMusicVoiceID = -1;
int rejump;
Tjump_sequence jumpSequence;
int gameMusicVoiceID = -1;
int start_speeds[6] = { 5, 4, 3, 2, 1, 0 };
char *version_str = "1.5.1";
Tmenu_slider snd_volume_slider = { 0, 0, 250, 25 };
Tmenu_slider msc_volume_slider = { 0, 0, 250, 25 };
Tmenu_selection eyecandy_selection;
Tmenu_selection scroll_speed_selection;
Tmenu_selection floor_size_selection;
Tmenu_selection gravity_selection;
Tmenu_params menu_params;
char replay_directory[1024];
int rec_pos;
int recording;
int rec_seed;
int hurry_y;
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
void pwd_garble_string(char *str, int k);
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
    static char logfilename[1024];
    va_list ptr;
    PACKFILE *fp;
    if (itrcheck) return;
    if (!logfilename[0]) get_logfile_path(logfilename, sizeof(logfilename));
    fp = port_fopen(logfilename, "at");
    if (fp) {
        va_start(ptr, format);
        vfprintf(fp, format, ptr);
        vsprintf(last_log, format, ptr);
        fputc('\n', fp);
        fclose(fp);
    }
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

    if (text_length(data[51].dat, func ? func : " ") >
        text_length(data[51].dat, txt ? txt : " "))
        w = text_length(data[51].dat, func ? func : " ");
    else
        w = text_length(data[51].dat, txt ? txt : " ");
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
    release_screen();
    poll_control(&ctrl, 0);                                    /* 493 */
    poll_control(menu_ctrl, 0);                                 /* 494 */
    while (is_any(&ctrl) || is_any(menu_ctrl) || key[KEY_ESC]) { /* 495 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0); rest(2);
    }
    clear_keybuf();                                            /* 500 */
    done = 0;
    status = 0;
    while (!closeButtonClicked && !done) {                     /* 502 */
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
            draw_sprite(screen, data[status ? 11 : 10].dat, 240, 220); /* 527 */
            draw_sprite(screen, data[status ? 7 : 8].dat, 365, 220);   /* 529 */
        }
        while (!cycle_count) rest(2);                         /* 532 */
    }
    poll_control(&ctrl, 0);                                    /* 535 */
    poll_control(menu_ctrl, 0);                                 /* 536 */
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

void pwd_garble_string(char *str, int k)
{
    int i;
    int len_i;

    len_i = strlen(str);
    for (i = 0; i < len_i; i++)
        str[i] ^= k - i;
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
    fp = port_fopen(fileName, "rb");
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
    fp = port_fopen(newFile, "wb");
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
            log2file(" %s (%s): %s", name, buf,
                !characters[count].bmp ? "error" : "ok");
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

    /* port: game-relative path; the file layer searches the user and
     * asset roots (historically getcwd() + "/characters/") */
    strcpy(base_char_dir, "characters/");
    base_char_dir_len = strlen(base_char_dir);
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

    fp = port_fopen("password.txt", "rt");
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
    log2file(" opening '%s'", pURL);
    port_open_url(pURL);
}

/* Partial recovery of main.c:1375, 0x40e7dc..0x40fe78.  The oracle starts
 * with packfile/network state, command-line processing, and the platform
 * subsystems before it exposes the datafile-backed game globals. */
int init_game(int argc, char **argv)
{
    PACKFILE *fp;
    RGB black;
    int i;
    char title[64];
    char tmpHandle[32];
    char cfgfilename[256];

    tmpHandle[0]=0; /* 1382 */
    init_ok=0; /* 1385 */
    log2file("\nINIT GAME"); /* 1393 */
    packfile_password(NULL); /* 1394 */
    sprintf(title,"Icy Tower v%s","1.5.1"); /* 1395 */
    set_window_title(title); /* 1395 */
    /* port: Winsock was only needed by the removed advertising module */
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
        int check;
        char *checkFile;
        checkFile=NULL;
        check=0;
        for (i=1;i<argc;i++) { /* 1493 */
            if (argv[i][0]!='-') checkFile=argv[i]; /* 1494 */
            if (!stricmp(argv[i],"-check")) check=1; /* 1497 */
            if (!stricmp(argv[i],"-jumps")) cmdline.jumps=1; /* 1500 */
            if (!stricmp(argv[i],"-combos")) cmdline.combos=1; /* 1503 */
            if (!stricmp(argv[i],"-sd")) cmdline.sd=1; /* 1506 */
            if (!stricmp(argv[i],"-keys")) cmdline.keys=1; /* 1509 */
            if (!stricmp(argv[i],"-all")) { /* 1512 */
                cmdline.keys=1; /* 1513 */
                cmdline.jumps=1; /* 1514 */
                cmdline.combos=1; /* 1515 */
                cmdline.sd=1; /* 1516 */
            }
            if (!stricmp(argv[i],"-tiny")) cmdline.tiny=1; /* 1518 */
        }
        if (check) { /* 1524 */
            log2file("Loading %s",argv[2]); /* 1525 */
            demo=load_replay(checkFile); /* 1526 */
            if (!demo) { /* 1527 */
                set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1528 */
                printf("<itrcheck_results status=\"error\">%s</itrcheck_results>\n",
                       get_filename(checkFile)); /* 1529 */
                log2file("*** Failed!"); /* 1530 */
                dropped_file_is_not_a_replay=1; /* 1531 */
                return 0; /* 1532 */
            }
        }
        else {
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1536 */
            allegro_message("<%s>\nis not a vaild option.",argv[1]); /* 1537 */
            log2file("*** Erroneous option (%s)",argv[1]); /* 1538 */
            dropped_file_is_not_a_replay=1; /* 1539 */
            return 0;
        }
        itrcheck=1; /* 1544 */
        log2file("ITRCHECK activated, checking <%s>",checkFile); /* 1545 */
    } else if (argc==2) { /* 1550 */
        log2file("Loading %s",argv[1]); /* 1551 */
        demo=load_replay(argv[1]); /* 1552 */
        if (!demo) { /* 1553 */
            strcpy(tmpHandle,get_filename(argv[1])); /* 1555 */
            {
                char *ext;
                ext=get_extension(tmpHandle); /* 1556 */
                ext[-1]=0; /* 1557 */
            }
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
    log2file("Creating hiscore tables"); /* 1575 */
    for (i=0;i<15;i++) { /* 1576 */
        hisc_tables[i]=make_hisc_table(hisc_names[i]); /* 1577 */
        if (!hisc_tables[i]) { /* 1578 */
            log2file(" *** failed"); /* 1579 */
            set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1580 */
            allegro_message("Failed reserve memory for highscore table."); /* 1581 */
            return 0;
        }
    }
    for (i=0;i<15;i++) /* 1585 */
        reset_hisc_table(hisc_tables[i],"Harold",1000,0); /* 1586 */
    log2file("Initiating controls"); /* 1590 */
    init_control(&ctrl); /* 1591 */
    get_configfile_path(cfgfilename,sizeof(cfgfilename)); /* 1595 */
    log2file("Loading config file"); /* 1597 */
    fp=pack_fopen(cfgfilename,"rp"); /* 1598 */
    if (fp) { /* 1599 */
        load_options(&options,fp); /* 1600 */
        for (i=0;i<15;i++) /* 1601 */
            if (!load_hisc_table(hisc_tables[i],fp)) /* 1602 */
                reset_hisc_table(hisc_tables[i],"FLD",1000,0); /* 1603 */
        pack_fclose(fp); /* 1606 */
    } else
    {
        log2file(" *** failed"); /* 1609 */
        log2file("Resetting to default config"); /* 1610 */
        reset_options(&options); /* 1611 */
    }
    /* port: the display mode is a port setting (icytower-port.ini); keep
     * the game's Fullscreen option in sync with it */
    options.full_screen = port_config_fullscreen() ? -1 : 0;
    if (tmpHandle[0]) { /* 1614 */
        log2file("Setting last profile"); /* 1615 */
        strcpy(options.lastProfile,tmpHandle); /* 1616 */
    }
    if (!itrcheck) { /* 1619 */
        options.timesStarted++; /* 1620 */
        log2file("Game started %d times",options.timesStarted); /* 1621 */

    set_color_depth(desktop_color_depth()); /* 1624 */
    if (!options.full_screen) { /* 1627 */
        log2file("Setting windowed mode 640x480"); /* 1628 */
        if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0)<0) { /* 1629 */
            log2file(" *** failed"); /* 1633 */
            options.full_screen=-1; /* 1634 */
        } else
            window=1; /* 1630 */
    }
    if (options.full_screen) {
        log2file("Setting fullscreen mode 640x480"); /* 1638 */
        if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0)<0) { /* 1639 */
            log2file(" *** failed"); /* 1643 */
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
    install_mouse(); /* 1659 */
    enable_hardware_cursor(); /* 1660 */
    select_mouse_cursor(2); /* 1661 */
    if (!options.full_screen) /* 1663 */
        show_mouse(screen); /* 1664 */
    log2file("Graphics mode set. (screen = %d)",screen); /* 1668 */

    {
    DATAFILE *loader;
    BITMAP *fldLogo;
    int whiteColor;
    textprintf_centre_ex(screen,font,320,220,makecol(180,180,180),-1,
                         "please wait"); /* 1671 */
    set_color_conversion(COLORCONV_NONE); /* 1674 */
    packfile_password("(c) Free Lunch Design"); /* 1675 */
    loader=load_datafile("data/loading.dat"); /* 1676 */
    log2file("Loading loader"); /* 1677 */
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

    }
    log2file("Setting focus modes"); /* 1721 */
    if (options.full_screen)
        set_display_switch_mode(SWITCH_BACKAMNESIA); /* 1723 */
    else
        set_display_switch_mode(SWITCH_BACKGROUND); /* 1726 */
    log2file("Setting focus callbacks"); /* 1728 */
    set_display_switch_callback(SWITCH_IN,switchedToProgram); /* 1729 */
    set_display_switch_callback(SWITCH_OUT,switchedFromProgram); /* 1730 */
    set_close_button_callback(clickedCloseButton); /* 1731 */
    hist_srand((unsigned int)time(NULL)); /* 1733 */
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
            int i;
            Tgamepad *gp;
            gp=get_gamepad(); /* 1765 */
            log2file(" getting values from gamepad.txt"); /* 1766 */
            set_config_file("gamepad.txt"); /* 1767 */
            gp->up=get_gamepad_value("up"); /* 1768 */
            gp->left=get_gamepad_value("left"); /* 1769 */
            gp->right=get_gamepad_value("right"); /* 1770 */
            gp->down=get_gamepad_value("down"); /* 1771 */
            for (i=0;i<32;i++) { /* 1772 */
                char buf[8];
                sprintf(buf,"b%d",i+1); /* 1774 */
                gp->b[i]=get_gamepad_value(buf); /* 1775 */
            }
        } else {
            Tgamepad *gp;
            gp=get_gamepad(); /* 1779 */
            log2file(" gamepad.txt is missing, setting defaults"); /* 1780 */
            gp->up=4; /* 1781 */
            gp->left=1; /* 1782 */
            gp->right=2; /* 1783 */
            gp->down=8; /* 1784 */
            for (i=0;i<32;i++) /* 1785 */
                gp->b[i]=16; /* 1786 */
        }
    } else
        log2file(" no gamepad or joystick found, play with keyboard only"); /* 1791 */

    log2file("Reserving memory"); /* 1795 */
    draw_progress_bar(); /* 1796 */
    swap_screen=create_bitmap(640,480); /* 1797 */
    if (!swap_screen) { /* 1798 */
        log2file(" *** failed"); /* 1799 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1800 */
        allegro_message("Failed reserve memory screen buffers."); /* 1801 */
        return 0; /* 1802 */
    }

    pwd_garble_string(init_string,50); /* 1806 */
    log2file("Loading data"); /* 1818 */
    set_color_conversion(0x00ffffff); /* 1820 */
    draw_progress_bar(); /* 1821 */
    packfile_password(init_string); /* 1822 */
    data=load_datafile_callback("data/data.dat",datafile_callback_slow); /* 1823 */
    if (!data) { /* 1824 */
        log2file(" *** failed"); /* 1825 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1826 */
        allegro_message("Failed to load datafile."); /* 1827 */
        return 0; /* 1828 */
    }
    packfile_password(NULL); /* 1830 */
    }
    log2file("Initiating player"); /* 1835 */
    draw_progress_bar(); /* 1836 */
    player_id=hist_rand()%1000; /* 1837 */
    ply[player_id]=malloc(sizeof(*ply[player_id])); /* 1838 */
    if (!ply[player_id]) { /* 1839 */
        log2file(" *** failed"); /* 1840 */
        set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1841 */
        allegro_message("Failed to allocate memory for player."); /* 1842 */
        return 0; /* 1843 */
    }
    if (!itrcheck) { /* 1846 */
        char profiledir[1024];
        int last_cc;
        ((RGB *)data[0].dat)[0].r=((RGB *)data[0].dat)[0].g=((RGB *)data[0].dat)[0].b=0; /* 1849 */
        gameover_bmp=data[55].dat; /* 1850 */
        log2file("Checking profile directory"); /* 1855 */
        get_profiles_dir(profiledir,sizeof(profiledir)); /* 1858 */
        if (!file_exists(profiledir,FA_ALL,0)) { /* 1860 */
            log2file("  does not exist, trying to create"); /* 1861 */
            port_mkdir(profiledir); /* 1863 */
            if (!file_exists(profiledir,FA_ALL,0)) { /* 1867 */
                log2file("  *** failed!"); /* 1868 */
                set_gfx_mode(GFX_TEXT,0,0,0,0); /* 1869 */
                allegro_message("Failed to create profile directory %s",profiledir); /* 1870 */
                return 0; /* 1871 */
            }
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
        log2file(" loading sounds"); /* 1926 */
        packfile_password(init_string); /* 1927 */
        sfx=load_datafile_callback("data/sfx15.dat",datafile_callback); /* 1928 */
        if (!sfx) { /* 1929 */
            log2file(" could not load data/sfx15.dat"); /* 1930 */
            strcpy(sfx_file,"no sound"); /* 1931 */
            log2file(" no sounds loaded"); /* 1932 */
        }
        else {
            strcpy(sfx_file,"sfx15.dat"); /* 1935 */
            log2file(" sfx15.dat loaded"); /* 1936 */
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
        }
        log2file("Setting menu values"); /* 1978 */
        snd_volume_slider.value=options.snd_volume; /* 1979 */
        msc_volume_slider.value=options.msc_volume; /* 1980 */
        eyecandy_selection.value=options.flash; /* 1981 */
        gravity_selection.value=options.gravity; /* 1982 */
        floor_size_selection.value=options.floor_size; /* 1983 */
        scroll_speed_selection.value=options.start_speed; /* 1984 */
        floors.max=profile->best_floor>999 ? 9 : profile->best_floor/100; /* 1986 */
        floors.value=profile->start_floor>floors.max ? floors.max : profile->start_floor; /* 1987 */
        log2file("Cleaning up"); /* 2063 */
        draw_progress_bar(); /* 2064 */
        log2file("Welcome to Icy Tower"); /* 2069 */
        draw_progress_bar(); /* 2070 */
        last_cc=0; /* 2072 */
        while (!keypressed() && cycle_count<=149) { /* 2072 */
            if (!(cycle_count%10) && cycle_count!=last_cc) { /* 2073 */
                last_cc=cycle_count; /* 2074 */
                draw_progress_bar(); /* 2075 */
                last_cc=cycle_count; /* 2076 */
            }
            rest(2); /* 2078 */
        }
        seed=hist_rand()%2367; /* 2082 */
        fadeOut(16); /* 2083 */
        clear_bitmap(screen); /* 2084 */
        vsync(); /* 2085 */
        clear_keybuf(); /* 2086 */
    }
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
            int x = fixtoi((fixsin(itofix(y + logic_count * 5)) * ply[player_id]->level) / 2);              /* 2294 */
            blit(bmp, screen, 0, y, x, y, 640, 1);
        }
    }
    else if (blit_mode == 4) {                                 /* 2297 */
        int y = ply[player_id]->level % 480;                    /* 2298 */
        line(bmp, 0, 479, 639, 479, 0);
        line(bmp, 0, 0, 639, 0, 0);
        blit(bmp, screen, 0, 0, 0, y, bmp->w, bmp->h);           /* 2301 */
        blit(bmp, screen, 0, 0, 0, y - 480, bmp->w, bmp->h);     /* 2302 */
    }
    else if (blit_mode == 5) {                                 /* 2304 */
        int x = MID(0, ply[player_id]->x - 160.0, 320);         /* 2305 */
        int y = MID(0, ply[player_id]->y - 160.0, 240);         /* 2306 */
        stretch_blit(bmp, screen, x, y, 320, 240, 0, 0, 640, 480); /* 2307 */
    }
    else if (blit_mode == 6) {                                 /* 2309 */
        int x = MID(0, ply[player_id]->x - 80.0, 520);          /* 2310 */
        int y = MID(0, ply[player_id]->y - 80.0, 360);          /* 2311 */
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
        if (!ply[player_id]->dead &&
            !(demo->data[rec_pos].key_flags & 0x80)) {
            flags=control->flags&0x93;
            if (demo->data[rec_pos].key_flags==flags)
                demo->data[rec_pos].cycle_count++;
            else {
                rec_pos++;
                demo->data[rec_pos].key_flags=flags;
                demo->data[rec_pos].cycle_count=0;
            }
        }
        else {
            demo->data[rec_pos+1].key_flags=0x80;
            demo->data[rec_pos+1].cycle_count=0;
            demo->data[rec_pos+2].key_flags=0;
            demo->data[rec_pos+2].cycle_count=0;
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
{ int x, y;
    int p_im;
    int flip;

    int cx, cy;
    int ls;

    int fo = profile->start_floor * 3 + 17;
    int so = profile->start_floor + 101;

    frame_count++;


    int max_bg_id = 2;
    if (ply[player_id]->level > 200) max_bg_id = 3;
    if (ply[player_id]->level > 350) max_bg_id = 4;
    if (ply[player_id]->level > 600) max_bg_id = 5;


    while (map.offset / 256 > last_stripe_y) {
        last_stripe_y++;


        bg_stripe_ids[4] = bg_stripe_ids[3]; bg_stripe_ids[3] = bg_stripe_ids[2]; bg_stripe_ids[2] = bg_stripe_ids[1]; bg_stripe_ids[1] = bg_stripe_ids[0];


        if (new_rand() % 100 > 40)
            bg_stripe_ids[0] = 0;
        else {

            bg_stripe_ids[0] = new_rand() % max_bg_id;
            if (bg_stripe_ids[0] == bg_stripe_ids[1] || bg_stripe_ids[0] == bg_stripe_ids[2])
                bg_stripe_ids[0] = 0;
        }
    }



    for (ls = -1; ls < 4; ls++)
        blit(data[bg_stripe_ids[ls + 1] + 1].dat, bmp, 0, 0, 37, ls * 128 + (map.offset % 256) / 2, ((BITMAP *)data[bg_stripe_ids[ls + 1] + 1].dat)->w, ((BITMAP *)data[bg_stripe_ids[ls + 1] + 1].dat)->h);










    if (hurry_y > -100 && hurry_y < 480 && options.flash != 2)
        draw_sprite(bmp, data[67].dat, 320 - ((BITMAP *)data[67].dat)->w / 2, hurry_y);




    for (y = 31; y >= 0; y--) { cx = 464 - y * 16;
        if (!map.room[y].empty) {
            int f = fo + map.room[y].tiles * 3; if (f > 44) f = 44;
            if (map.room[y].level > 4999) f += 3;
            x = map.room[y].start_tile;
            draw_sprite(bmp, data[f].dat, x * 16 - 5, cx + (map.offset % 16) - 6);
            x++;
            while (x < map.room[y].end_tile) {
                draw_sprite(bmp, data[f + 1].dat, x * 16, cx + (map.offset % 16) - 6);
                x++;
            }
            draw_sprite(bmp, data[f + 2].dat, x * 16, cx + (map.offset % 16) - 6);

            if (debug && key[KEY_F2]) textprintf_ex(bmp, font, 520, cx + (map.offset % 16), 15, -1, "%d", (map.room[y].level - 1) / 5);
        }
        if (map.room[y].sign) {
            int s = so + map.room[y].tiles; if (s > 110) s = 110;
            if (map.room[y].level > 4999) s++;
            int sy = cx + (map.offset % 16) + 10;
            int sw = ((BITMAP *)data[s].dat)->w;
            cy = (map.room[y].start_tile + (map.room[y].end_tile - map.room[y].start_tile) / 2) * 16;
            draw_sprite(bmp, data[s].dat, cy, sy);
            int c1 = makecol(255, 255, 255);
            int c2 = makecol(55, 55, 55);
            cy += sw / 2; textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 7, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy + 2, sy + 6, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy, sy + 6, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 5, c2, -1, "%d", map.room[y].sign);
            textprintf_centre_ex(bmp, data[54].dat, cy + 1, sy + 6, c1, -1, "%d", map.room[y].sign);
        }
    }


    for (fo = 0; fo < 512; fo++)
        if (stars[fo].intensity)
            draw_sprite(swap_screen, data[stars[fo].color + 117].dat, fixtoi(stars[fo].x), fixtoi(stars[fo].y));






    p_im = 6; if (!ply[player_id]->status) p_im = 1;
    if (ply[player_id]->status == 3 && ply[player_id]->sy > 3.0) p_im = 7;
    if (ply[player_id]->status == 2 && ply[player_id]->sy > 3.0) p_im = 7;
    if (ply[player_id]->status == 1 && ply[player_id]->sy < -3.0) p_im = 5;

if (!ply[player_id]->status) if (!ply[player_id]->status) if (!ply[player_id]->status) { if (ABS(ply[player_id]->sx) < 0.02) p_im = 0; }
    if (p_im >= 5 && p_im <= 7 && ABS(ply[player_id]->sx) < 0.01) p_im = 8;

    if (p_im != 1 || ABS(ply[player_id]->sx) < 0.2) ply[player_id]->frame = 0;
    else
    if (ply[player_id]->frame > 3) ply[player_id]->frame = 0;





    BITMAP *customFrame = custom.frame[0];
    int oy = 1 - customFrame->h;
    int ox = 0;

    if (!p_im) {

        if (ply[player_id]->edge) {
            if (logic_count & 8) p_im = 13; else p_im = 14;


            customFrame = custom.frame[p_im];

            if (ply[player_id]->edge == 2)
                draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x - customFrame->w + 11, (int)ply[player_id]->y + oy);




            else
                draw_sprite(bmp, customFrame, (int)ply[player_id]->x - 11, (int)ply[player_id]->y + oy);
        }
        else {


            if (map.offset > 200 && ply[player_id]->y > 400.0) customFrame = custom.frame[11];
            else if (logic_count <= 11) if (logic_count <= 11) customFrame = custom.frame[9];
            else if (logic_count > 24)
                if (logic_count <= 36) customFrame = custom.frame[10];



            ox = -(customFrame->w / 2);

            if (ply[player_id]->sx > 0.0) draw_sprite(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy); else draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        }
    }
    else {

        if (ply[player_id]->rotate) {
            customFrame = custom.frame[12]; cx = customFrame->w / 2;
            rotate_sprite(bmp, customFrame, (int)ply[player_id]->x - cx, (int)ply[player_id]->y - 8 - custom.frame[0]->h, ply[player_id]->angle);


        } else {
            customFrame = custom.frame[p_im + ply[player_id]->frame];
            ox = -(customFrame->w / 2);
            if (ply[player_id]->sx > 0.0) draw_sprite(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy); else draw_sprite_h_flip(bmp, customFrame, (int)ply[player_id]->x + ox, (int)ply[player_id]->y + oy);
        }
    }












































    for (ls = -1; ls < 4; ls++) {
        draw_sprite(bmp, data[100].dat, 565, ls * 124 + (int)((map.offset % 84) * 1.476)); draw_sprite_h_flip(bmp, data[100].dat, -57, ls * 124 + (int)((map.offset % 84) * 1.476));
    }




    draw_sprite(bmp, data[16].dat, 22, 100);
    if (ply[player_id]->in_combo) {
        blit(data[15].dat, bmp, 0, 100 - ply[player_id]->in_combo, 33, 219 - ply[player_id]->in_combo, 16, ply[player_id]->in_combo);
        draw_sprite(bmp, data[14].dat, -8, 210);
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d", ply[player_id]->acc_level);
    }
    else if (reward_time) if (reward_time) if (reward_time) {
        draw_sprite(bmp, data[14].dat, -8, 210);
        textprintf_centre_ex(bmp, data[50].dat, 42, 210, -1, -1, "%d", ply[player_id]->latest_combo);
    }


    if (hurry_y < 251 || hurry_y > 479) { cx = 0; cy = 0; } else { cx = logic_count % 3 - 1; cy = (logic_count + 1) % 3 - 1; }
    draw_sprite(bmp, data[12].dat, cx + 6, cy + 10);
    if (hurry_y >= 201 && hurry_y <= 479) { cx = (logic_count + 2) % 3 - 1; cy = (logic_count + 3) % 3 - 1; }
    rotate_sprite(bmp, data[13].dat, cx + 34, cy + 28, clock_angle ? ftofix((clock_angle % 1500) * 0.1706666) : 0);
    if (reward_time)
        draw_reward(swap_screen);
















    textprintf_ex(bmp, data[52].dat, 8, 440, -1, -1, "score: %d", ply[player_id]->level * 10 + ply[player_id]->score);


    if (!recording) {
        char myBuf[256];
        int myPos = 0;

        if (frame_count & 8) {
            strcpy(myBuf, "REPLAY");
            myPos = 630 - text_length(data[53].dat, myBuf);
            textprintf_ex(bmp, data[53].dat, myPos + 1, 5, makecol(0, 0, 0), -1, "REPLAY");
            textprintf_ex(bmp, data[53].dat, myPos, 4, makecol(255, 255, 255), -1, "REPLAY");
        }


        if (is_playing_custom_game) {
            sprintf(myBuf, "%s Floors", floor_size_selection.caption[demo->floor_size]);
            cx = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 16, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, cx, 15, makecol(255, 255, 255), -1);

            sprintf(myBuf, "%s Speed", scroll_speed_selection.caption[demo->start_speed]);
            cx = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, cx + 1, 26, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, cx, 25, makecol(255, 255, 255), -1);

            strcpy(myBuf, gravity_selection.caption[demo->gravity]);
            myPos = 630 - text_length(data[53].dat, myBuf);
            textout_ex(bmp, data[53].dat, myBuf, myPos + 1, 36, makecol(0, 0, 0), -1);
            textout_ex(bmp, data[53].dat, myBuf, myPos, 35, makecol(255, 255, 255), -1);
        }



        BITMAP *vcr = data[127].dat;
        myPos = rec_pos; int len = demo->size;

        x = 635 - vcr->w;
        y = 475 - vcr->h;
        draw_sprite(bmp, vcr, x, y);
        if (!ply[player_id]->dead) {
            if (is_left(&ctrl)) draw_sprite(bmp, data[128].dat, x + 97, y + 5);
            if (is_fire(&ctrl)) draw_sprite(bmp, data[130].dat, x + 107, y + 5);
            if (is_right(&ctrl)) draw_sprite(bmp, data[129].dat, x + 117, y + 5);
        }
        cx = y + 10;
        cy = x + 10; set_clip_rect(bmp, cy, 0, 623, 479);
        char scrollerText[70];
        sprintf(scrollerText, "%s%s%s", demo->name, demo->comment[0] ? " - " : "", !demo->comment[0] ? "" : demo->comment);
        textout_ex(bmp, data[53].dat, demo->name, x + 12 - scroll_count / 2, cx + 4, makecol(150, 150, 160), -1);
        textout_ex(bmp, data[53].dat, demo->name, x + 13 - scroll_count / 2, cx + 4, makecol(200, 200, 210), -1);
        if (demo->comment[0]) {
            textout_ex(bmp, data[53].dat, " - ", x + 12 - scroll_count / 2 + text_length(data[53].dat, demo->name), cx + 4, makecol(200, 200, 210), -1);
            textout_ex(bmp, data[53].dat, demo->comment, x + 30 - scroll_count / 2 + text_length(data[53].dat, demo->name), cx + 4, makecol(200, 200, 210), -1);
        }
        set_clip_rect(bmp, 0, 0, 639, 479);

        if (demo->comment[0]) {
            if (scroll_delay > 0)
                scroll_delay--;
            else {

                scroll_count++;
                if (scroll_count / 2 > text_length(data[53].dat, scrollerText))
                    scroll_count = -250;
            }
        }


        rect(bmp, cy, cx + 20, cy + (myPos * 117 / len > 116 ? 116 : myPos * 117 / len), cx + 19, makecol(50, 200, 50));
    }


    if (debug && key[KEY_F2]) {
        textprintf_ex(bmp, font, 0, 0, 15, -1, "FPS:%6d / %d", fps, lps);
        textprintf_ex(bmp, font, 0, 10, 15, -1, "REC:%6d / %d", rec_pos, demo->size);
        textprintf_ex(bmp, font, 0, 20, 15, -1, "    %6d  (%d) ", demo->data[rec_pos].key_flags, demo->data[rec_pos].cycle_count);
        textprintf_ex(bmp, font, 200, 0, 15, -1, "POS: %d, %d", (int)ply[player_id]->x, (int)ply[player_id]->y);
        textprintf_ex(bmp, font, 200, 10, 15, -1, " dx: %1.2f", ply[player_id]->sx);
        textprintf_ex(bmp, font, 200, 20, 15, -1, "rjp: %d", options.jump_hold);
        textprintf_ex(bmp, font, 400, 0, 15, -1, "any: %6d %6d %6d", any11, any12, any13);
        textprintf_ex(bmp, font, 400, 10, 15, -1, "any: %6d %6d %6d", any21, any22, any23);
    }
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
    new_srand(hist_rand() % 0x18ff8);
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
        hist_srand(time(0));
        rec_seed = hist_rand();
        demo->random_seed = rec_seed;
    }

    for (i = 0; i < 15; i++)
        new_personal_best[i] = 0;
    hist_srand(rec_seed);
    log2file(" creating map layout");
    reset_map(&map);
    for (i = 0; i < 30; i++)
        add_floor(&map);
    snapshot_discontinuity();
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
    int left, right;


    int fx1 = 0, fy1, fx2 = 0, fy2;


    int plx1 = (int)ply[player_id]->x - 11;
    int ply1 = (int)ply[player_id]->y + 1;
    int plx2 = lastX - 11;
    int ply2 = lastY;


    int prx1 = (int)ply[player_id]->x + 11;
    int pry1 = ply1;
    int prx2 = lastX + 11;
    int pry2 = lastY;


    int ilx, ily, irx, iry;


    fy1 = -12345678;

    getFloorData(&map, (int)ply[player_id]->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) {

            if (ply[player_id]->status == 2 ||
                ply[player_id]->status == 0) ply[player_id]->status = 3;
            fx1 = fx2 = 0;
            return;
        }
    }
    fy2 = fy1;
    if (debug && key[KEY_F2]) {

        int col1 = makecol(255, 0, 0);
        int col2 = makecol(255, 255, 0);


        line(screen, fx1, fy1, fx2, fy2, col1);


        line(screen, plx1, ply1, plx2, ply2, col2);
        line(screen, prx1, pry1, prx2, pry2, col2);
    }


    left = line_intersect(fx1, fy1, fx2, fy2, plx1, ply1, plx2, ply2, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy2, prx1, pry1, prx2, pry2, &irx, &iry);


    if (left + right == 0 && (ply[player_id]->status == 2 ||
        ply[player_id]->status == 0)) ply[player_id]->status = 3;

    if (left != right) ply[player_id]->edge = left ? 1 : 2;
    else ply[player_id]->edge = 0;

    if (left + right != 0 && (ply[player_id]->status == 2 || ply[player_id]->status == 3)) {
        if (left && right && (ilx < -10000 || irx < -10000 || ilx > 10000 || irx > 10000))
            return;



        play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        ply[player_id]->y = fy1 - 1;
        ply[player_id]->x = left ? ilx + 11 : irx - 11;
        ply[player_id]->rotate = 0;
    }
}


/* Partial recovery of main.c, 0x4088c8..0x408d08.  This variant retries the
 * floor segment four pixels lower before transitioning to falling state. */
void handle_player_collision_vector_2(int lastX, int lastY)
{
    int left, right;


    int fx1, fy1, fx2, fy2;


    int plx1 = (int)ply[player_id]->x - 11;
    int ply1 = (int)ply[player_id]->y + 1;
    int plx2 = lastX - 11;
    int ply2 = lastY;


    int prx1 = (int)ply[player_id]->x + 11;
    int pry1 = ply1;
    int prx2 = lastX + 11;
    int pry2 = lastY;


    int ilx, ily, irx, iry;


    int col1 = makecol(255, 0, 0);
    int col2 = makecol(255, 255, 0);


    fy1 = -12345678;

    getFloorData(&map, (int)ply[player_id]->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) fx1 = fx2 = 0;
    }
    fy2 = fy1;

    if (debug) if (debug && key[KEY_F2]) {

        line(screen, fx1, fy1, fx2, fy2, col1);


        line(screen, plx1, ply1, plx2, ply2, col2);
        line(screen, prx1, pry1, prx2, pry2, col2);
    }


    left = line_intersect(fx1, fy1, fx2, fy2, plx1, ply1, plx2, ply2, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy2, prx1, pry1, prx2, pry2, &irx, &iry);

    if (!left && (left == -right || (left == 0 && (right ^ left) == 0))) {
        int snap = 4;
        left = line_intersect(fx1, fy1 + snap, fx2, fy2 + snap, plx1, ply1, plx2, ply2, &ilx, &ily);
        right = line_intersect(fx1, fy1 + snap, fx2, fy2 + snap, prx1, pry1, prx2, pry2, &irx, &iry);
    }


    if (left + right == 0) { if (ply[player_id]->status == 2) ply[player_id]->status = 3;
        if (ply[player_id]->status == 0) ply[player_id]->status = 3; }

    if (left != right && (right == 0 || left != right)) ply[player_id]->edge = left ? 1 : 2;
    else ply[player_id]->edge = 0;

    if (left + right != 0 && (ply[player_id]->status == 2 || ply[player_id]->status == 3)) {
        play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        ply[player_id]->y = fy1 - 1;
        ply[player_id]->x = !left ? irx - 11 : ilx + 11;
        ply[player_id]->rotate = 0;
    }
}


/* Partial recovery of main.c, 0x408358..0x4088c8.  Combo mode uses ordinary
 * solid-foot correction first, then a floor-segment landing intersection. */
void handle_player_collision_combo(int lastX, int lastY)
{
    int left, right;


    int fx1, fy1, fx2, fy2;


    int plx1 = (int)ply[player_id]->x - 11;
    int ply1 = (int)ply[player_id]->y + 1;
    int plx2 = lastX - 11;
    int ply2 = lastY;


    int prx1 = (int)ply[player_id]->x + 11;
    int pry1 = (int)ply[player_id]->y + 1;
    int prx2 = lastX + 11;
    int pry2 = lastY;


    int ilx, ily, irx, iry;


    int col1 = makecol(255, 0, 0);
    int col2 = makecol(255, 255, 0);

    int solid1, solid2;


    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    any11 = solid1;
    any12 = solid2;
    any21 = any22 = any23 = 0;
    if (solid1 + solid2 == 0) { if (ply[player_id]->status == 2) ply[player_id]->status = 3;
        if (ply[player_id]->status == 0) ply[player_id]->status = 3; }
    else if ((ply[player_id]->status != 1 && ((unsigned char)ply[player_id]->status != 1 || (unsigned short)ply[player_id]->status != 1 || ply[player_id]->status != 1))) if (ply[player_id]->status != 2) {
        if (ply[player_id]->status) play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        if (solid1) { ply[player_id]->y -= solid1 - 9999; ply[player_id]->rotate = 0; }
        else {
            if (solid2) { ply[player_id]->y -= solid2 - 9999; ply[player_id]->rotate = 0; }
            if (solid2 == 0 || solid1 + solid2 == 0 || (solid1 ^ solid2) == 0 || (solid1 | solid2) == 0) ply[player_id]->rotate = 0;
        }
        if (solid1 != solid2) ply[player_id]->edge = solid1 ? 1 : 2;
        else ply[player_id]->edge = 0;
        return;
    }



    fy1 = -12345678;

    getFloorData(&map, (int)ply[player_id]->y, &fy1, &fx1, &fx2);
    if (fy1 == -12345678) {
        getFloorData(&map, lastY, &fy1, &fx1, &fx2);
        if (fy1 == -12345678) fx1 = fx2 = 0;
    }
    fy2 = fy1;

    if (debug && key[KEY_F2]) {

        line(screen, fx1, fy1, fx2, fy2, col1);


        line(screen, plx1, ply1, plx2, ply2, col2);
        line(screen, prx1, pry1, prx2, pry2, col2);
    }


    left = line_intersect(fx1, fy1, fx2, fy2, plx1, ply1, plx2, ply2, &ilx, &ily);
    right = line_intersect(fx1, fy1, fx2, fy2, prx1, pry1, prx2, pry2, &irx, &iry);


    if (left + right == 0 && (ply[player_id]->status == 2 ||
        ply[player_id]->status == 0)) ply[player_id]->status = 3;

    if (left != right) ply[player_id]->edge = left ? 1 : 2;
    else ply[player_id]->edge = 0;

    if ((left + right != 0 || ((((left + right) & 3) != 0 && ((left + right) & 12) != 0) || (((left + right) & 3) == 0 && (((left + right) & 48) != 0 && ((left + right) & 192) != 0)))) && (ply[player_id]->status == 2 || ply[player_id]->status == 3)) {
        play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        ply[player_id]->y = fy1 - 1;
        ply[player_id]->x = left ? ilx + 11 : irx - 11;
        ply[player_id]->rotate = 0;
    }
}

/* Oracle: main.c, 0x407fd8..0x408358.  The legacy mode first tests the
 * current feet, then sweeps a midpoint when the player moved downward. */
void handle_player_collision_old(int lastX, int lastY)
{
    int midX, midY, dX, dY;
    int solid1, solid2;


    dX = lastX - (int)ply[player_id]->x; solid1 = dX; if(solid1 < 0) solid1 = (int)(0u-(unsigned)solid1); dX=solid1;
    dY = lastY - (int)ply[player_id]->y; if (dY < 0) dY = -dY;
    if ((int)ply[player_id]->x < lastX) midX = lastX - dX / 2;
    else midX = lastX + (dX >> 1);
    if ((int)ply[player_id]->y < lastY) midY = lastY - dY / 2;
    else midY = lastY + dY / 2;

    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    any11 = solid1;
    any12 = solid2;
    any21 = any22 = any23 = 0;
    if (solid1 + solid2 == 0) { if (ply[player_id]->status == 2) ply[player_id]->status = 3;
        if (ply[player_id]->status == 0) ply[player_id]->status = 3; }
    else if (ply[player_id]->status != 1) if (ply[player_id]->status != 2) {
        if (ply[player_id]->status) play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        if (solid1) ply[player_id]->y -= solid1 - 9999;
        else if (solid2) ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        if (solid1 != solid2) ply[player_id]->edge = solid1 ? 1 : 2;
        else ply[player_id]->edge = 0;
        return; }
    switch (midY > lastY) {
        case 1: {
        solid1 = is_solid(&map, midX - 11, midY);
        solid2 = is_solid(&map, midX + 11, midY);
        any21 = solid1;
        any22 = solid2;
        if (solid1 + solid2 == 0) { if (ply[player_id]->status == 2) ply[player_id]->status = 3;
            if (ply[player_id]->status == 0) ply[player_id]->status = 3; }
        else if ((ply[player_id]->status != 1 || (unsigned char)ply[player_id]->status != 1)) if (ply[player_id]->status != 2) {
            any23 = 1;
            if (ply[player_id]->status) play_sound(sounds[8], 1, 1);
            ply[player_id]->status = 0;
            ply[player_id]->sy = 0;
            if (solid1) ply[player_id]->y -= solid1 - 9999;
            else if (solid2) ply[player_id]->y -= solid2 - 9999;
            ply[player_id]->rotate = 0;
            if (solid1 != solid2) ply[player_id]->edge = solid1 ? 1 : 2;
            else ply[player_id]->edge = 0;
        }

            break;
        }
        case 2: any23 = 2; break;
        case 3: any23 = 3; break;
        case 0: break;
        default: break;
    }
}

void handle_player_collision_original(int lastX, int lastY)
{
    int solid1;
    int solid2;

    solid1 = is_solid(&map, (int)ply[player_id]->x - 11, (int)ply[player_id]->y);
    solid2 = is_solid(&map, (int)ply[player_id]->x + 11, (int)ply[player_id]->y);
    any11 = solid1;
    any12 = solid2;
    any21 = any22 = any23 = 0;
    if (solid1 + solid2 == 0) {
        if (ply[player_id]->status == 2) ply[player_id]->status = 3;
        if (ply[player_id]->status == 0) ply[player_id]->status = 3;
    }
    else if ((ply[player_id]->status != 1 || ((unsigned char)ply[player_id]->status != 1))) if (ply[player_id]->status != 2) {
        if (ply[player_id]->status) play_sound(sounds[8], 1, 1);
        ply[player_id]->status = 0;
        ply[player_id]->sy = 0;
        if (solid1) ply[player_id]->y -= solid1 - 9999;
        else if (solid2) ply[player_id]->y -= solid2 - 9999;
        ply[player_id]->rotate = 0;
        if (solid1 != solid2) ply[player_id]->edge = solid1 ? 1 : 2;
        else ply[player_id]->edge = 0;
    }
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
int quit; quit = 0;
int scroll_acc;
int scroll; scroll = -1;
int max_scroll;
int speeds[9] = { 1500, 3000, 4500, 6000, 7500, 9000, 10500, 1800000, 9000000 };
int next_speed; next_speed = 0;
int next_aight; next_aight = 50;
int allow_smpl; allow_smpl = 1;
int game_over; game_over = 0;
int falling; falling = 0;
int shake; shake = 0;
int flash;
int step_count; step_count = 0;
int next_floor; next_floor = -1;
int play_again;
int tot_scroll;
int lastX; int lastY; int midX; int midY;
int numComboJumps; numComboJumps = 0;
int totComboFloors; totComboFloors = 0;
int startTime;
int endTime; endTime = 0;
int lastJumpLength; lastJumpLength = 0;

int oldUnlockedFloors;
int current_rank_id;

Tcontrol rec_ctrl; rec_ctrl = ctrl;

if (!itrcheck) {
oldUnlockedFloors = profile->best_floor / 100;
current_rank_id = get_rank_id(profile); } else { current_rank_id = 0; oldUnlockedFloors = 0;
}




int time_cheat_count; time_cheat_count = 0;

clock_t clockTimeStart; clock_t clockTimeEnd;
double clockElapsed;
double totClockTimes;


int qpc_start; int qpc_end;

double qpc_elapsed;
double totQPCTimes;

int timeTimeStart; int timeTimeEnd;
int timeElapsed;
double totTimeTimes;

int musicCounter; musicCounter = 0;
int lastMusicPos; lastMusicPos = 0;
float accMusics; accMusics = 0.0f;
int totMusics; totMusics = 0;




if (recording) if (recording) {
demo->tc_posts = 0;
for (i = 0; i < 100; i++) {
demo->tc_c_data[i] = 0.0f;
demo->tc_q_data[i] = 0.0f;
demo->tc_t_data[i] = 0.0f;
demo->tc_s_data[i] = 0.0f;
demo->tc_f_data[i] = 0.0f;
}
}


log2file(" setting up play data");
fall_count = 0;
clock_angle = 0;
map.offset = 0;

fast_forward = 0;
fast_fast_forward = 0;

update_frame();
if (!itrcheck) {
port_snapshot_pre(); draw_frame(swap_screen); port_snapshot_post();

fadeIn(swap_screen, 16);
play_sound(custom.yo, 0, 0);
startGameMusic();
}

if (!itrcheck)
if (bg_beat) {
checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1);
}




cycle_count = 0;

log2file(" play started");
startTime = time(NULL);




qpc_start = port_qpc_low();



int qpc_freq;




clockTimeStart = clock();

timeTimeStart = time(NULL);


playing = TRUE;
while (playing) {

if (closeButtonClicked) return 0; {



cycle_count = 0;

logic_count++;
step_count++;
fall_count++;
time_cheat_count++;
musicCounter++;


if (!itrcheck) {
if (lastFocus != hasFocus) {
if (hasFocus) {
if (bg_beat) {
checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1);
}




startGameMusic(); totMusics = 0; accMusics = 0.0f; musicCounter = 0;
} else {

if (checkMusicVoiceID >= 0) {
voice_stop(checkMusicVoiceID);
}
checkMusicVoiceID = -1;

stopGameMusic();
}
lastFocus = hasFocus;
}
}


if (!itrcheck) { if (checkMusicVoiceID >= 0) {
int vgp; vgp = voice_get_position(checkMusicVoiceID);
if (vgp < lastMusicPos) { musicCounter = 0;
}


float a = vgp / 44000.0;
float b;

if (a > 0.01) {
b = musicCounter / 50.0; accMusics += b / a;
totMusics++; } lastMusicPos = vgp;
}
}







if (recording && map.offset > 100 && !ply[player_id]->dead) {

if (time_cheat_count == 1000) {



double clockSpeed; double qpcSpeed; double timeSpeed;


clockTimeEnd = clock();
clockElapsed = clockTimeEnd - clockTimeStart; clockSpeed = (50.0 * clockElapsed) / 1000.0;
if (clockSpeed > 0.0) { totClockTimes = (1000.0 * clockSpeed) / clockSpeed / 20.0; } else { totClockTimes = -0.05;
}



qpc_freq = port_qpf_low();
qpc_end = port_qpc_low();


qpc_elapsed = qpc_end - qpc_start; qpcSpeed = (50.0 * qpc_elapsed / qpc_freq) / 20.0; totQPCTimes = qpcSpeed;







timeTimeEnd = time(NULL);









timeElapsed = timeTimeEnd - timeTimeStart;
timeSpeed = (50.0 * timeElapsed) / 20.0;
totTimeTimes = timeSpeed;
demo->tc_c_data[demo->tc_posts] = 0.0 + totClockTimes;
demo->tc_q_data[demo->tc_posts] = 0.0 + totQPCTimes;
demo->tc_t_data[demo->tc_posts] = 0.0 + totTimeTimes;
demo->tc_f_data[demo->tc_posts] = ply[player_id]->level;
if (totMusics != 0) {
demo->tc_s_data[demo->tc_posts] = 50.0 * accMusics / totMusics;
}
demo->tc_posts = demo->tc_posts < 98 ? demo->tc_posts + 1 : 99;










clockTimeStart = clock();


qpc_start = port_qpc_low();



timeTimeStart = time(NULL); totMusics = 0; accMusics = 0.0f; time_cheat_count = 0;
}
}

















if (debug) {
if (key[KEY_1]) { if (allow_smpl) start_reward(5); }
if (key[KEY_2]) { if (allow_smpl) start_reward(7); }
if (key[KEY_3]) { if (allow_smpl) start_reward(15); }
if (key[KEY_4]) { if (allow_smpl) start_reward(25); }
if (key[KEY_5]) { if (allow_smpl) start_reward(35); }
if (key[KEY_6]) { if (allow_smpl) start_reward(50); }
if (key[KEY_7]) { if (allow_smpl) start_reward(70); }
if (key[KEY_8]) { if (allow_smpl) start_reward(100); }
if (key[KEY_9]) { if (allow_smpl) start_reward(140); }
if (key[KEY_0]) { if (allow_smpl) start_reward(200); }
allow_smpl = !(key[KEY_1] || key[KEY_2] || key[KEY_3] || key[KEY_4] || key[KEY_5] || key[KEY_6] || key[KEY_7] || key[KEY_8] || key[KEY_9] || key[KEY_0]);
}




midX = (int)ply[player_id]->x;
midY = (int)ply[player_id]->y;


handle_player_input(&ctrl);
update_player(ply[player_id]);


if (!itrcheck) {
if (ply[player_id]->rotate && ply[player_id]->in_combo && !options.flash)
create_particle(stars, (int)ply[player_id]->x, (int)ply[player_id]->y - 16);


for (i = 0; i < 512; i++) if (stars[i].intensity) update_particle(&stars[i]); } lastY = midY;





old_map_pos = map.offset; scroll_acc = 0;

if (ply[player_id]->y < 160.0) { scroll_acc = 1;

if (ply[player_id]->y < 140.0) if (ply[player_id]->y < 140.0) scroll_acc++;
if (ply[player_id]->y < 120.0) if (ply[player_id]->y < 120.0) scroll_acc++;
if (ply[player_id]->y < 100.0) if (ply[player_id]->y < 100.0) scroll_acc++;
if (ply[player_id]->y < 80.0) if (ply[player_id]->y < 80.0) scroll_acc++;
if (ply[player_id]->y < 60.0) if (ply[player_id]->y < 60.0) scroll_acc++;
if (ply[player_id]->y < 40.0) if (ply[player_id]->y < 40.0) scroll_acc += 2;
if (ply[player_id]->y < 20.0) if (ply[player_id]->y < 20.0) scroll_acc += 2;
if (ply[player_id]->y < 0.0) if (ply[player_id]->y < 0.0) scroll_acc += 3;
map.offset = old_map_pos + scroll_acc;

ply[player_id]->y += scroll_acc;
lastY = midY + scroll_acc; } tot_scroll = scroll_acc;



if (!ply[player_id]->dead) clock_angle++;

if (map.offset > 100 && !ply[player_id]->dead) {
if (scroll == -1)
scroll = start_speeds[demo->start_speed];

if (!scroll) {
if (step_count & 1) {
map.offset++;
tot_scroll++;
ply[player_id]->y += 1.0;
lastY++;
}
}
else {
map.offset += scroll;
tot_scroll += scroll;
ply[player_id]->y += scroll;
lastY += scroll;
}
}
else if (!ply[player_id]->dead) {
clock_angle = 0; fall_count = 0;
}



any13 = tot_scroll;

if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;
if (demo->speed_increase)
if (!ply[player_id]->dead && speeds[next_speed] < fall_count && scroll < 5) {
ply[player_id]->ccc[next_speed] = ply[player_id]->level;

next_speed++;
scroll++;
hurry_y = 479;
play_sound(speaker[0], 0, 0);
play_sound(sounds[4], 0, 0);
}


if (scroll == 5) {
fall_count -= 45;
if (!ply[player_id]->dead) clock_angle -= 45;
}

if (old_map_pos % 16 > map.offset % 16) {
add_floor(&map);

} else
if (tot_scroll > 15) {

add_floor(&map);
}























switch (collision_type) { case 0:
handle_player_collision_original(midX, lastY);
break; case 1:
handle_player_collision_old(midX, lastY);
break; case 2:
handle_player_collision_vector(midX, lastY);
break; case 3:
handle_player_collision_vector_2(midX, lastY);
break; case 4:
handle_player_collision_combo(midX, lastY);
break; default:

allegro_message("unknown collision type"); break;
}





if (ply[player_id]->rotate) ply[player_id]->angle += itofix(8);



if (ply[player_id]->in_combo) {
ply[player_id]->in_combo--;
if (!ply[player_id]->in_combo)
if (ply[player_id]->acc_jumps > 1) {
ply[player_id]->score += ply[player_id]->acc_level * ply[player_id]->acc_level;
int rewResult; rewResult = start_reward(ply[player_id]->acc_level);
if (recording && !is_playing_custom_game) profile->rewards[rewResult]++;
totComboFloors += ply[player_id]->acc_level;
numComboJumps++;

Tgd_combo c;
c.length = ply[player_id]->acc_level;
c.start = gdComboStart;
c.end = c.start + c.length;
add_combo(gameData, &c);

ply[player_id]->latest_combo = ply[player_id]->acc_level;
if (ply[player_id]->acc_level > ply[player_id]->best_combo)
ply[player_id]->best_combo = ply[player_id]->acc_level;
}
}




if (!ply[player_id]->status) {

level = (get_level(&map, (int)ply[player_id]->y) - 1) / 5;




diff = level - ply[player_id]->level;
if (diff != 0) {
if (diff != gdLastJumpDiff) {


jumpSequence.dist = gdLastJumpDiff;
add_jump_sequence(gameData, &jumpSequence);


jumpSequence.num = 1;
jumpSequence.start = level - diff;


} else jumpSequence.num++;


gdLastJumpDiff = diff;
}




if (level >= ply[player_id]->level) {

diff = level - ply[player_id]->level;


if (diff != lastJumpLength && diff != 0) {
for (i = 0; i < 5; i++) {


if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])
ply[player_id]->jcTop[i] = ply[player_id]->jc[i];


ply[player_id]->jc[i] = 0; } lastJumpLength = 0;
}





if (diff > 0) {
if (diff <= 5)
ply[player_id]->jc[diff - 1]++; lastJumpLength = diff;
}




if (diff > 0 && diff != 1) {
if (ply[player_id]->in_combo) {
ply[player_id]->acc_level += diff;
ply[player_id]->acc_jumps++;
ply[player_id]->in_combo = 100;
}
else {
ply[player_id]->acc_level = diff;
ply[player_id]->acc_jumps = 1;
ply[player_id]->in_combo = 100;
}
}

if (diff == 1 && ply[player_id]->in_combo)
ply[player_id]->in_combo = 1;


if (!ply[player_id]->in_combo)
gdComboStart = level;
}
else {



if (ply[player_id]->in_combo) ply[player_id]->in_combo = 1;

for (i = 0; i < 5; i++) {


if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])
ply[player_id]->jcTop[i] = ply[player_id]->jc[i];


ply[player_id]->jc[i] = 0; } lastJumpLength = 0;
}








ply[player_id]->level = level;




if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level)

ply[player_id]->no_combo_top_floor = gdComboStart;
}





if (ply[player_id]->y > 540.0 && !ply[player_id]->dead) {
if (itrcheck) playing = FALSE;
if (ply[player_id]->in_combo && ply[player_id]->acc_jumps > 1)
ply[player_id]->biggest_lost_combo = ply[player_id]->acc_level;

ply[player_id]->in_combo = 0;
ply[player_id]->dead = 1;
play_sound(custom.falling, 0, 1);

endTime = time(0);


for (i = 0; i < 5; i++) {


if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])
ply[player_id]->jcTop[i] = ply[player_id]->jc[i];


ply[player_id]->jc[i] = 0;
}


jumpSequence.dist = gdLastJumpDiff;
add_jump_sequence(gameData, &jumpSequence);


if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level) if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level)
ply[player_id]->no_combo_top_floor = ply[player_id]->level; lastJumpLength = 0; falling = 1;
}




if (ply[player_id]->y > 900.0 && !game_over) {

play_sound(speaker[1], 0, 0); game_over = 2;
}

if (falling) falling++;
if (falling > ply[player_id]->level * 5 || falling > 250) {
play_sound(sounds[6], 0, 1);
if (custom.falling)
stop_sample(custom.falling);


ply[player_id]->shake = 24; falling = 0;
}



if (ply[player_id]->level >= next_aight) {
play_sound(sounds[2], 0, 0);
if (!options.flash) for (i = 0; i < next_aight / 2; i++) {
int p; p = create_particle(stars, (new_rand() % 600) + 20, 480);
stars[p].sy = -(((new_rand() % 200) << 16) / 10);
}
if (next_aight > 999)
next_aight += 500;
else

next_aight += 50;
}



if (!ply[player_id]->edge) ply[player_id]->edge_drawn = 0;
if (ply[player_id]->edge_drawn) {
if (ply[player_id]->edge_drawn == 11 && !ply[player_id]->status) play_sound(custom.edge, 1, 1);
if (ply[player_id]->edge_drawn == 50)
ply[player_id]->edge_drawn = 0;
}

if (debug) {
if (ply[player_id]->dead <= 99) playing = FALSE;
}




else if (recording && ply[player_id]->dead > 100) playing = FALSE;





if (!itrcheck && key[KEY_F1]) {
int pauseTime = time(NULL);
take_screenshot(swap_screen);
while (key[KEY_F1]) ;
int addTime = time(NULL) - pauseTime;
if (addTime > 0)
startTime += addTime;






if (checkMusicVoiceID >= 0) {

musicCounter = voice_get_position(checkMusicVoiceID) * 50.0f / 44000.0f; totMusics = 0; accMusics = 0.0f;
}




clockTimeStart = clock();


qpc_start = port_qpc_low();



timeTimeStart = time(NULL); time_cheat_count = 0;
}


if (ply[player_id]->shake || ((unsigned char)ply[player_id]->shake && (unsigned short)ply[player_id]->shake && ply[player_id]->shake)) {
ply[player_id]->shake--;

shake = new_rand() % 8;
}

update_frame();



if (!quit && closeButtonClicked) { quit = 1; playing = FALSE;
}



if (recording) {
if (key[KEY_ESC]) {
if (ply[player_id]->dead) {
log2file("  player quit after dying"); playing = 0;
} else {



int pauseTime = time(NULL);
int fc = fall_count;
int ca = clock_angle;
log2file("  game paused with esc");

for (i = 0; i < 640; i += 2) {
vline(swap_screen, i, 0, 480, 0);
hline(swap_screen, 0, i, 640, 0);
}
textout_centre_ex(swap_screen, data[50].dat, "DO YOU REALLY WANT TO EXIT?", 320, 160, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "Press any key to resume", 320, 210, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "Press ESC to exit", 320, 240, -1, -1);
blit_to_screen(swap_screen);
play_sound(custom.wazup, 0, 1);

poll_control(&ctrl, 0);
while (is_any(&ctrl) || is_pause(&ctrl) || (!closeButtonClicked && key[KEY_ESC])) {
poll_control(&ctrl, 0);
rest(2);
}
clear_keybuf();
while (!keypressed() && !is_any(&ctrl) && !is_pause(&ctrl) && !closeButtonClicked && !key[KEY_ESC]) {
poll_control(&ctrl, 0);
rest(2);
}
while (!closeButtonClicked && is_pause(&ctrl)) {
poll_control(&ctrl, 0);
rest(2);
}
if (key[KEY_ESC]) {



log2file("  game quit from esc pause");
profile->games_quit++;
endTime = time(NULL); quit = 1; playing = 0;
}
clear_keybuf();

fall_count = fc;
clock_angle = ca;
log2file("  game unpaused");
int addTime = time(NULL) - pauseTime;
if (addTime > 0)
startTime += addTime;






if (checkMusicVoiceID >= 0) {

musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); totMusics = 0; accMusics = 0.0f;
}



clockTimeStart = clock();


qpc_start = port_qpc_low();



timeTimeStart = time(NULL); time_cheat_count = 0;
}
}

if (is_pause(&ctrl) && ply[player_id]->dead == 0) {
int pauseTime = time(NULL);
int fc = fall_count;
int ca = clock_angle;
log2file("  game paused with pause key");

for (i = 0; i < 640; i += 2) {
vline(swap_screen, i, 0, 480, 0);
hline(swap_screen, 0, i, 640, 0);
}
textout_centre_ex(swap_screen, data[50].dat, "Game Paused", 320, 160, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "Press any key to resume", 320, 210, -1, -1);
blit_to_screen(swap_screen);
play_sound(custom.wazup, 0, 1);
poll_control(&ctrl, 0);
while (is_any(&ctrl) || is_pause(&ctrl)) {
poll_control(&ctrl, 0);
rest(2);
}

clear_keybuf();
while (!keypressed()) { if (is_any(&ctrl) || is_pause(&ctrl) || key[KEY_ESC]) break;
poll_control(&ctrl, 0);
rest(2);
}

poll_control(&ctrl, 0);
while (is_pause(&ctrl) || key[KEY_ESC]) {
poll_control(&ctrl, 0);
rest(2);
}


fall_count = fc;
clock_angle = ca;
log2file("  game unpaused");
int addTime = time(NULL) - pauseTime;
if (addTime > 0)
startTime += addTime;






if (checkMusicVoiceID >= 0) {

musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); totMusics = 0; accMusics = 0.0f;
}



clockTimeStart = clock();


qpc_start = port_qpc_low();



timeTimeStart = time(NULL); time_cheat_count = 0;
}
}
else {
if (!itrcheck) {

poll_control(&rec_ctrl, 0);

if (ply[player_id]->dead) {

log2file("  replay ended after death"); playing = 0;
}







if (key[KEY_ESC]) {
log2file("  quit from replay"); quit = 1; playing = 0;
}




if (key[KEY_SPACE]) { if (ply[player_id]->dead == 0) {
log2file("  replay paused");
while (key[KEY_SPACE]) poll_control(&rec_ctrl, 1);
while (!key[KEY_SPACE] && !key[KEY_RIGHT] && !key[KEY_ESC] && !key[KEY_UP]) {
poll_control(&rec_ctrl, 1);
if (key[KEY_F1]) {
take_screenshot(swap_screen);
while (key[KEY_F1]) ;
}
}
while (key[KEY_SPACE]) poll_control(&rec_ctrl, 1);
fast_forward = 0;
fast_fast_forward = 0;
log2file("  replay unpaused");
}
}
if (key[KEY_RIGHT]) {
fast_forward++;
fast_fast_forward = 0;
}
else
fast_forward = 0;


if (key[KEY_UP]) {
if (!ply[player_id]->dead && ply[player_id]->level < demo->floor - 10) {
fast_fast_forward++;
fast_forward = 0;
next_floor = ((ply[player_id]->level + 100) / 100) * 100;

next_floor = MIN(next_floor, demo->floor - 10); } else { fast_fast_forward = 0; next_floor = -1;
}
}





else if (ply[player_id]->level >= next_floor || ply[player_id]->dead) {
fast_fast_forward = 0; next_floor = -1;
}
}
}





if (!itrcheck) {
static int someCounter;
int ffstep;
int drew;

someCounter++;


ffstep = fast_forward ? 4 : 1;


if (fast_fast_forward) ffstep = 32;



int skipDrawing;


if (!quit && someCounter % ffstep == 0) {
port_snapshot_pre(); draw_frame(swap_screen); port_snapshot_post();







if (ply[player_id]->shake) { acquire_screen();

blit(swap_screen, swap_screen, 0, shake, 0, 0, swap_screen->w, swap_screen->h);
blit_to_screen(swap_screen); release_screen();
} else {


blit_to_screen(swap_screen);
}

if (!debug) {
while (cycle_count == 0) rest(2);


} else if (key[KEY_TAB] && key[KEY_LSHIFT]) {
while (cycle_count <= 7) { rest(0); }
} else {
while (!cycle_count) rest(2);
}
}
}


if (!itrcheck) rest(2);
}
}


if (recording) {
int addTime = endTime - startTime;
if (addTime > 0)
profile->seconds_spent_playing += addTime;
} else {










gameData->score = ply[player_id]->level * 10 + ply[player_id]->score;
gameData->floor = ply[player_id]->level;
gameData->combo = ply[player_id]->best_combo;
gameData->no_combo_top_floor = ply[player_id]->no_combo_top_floor;
gameData->biggest_lost_combo = ply[player_id]->biggest_lost_combo;

for (i = 0; i < 5; i++) gameData->ccc[i] = ply[player_id]->ccc[i];


for (i = 0; i < 5; i++) gameData->jc[i] = ply[player_id]->jcTop[i];
{


int keys_pressed[7] = {0};
int key_flag[7] = { 16, 1, 2, 4, 8, 32, 128 };
int last_keys[7] = {0};

if (demo->size > 0) { for (i = 0; i < demo->size; i++) {
int k;
for (k = 0; k < 7; k++) {
if (!last_keys[k] && (key_flag[k] & demo->data[i].key_flags))
keys_pressed[k]++;

last_keys[k] = key_flag[k] & demo->data[i].key_flags;
}
}
}
gameData->jump = keys_pressed[0];
gameData->left = keys_pressed[1];
gameData->right = keys_pressed[2];


if (itrcheck) {
char *xmlStr = getGameDataXML(gameData);
printf("%s", xmlStr);
free(xmlStr);
}
} if (itrcheck) if (itrcheck) return 0;
}




























log2file(" play ended");
fast_forward = 0;
fast_fast_forward = 0;









































if (recording) { if (!quit) {


demo->score = ply[player_id]->level * 10 + ply[player_id]->score;
demo->floor = ply[player_id]->level;
demo->combo = ply[player_id]->best_combo;
demo->rejump = options.jump_hold;
demo->no_combo_top_floor = ply[player_id]->no_combo_top_floor;
demo->biggest_lost_combo = ply[player_id]->biggest_lost_combo;

for (i = 0; i < 5; i++) demo->ccc[i] = ply[player_id]->ccc[i];


for (i = 0; i < 5; i++) demo->jc[i] = ply[player_id]->jcTop[i];





if (!is_playing_custom_game) {
profile->games_played++;

profile->total_floors += demo->floor;
profile->total_score += demo->score;
profile->total_combos += numComboJumps;
profile->total_combo_floors += totComboFloors;
for (i = 0; i < 5; i++) {
if (demo->ccc[i] > 0) {
profile->cccNum[i]++;
profile->cccTotal[i] += demo->ccc[i];
}
}
} else {

profile->custom_games_played++;
}





if (!file_exists(replay_directory, -1, NULL))

port_mkdir(replay_directory);






if (!is_playing_custom_game) {
if (profile->best_floor < demo->floor) {
profile->best_floor = demo->floor;
myDeleteFile(replay_directory, profile->best_replay_names[2]);
sprintf(profile->best_replay_names[2], "%s_best_floor_%d.itr", profile->handle, demo->floor);
save_replay(replay_directory, profile->best_replay_names[2], demo, rec_pos + 2, 1);
new_personal_best[2] = 1;
}

if (profile->best_combo < demo->combo) {
profile->best_combo = demo->combo;
myDeleteFile(replay_directory, profile->best_replay_names[1]);
sprintf(profile->best_replay_names[1], "%s_best_combo_%d.itr", profile->handle, demo->combo);
save_replay(replay_directory, profile->best_replay_names[1], demo, rec_pos + 2, 1);
new_personal_best[1] = 1;
}

if (profile->best_score < demo->score) {
profile->best_score = demo->score;
myDeleteFile(replay_directory, profile->best_replay_names[0]);
sprintf(profile->best_replay_names[0], "%s_best_score_%d.itr", profile->handle, demo->score);
save_replay(replay_directory, profile->best_replay_names[0], demo, rec_pos + 2, 1);
new_personal_best[0] = 1;
}

if (profile->no_combo_top_floor < ply[player_id]->no_combo_top_floor) {
profile->no_combo_top_floor = ply[player_id]->no_combo_top_floor;
myDeleteFile(replay_directory, profile->best_replay_names[4]);
sprintf(profile->best_replay_names[4], "%s_best_no_combo_%d.itr", profile->handle, demo->no_combo_top_floor);
save_replay(replay_directory, profile->best_replay_names[4], demo, rec_pos + 2, 1);
new_personal_best[4] = 1;
}

if (profile->biggest_lost_combo < ply[player_id]->biggest_lost_combo) {
profile->biggest_lost_combo = ply[player_id]->biggest_lost_combo;
myDeleteFile(replay_directory, profile->best_replay_names[3]);
sprintf(profile->best_replay_names[3], "%s_best_lost_combo_%d.itr", profile->handle, demo->biggest_lost_combo);
save_replay(replay_directory, profile->best_replay_names[3], demo, rec_pos + 2, 1);
new_personal_best[3] = 1;
}

for (i = 1; i < 6; i++) {
if (profile->ccc[i - 1] < ply[player_id]->ccc[i - 1]) {
profile->ccc[i - 1] = ply[player_id]->ccc[i - 1];
myDeleteFile(replay_directory, profile->best_replay_names[4 + i]);
sprintf(profile->best_replay_names[4 + i], "%s_best_cc%d_%d.itr", profile->handle, i, ply[player_id]->ccc[i - 1]);
save_replay(replay_directory, profile->best_replay_names[4 + i], demo, rec_pos + 2, 1);
new_personal_best[4 + i] = 1;
}
}

for (i = 1; i < 6; i++) {
if (profile->jc[i - 1] < ply[player_id]->jcTop[i - 1]) {
profile->jc[i - 1] = ply[player_id]->jcTop[i - 1];
myDeleteFile(replay_directory, profile->best_replay_names[9 + i]);
sprintf(profile->best_replay_names[9 + i], "%s_best_jj%d_%d.itr", profile->handle, i, ply[player_id]->jcTop[i - 1]);
save_replay(replay_directory, profile->best_replay_names[9 + i], demo, rec_pos + 2, 1);
new_personal_best[9 + i] = 1;
}
}
}


if (save_replay(replay_directory, "last_game.itr", demo, rec_pos + 2, 1) < 0) {
my_alert("Failed to save replay.", "(last_game.itr)", 0, 1);
uberChecksum = 0;
} else {


char fbuf[2048];
sprintf(fbuf, "%slast_game.itr", replay_directory);
Treplay *rr; rr = load_replay(fbuf);
if (rr) {
uberChecksum = calc_replay_checksum(demo);
destroy_replay(rr);
}
}
}
}











syncProfileFromOptions();
save_profile(profile);
play_again = 0;
if (!quit && !closeButtonClicked) {
float hy;
int gotHigh; gotHigh = 0;


int qualify[15];
int qualifyValue[15];
for (i = 0; i < 15; i++) qualify[i] = 0;

qualifyValue[0] = ply[player_id]->level * 10 + ply[player_id]->score;
qualifyValue[2] = ply[player_id]->level;
qualifyValue[1] = ply[player_id]->best_combo;
qualifyValue[3] = ply[player_id]->biggest_lost_combo;
qualifyValue[4] = ply[player_id]->no_combo_top_floor;
for (i = 0; i < 5; i++) {
qualifyValue[5 + i] = ply[player_id]->ccc[i];
qualifyValue[10 + i] = ply[player_id]->jcTop[i];
}

for (i = 0; i < 15; i++) {
qualify[i] = qualify_hisc_table(hisc_tables[i], qualifyValue[i]);
gotHigh += qualify[i];
}


if (!recording) gotHigh = 0;

int gameover_bmp_id; gameover_bmp_id = gotHigh > 0 ? 62 : 55;
if (is_playing_custom_game) gameover_bmp_id = 0x37;

if (gotHigh && !is_playing_custom_game) {
log2file(" player qualified for highscore");
play_sound(sounds[7], 0, 0);
} else {

log2file(" player did not qualify for highscore");
play_sound(speaker[1], 0, 0);
}


if (!debug) {
int done = 20;
int alpha_pos = 0;
int pos = 0;
char letters[31] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ .\244\0";
int len = strlen(letters) - 1;
char buf[8] = { '.', 0, '.', 0, '.', 0, 0, 0 };
int skip_keys = 0;

int isGuest = !stricmp(profile->handle, "guest");
hy = 480.0f;
float hyTarget = 140.0f;
while (hy > hyTarget) {
cycle_count = 0;
ply[player_id]->dead -= 16;

hy = hy + (130.0f - hy) * 0.1;
update_frame();
for (i = 0; i < 512; i++) if (stars[i].intensity) update_particle(&stars[i]);
if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;
port_snapshot_pre(); draw_frame(swap_screen); port_snapshot_post();
draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify, qualifyValue, is_playing_custom_game ? 0 : (recording != 0));
if (isGuest && gotHigh && !is_playing_custom_game && recording) {
textout_centre_ex(swap_screen, data[52].dat, "Enter your initials", 320, (int)(hy * 2 + 80), -1, -1);
}

if (falling) falling++;
if (falling > ply[player_id]->level * 5 || falling > 250) {
play_sound(sounds[6], 0, 1);
if (custom.falling) stop_sample(custom.falling);

ply[player_id]->shake = 24; falling = 0;
}
if (ply[player_id]->shake) { acquire_screen();

blit(swap_screen, screen, 0, new_rand() % 8, 0, 0, swap_screen->w, swap_screen->h); release_screen();

ply[player_id]->shake--; } else

blit_to_screen(swap_screen);


if (key[KEY_F1]) {
take_screenshot(swap_screen);
while (key[KEY_F1]) { }
}


if (!key[KEY_TAB] || !key[KEY_LSHIFT])


while (!cycle_count) rest(2);
}

ply[player_id]->dead = 0;
clear_keybuf();



if (!recording)
summary_scroller_message[0] = 0;
else {
if (is_playing_custom_game) {
strcpy(summary_scroller_message, "Custom mode is crazy fun but does not add to your profile. " "Play Classic Mode to compete in the highscore lists and " "climb in rank!");
} else {

if (gotHigh) {
int skipCategories[5];
int h;
int achs;
strcpy(summary_scroller_message, "New personal records!    ");
















strcpy(summary_scroller_message, isGuest ? "You're playing in guest mode. Start a profile and record your progress!" : hints[new_rand() % 45]);



} else {
strcpy(summary_scroller_message, isGuest ? "You're playing in guest mode. Start a profile and record your progress!" : hints[new_rand() % 45]);
}
}
}


init_scroller(&summary_scroller, data[54].dat, summary_scroller_message, 640, 30, -1);
scroll_scroller(&summary_scroller, -150);
int scrollerTargetY = 0;
int scrollerY = -20;


int new_rank_id; new_rank_id = get_rank_id(profile);
int rank_bmp_id; rank_bmp_id = new_rank_id + 0x4a;
int rank_y; rank_y = 0x244;
int rankTargetY = 320;

while (done) {
if (closeButtonClicked) return 0;



cycle_count = 0;
step_count++;

update_frame();

if (key[KEY_F1]) {
take_screenshot(swap_screen);
while (key[KEY_F1]) { }
}


if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;
port_snapshot_pre(); draw_frame(swap_screen); port_snapshot_post();
draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify, qualifyValue, is_playing_custom_game ? 0 : (recording != 0));
if (isGuest && gotHigh && !is_playing_custom_game && recording) {
textout_centre_ex(swap_screen, data[52].dat, "Enter your initials", 320, (int)(hy * 2 + 80), -1, -1);

if (pos != 0 || (step_count & 4)) if (pos != 0 || (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, &buf[0], 300, (int)(hy * 2 + 120), -1, -1);
if (pos != 1 || (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, &buf[2], 320, (int)(hy * 2 + 120), -1, -1);
if (pos != 2 || (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, &buf[4], 340, (int)(hy * 2 + 120), -1, -1);
if (pos == 3 && (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, "%", 360, (int)(hy * 2 + 120), -1, -1);
}

if (new_rank_id != current_rank_id) {
draw_sprite(swap_screen, data[rank_bmp_id].dat, 20, rank_y);
textout_ex(swap_screen, data[52].dat, "rank up!", 20, rank_y + 0x46, -1, -1);
rank_y = (int)((rankTargetY - rank_y) * 0.1 + rank_y);
}


if (summary_scroller_message[0]) {
scroll_scroller(&summary_scroller, -2);
drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
set_trans_blender(0, 0, 0, 110);
rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 20, makecol(0, 0, 0));
rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 18, makecol(0, 0, 0));
rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 16, makecol(0, 0, 0));
solid_mode();
draw_scroller(&summary_scroller, swap_screen, 1, scrollerY, makecol(150, 150, 150));
if (!draw_scroller(&summary_scroller, swap_screen, 0, scrollerY, makecol(200, 200, 200))) restart_scroller(&summary_scroller);

scrollerY = (int)((scrollerTargetY - scrollerY) * 0.1 + scrollerY);
}



if (falling) falling++;
if (falling > ply[player_id]->level * 5 || falling > 250) {
play_sound(sounds[6], 0, 1);
if (custom.falling)
stop_sample(custom.falling);


ply[player_id]->shake = 24; falling = 0;
}
if (ply[player_id]->shake) { acquire_screen();


blit(swap_screen, screen, 0, new_rand() % 8, 0, 0, swap_screen->w, swap_screen->h); release_screen();

ply[player_id]->shake--; } else


blit_to_screen(swap_screen);


if (isGuest && gotHigh && !is_playing_custom_game && recording) {
poll_control(&ctrl, 0);
if (keypressed()) { if (done == 20) {
int k; k = readkey() & 0xff; k -= 0x20;
if (k == -24) k = (signed char)0xa4;
if (k == 14) k = '.';
if (k == 1) k = '!';
if (k != 0x20) {
for (i = 0; i < len; i++) {
if (letters[i] == k) {


buf[pos * 2] = letters[i];
pos++; alpha_pos = i; skip_keys = 100;
if (pos == 3) done = 19;
}
}
}
}
}
if (skip_keys) { skip_keys--; } else {
if (is_right(&ctrl)) {
alpha_pos++; skip_keys = 8;
if (alpha_pos > len) alpha_pos = 0;
}

if (is_left(&ctrl)) { skip_keys = 8;

alpha_pos--; if (alpha_pos < 0) alpha_pos = len;
}

if (is_fire(&ctrl)) {
if (letters[alpha_pos] == (char)0xa4) { if (pos != 0) {
buf[pos * 2] = '.';
pos--; } skip_keys = 100;

} else if (pos <= 1) { pos++; skip_keys = 100; } else {
if (done == 20) {

pos++; done = 19; } skip_keys = 100;
}
}

if (key[KEY_DEL] || key[KEY_BACKSPACE]) {
buf[pos * 2] = '.'; skip_keys = 7;
if (pos != 0) pos--; } else if (skip_keys) {



skip_keys--; } }
if (!is_any(&ctrl) && !key[KEY_DEL] && !key[KEY_BACKSPACE]) skip_keys = 0;

if (pos <= 2) buf[pos * 2] = letters[alpha_pos];
}

if (done != 20) done--;

poll_control(&ctrl, 0);
if (!isGuest || !gotHigh || is_playing_custom_game) {
if (keypressed() || is_fire(&ctrl)) {
if (done == 20) done = 14;
}
}



if (!key[KEY_TAB] || !key[KEY_LSHIFT])


while (!cycle_count) rest(2);

}





if (!is_playing_custom_game && recording) {
char guestName[4]; guestName[0] = buf[0]; guestName[1] = buf[2]; guestName[2] = buf[4]; guestName[3] = 0;
char postName[32];
strcpy(postName, isGuest ? guestName : profile->handle);




for (i = 0; i < 15; i++) {
if (qualify[i] > 0) {
enter_hisc_table(hisc_tables[i], qualifyValue[i], postName);
sort_hisc_table(hisc_tables[i]);
}
}
}

}






if (recording && !debug && !is_playing_custom_game) {
int f = ply[player_id]->level / 100;

if (f > oldUnlockedFloors && f <= 9) {

fadeOut(16);


blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);


set_trans_blender(0, 0, 0, 158);
drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
solid_mode();


draw_sprite(swap_screen, data[58].dat, 320 - ((BITMAP *)data[58].dat)->w / 2, 20);
textout_centre_ex(swap_screen, data[52].dat, "A new start floor", 320, 0x12c, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "has been unlocked!", 320, 0x15e, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "(Get it in the options menu)", 320, 0x1b8, -1, -1);
play_sound(sounds[2], 0, 0);
fadeIn(swap_screen, 16);
while (key[KEY_ESC] || (key[KEY_ENTER] | key[KEY_SPACE])) { }
while (!key[KEY_ESC] && !key[KEY_ENTER] && !key[KEY_SPACE]) { }
}
}




save_config();


stopGameMusic();
if (checkMusicVoiceID >= 0)
voice_stop(checkMusicVoiceID);


if (recording) { if (!debug) if (!debug) {
in_replay_menu = 1;
play_again = do_replay_menu();
in_replay_menu = 0;
}
}
}


if (recording) play_sound(speaker[2], 0, 0);

stopGameMusic();
if (checkMusicVoiceID >= 0)
voice_stop(checkMusicVoiceID);


clear(screen);

return play_again;
}

void show_credits(void)
{
    double vol = options.msc_volume;
    double vol_step = vol / 150.0f;
    int gc;
    BITMAP *logoBMP;

    clear(swap_screen);
    logoBMP = data[125].dat;

    blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);
    draw_sprite(swap_screen, logoBMP, 320 - logoBMP->w / 2, 10);

    textout_centre_ex(swap_screen, data[50].dat, "Thanks for playing!", 320, 280, -1, -1);
    textout_centre_ex(swap_screen, data[52].dat, "DESIGN & CODING: Johan Peitz", 320, 360, -1, -1);

    textout_centre_ex(swap_screen, data[52].dat, "GRAPHICS: Emanuel Garnheim", 320, 390, -1, -1);

    fadeIn(swap_screen, 16);

    closeButtonClicked = 0;
    cycle_count = 0;
    while (!closeButtonClicked && !key[KEY_ESC] && cycle_count <= 149) {
        gc = cycle_count;

        checkMenuFocus();

        if (bg_menu) adjust_sample(bg_menu, (int)vol, 128, 1000, 1);
        vol -= vol_step;
        while (gc == cycle_count) rest(2);
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
            port_config_set_fullscreen(1);
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
        port_config_set_fullscreen(0);
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
    const int scroller_step = -1;
    static int face;
    static int count;
    int old_msc;
    BITMAP *head_bmp;
    BITMAP *head_shadow;
    BITMAP *head;
    int headX;
    int headY;
    char welcomeMessage[512];
    int mouseInAd;
    int i;

    count++;                                                        /* 5142 */
    if (new_rand() % 198 == 1) face++;                               /* 5143 */
    if (face == 3) face = 0;                                         /* 5144 */

    if (key[KEY_F1]) {                                               /* 5147 */
        take_screenshot(swap_screen);                                /* 5148 */
        while (key[KEY_F1]);                                         /* 5149 */
    }
    testWindowResolution();                                          /* 5152 */

    if (pFLDAd) {                                                    /* 5156 */
        mouseInAd = mouse_x < pFLDAdBitmap->w && mouse_y > SCREEN_H - pFLDAdBitmap->h; /* 5158 */
        if (key[KEY_F5] || (mouseInAd && (mouse_b & 1) && !(lastMouseB & 1))) { /* 5160 */
            while (key[KEY_F5]) rest(2);                             /* 5161 */
            options.full_screen = 0;                                 /* 5165 */
            testWindowResolution();                                  /* 5166 */
            open_web_browser((char *)pFLDAd->pVisitURL);              /* 5169 */
            my_alert("Go online!", "Your browser has been opened.", 0, 1); /* 5170 */
        }
        lastMouseB = mouse_b;                                        /* 5172 */
        if (mouseInAd)                                               /* 5176 */
            select_mouse_cursor(A4_MOUSE_CURSOR_HAND);               /* 5177 */
        else
            select_mouse_cursor(MOUSE_CURSOR_ARROW);                 /* 5179 */
    }

    blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);          /* 5187 */
    draw_sprite(swap_screen, data[71].dat, 330, 280);                /* 5193 */
    draw_sprite(swap_screen, data[125].dat, 0, 0);                   /* 5197 */
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
    rectfill(swap_screen, 0, 462, 639, 479, makecol(0, 0, 0));           /* 5223 */
    rectfill(swap_screen, 0, 463, 639, 479, makecol(0, 0, 0));           /* 5224 */
    rectfill(swap_screen, 0, 464, 639, 479, makecol(0, 0, 0));           /* 5225 */
    solid_mode();                                                     /* 5226 */
    draw_scroller(&greeting_scroller, swap_screen, 1, 462, makecol(150, 150, 150)); /* 5227 */
    if (!draw_scroller(&greeting_scroller, swap_screen, 0, 462,      /* 5228 */
                       makecol(200, 200, 200)))
        restart_scroller(&greeting_scroller);

    textprintf_ex(swap_screen, data[54].dat, 5, 3, makecol(100, 21, 20), -1,
                  "v%s %s", "1.5.1", debug ? " FUN MODE" : "");
    textprintf_ex(swap_screen, data[54].dat, 4, 2, makecol(162, 90, 51), -1,
                  "v%s %s", "1.5.1", debug ? " FUN MODE" : "");
    if (stricmp(profile->handle, "guest")) {
        sprintf(welcomeMessage, "Welcome, %%s! %s",
                get_rank_id(profile) == 0 ? "" : "Your rank is %s.");
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
        play_char.bmp = characters[play_char.value].bmp;              /* 5266 */
        curr_char = play_char.value;                                  /* 5267 */
        strcpy(profile->last_avatar, characters[play_char.value].name); /* 5268 */
    }

    floors.max = profile->best_floor > 999 ? 9 : profile->best_floor / 100; /* 5275 */
    profile->start_floor = floors.value < floors.max ? floors.value : floors.max; /* 5276 */
    menu_params.fo = floors.value * 3 + 17;                           /* 5277 */

    options.flash = get_selection_value(&eyecandy_selection);         /* 5282 */
    options.gravity = get_selection_value(&gravity_selection);        /* 5283 */
    options.floor_size = get_selection_value(&floor_size_selection);  /* 5284 */
    options.start_speed = get_selection_value(&scroll_speed_selection); /* 5285 */
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
    int tick = 0;
    int c;
    int i = strlen(string);


    if (!block)
        return -1;


    blit(bmp, block, pos_x - 1, pos_y - 1, 0, 0, block->w, block->h);

    while (key[KEY_ENTER] || key[KEY_SPACE]);
    clear_keybuf();


    while (!closeButtonClicked) {
        tick++;
        cycle_count = 0;

        checkMenuFocus();


        string[i] = (tick & 8) ? '|' : ' ';
        string[i + 1] = 0;
        vsync();
        blit(block, bmp, 0, 0, pos_x - 1, pos_y - 1, block->w, block->h);
        if (bg_color >= 0) rectfill(bmp, pos_x, pos_y, pos_x + block->w - 1, pos_y + block->h - 3, bg_color);
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


                case KEY_BACKSPACE:

                    if (--i < 0) i = 0;
                    break;

                case KEY_ENTER:
                    string[i] = 0;
                    destroy_bitmap(block);
                    return 0;


                default:
                    if (i < max_chars - 2 && strchr(letters, c)) {

                        if ((c >> 8) != KEY_SPACE || i) {

                            if (w - 9 > text_length(f, string)) {


                                string[i] = c;
                                i++;
                            }
                        }
                    }
            }
        }



        while (!cycle_count) rest(2);
    }


    destroy_bitmap(block);
    return 0;
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
                    int lets_save;
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
                    if (demo) {
                        thisChecksum=calc_replay_checksum(demo);
                        if (thisChecksum==uberChecksum) {
                        strncpy(demo->name,pname,30);
                        strcpy(demo->comment,comment);
                        replace_extension(buffer,fname,"itr",512);
                        sprintf(fpath,"%s%s",replay_directory,buffer);
                        lets_save=1;
                        if (exists(fpath))
                            lets_save=my_alert("The file exists.","Do you want to overwrite it?",1,0);
                        if (!lets_save) {
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
                        else {
                            my_alert("Failed to save replay.",
                                     "Temporary file mismatch.",0,1);
                            status=!isGuest;
                        }
                    }
                    else {
                        my_alert("Failed to save replay.",
                                 "Temporary file not found.",0,1);
                        status=!isGuest;
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
    int done;

    bg=create_bitmap(SCREEN_W,SCREEN_H);
    blit(screen,bg,0,0,0,0,SCREEN_W,SCREEN_H);
    memset(new_name,0,sizeof(new_name));
    done = 0;
    while (!done) {
        int res;
        char buf[129];

        checkMenuFocus();
        blit(bg,swap_screen,0,0,0,0,SCREEN_W,SCREEN_H);
        set_trans_blender(0,0,0,158);
        drawing_mode(DRAW_MODE_TRANS,0,0,0);
        rectfill(swap_screen,0,0,SCREEN_W,SCREEN_H,makecol(0,0,0));
        solid_mode();
        draw_sprite(swap_screen,data[87].dat,100,120);
        textout_ex(swap_screen,data[51].dat,"Welcome to Icy Tower",130,127,-1,-1);
        textout_ex(swap_screen,data[54].dat,"Yo, wazup? In Icy Tower, all your highscores",130,160,0,-1);
        textout_ex(swap_screen,data[54].dat,"and progress will be stored in a personal profile.",130,175,0,-1);
        textout_ex(swap_screen,data[54].dat,"AWESOME!",130,190,0,-1);
        textout_ex(swap_screen,data[54].dat,"Please enter a name for your profile:",130,220,0,-1);
        textout_right_ex(swap_screen,data[54].dat,"...and press enter.",430,260,0,-1);
        rectfill(swap_screen,129,240,430,258,makecol(255,255,255));
        rect(swap_screen,129,240,430,258,makecol(80,80,80));
        blit_to_screen(swap_screen);
        res=get_string(swap_screen,new_name,300,32,data[54].dat,130,240,makecol(0,0,0),-1);
        if (res>=-1) {
            if (res!=-1) {
                if (new_name[0]) {
                    replaceBadCharacters(new_name,'_');
                    profile=create_profile(new_name,0);
                    if (profile) {
                        sprintf(buf,"Welcome %s!",profile->handle);
                        my_alert(buf,"Your profile has been created!",0,1);
                        done = 1;
                    }
                    else
                        my_alert("Ooops!","That profile name is taken.",0,1);
                }
            }
            else {
                my_alert("Oh Well...","You can create a profile later in the OPTIONS menu.",0,1);
                profile=load_profile("guest");
                if (!profile)
                    profile=create_profile("guest",1);
                syncOptionsFromProfile();
                done = 1;
            }
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
    FILE *f;
    int i;
    int ret;
    int must_fade;
    int play_again;

    allegro_init();
    register_png_file_type();
    /* port: no chdir(); relative game paths are resolved against the user
     * and asset roots by the platform layer */
    get_executable_name(full_path, sizeof(full_path));
    replace_filename(working_directory, full_path, "", sizeof(working_directory));
    char logfilename[256] = {0};
    get_logfile_path(logfilename, sizeof(logfilename));
    f = port_fopen(logfilename, "wt");
    if (f) {
        fprintf(f, "Icy Tower v%s - log file\n----------------------------\n", "1.5.1");
        fclose(f);
    }
    for (i = 0; i < argc; i++)
        if (!stricmp(argv[i], "-check"))
            itrcheck = 1;
    log2file("Game started with the following commands:");
    for (i = 0; i < argc; i++)
        log2file("   %s", argv[i]);
    log2file("Working directory is:\n   %s", working_directory);
    if (!init_game(argc, argv)) {
        if (!dropped_file_is_not_a_replay) {
            log2file("* Failed to initialize the game *");
            allegro_message("Failed to initialize the game.");
        }
        log2file("Cleaning up Allegro");
        uninit_game();
        log2file("Done...");
        return 1;
    }
    if (!itrcheck) {
        log2file("Initiating scroller");
        init_scroller(&greeting_scroller, data[54].dat, scroller_greetings, 640, 30, -1);
        menu_params.font = data[51].dat;
        menu_params.bullet = data[72].dat;
        menu_params.pos = 0;
        menu_params.data = data;
        log2file("Initiating menu controls");
        init_control(&menu_params.ctrl);
        if (got_joystick)
            menu_params.ctrl.use_joy = 1;
        log2file("Resetting menu");
        reset_menu(main_menu, &menu_params, 0);
    }
    if (demo) {
        log2file("Running replay.");
        run_demo(NULL);
        if (itrcheck) {
            log2file("Exiting Allegro");
            allegro_exit();
            log2file("\nDone...");
            exit(0);
        }
    }
    load_new_ad_image();
    startMenuMusic();
    log2file("\nMAIN MENU LOOP");
    clear_keybuf();
    if (options.timesStarted == 1 && !stricmp("guest", options.lastProfile)) {
        main_menu_callback();
        draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);
        fadeIn(swap_screen, 16);
        force_create_profile();
        syncOptionsFromProfile();
        must_fade = 0;
    }
    else
        must_fade = 1;
    options.msc_volume = profile->msc_volume;
    options.snd_volume = profile->snd_volume;
    ret = 0;
    while (!closeButtonClicked && ret != 'k') {
        main_menu_callback();
        draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);
        if (must_fade)
            fadeIn(swap_screen, 16);
        else
            blit_to_screen(swap_screen);
        ret = handle_menu(main_menu, &menu_params, &ctrl, swap_screen, main_menu_callback, 355, 285, 0);
        if (ret == 'e' || ret == 0x85) {
            log2file(" new game selected");
            is_playing_custom_game = (ret != 'e');
            fadeOut(16);
            stopMenuMusic();
            play_again = 1;
            while (play_again) {
                if (demo) destroy_replay(demo);
                demo = NULL;
                play_again = 0;
                if (new_game()) {
                    play_again = play();
                    end_game();
                }
                fadeOut(16);
            }
            if (bg_menu)
                play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
            menu_params.pos = 0;
            must_fade = 1;
        }
        else if (ret == 'i') {
            log2file(" high scores selected");
            view_scores(hisc_tables, hisc_names);
            menu_params.pos = 3;
            must_fade = 0;
        }
        else if (ret == 'h') {
            log2file(" instructions selected");
            fadeOut(16);
            show_instructions();
            menu_params.pos = 1;
            must_fade = 1;
        }
        else if (ret == 'z') {
            log2file(" load replay selected");
            if (demo) destroy_replay(demo);
            log2file("   opening %s", replay_directory);
            while ((demo = replay_selector(&ctrl, replay_directory))) {
                fadeOut(16);
                stopMenuMusic();
                run_demo(NULL);
                fadeOut(16);
                main_menu_callback();
                draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);
                fadeIn(swap_screen, 32);
                if (bg_menu && options.msc_volume)
                    play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
            }
            menu_params.pos = 4;
            must_fade = 0;
        }
        rest(2);
    }
    fadeOut(16);
    log2file("\nShowing credits");
    show_credits();
    stopMenuMusic();
    uninit_game();
    log2file("\nDone...");
    return 0;
}


END_OF_MAIN()
