/*
 * Allegro 4.4.1 API subset for the Icy Tower SDL3 port.
 *
 * Only the API surface measured in docs/sdl3/allegro-api-usage.json (plus the
 * few functions that inventory missed: textprintf_*, hline/vline, clear,
 * allegro_init/message, set_window_title, ...) is provided.  Numeric values,
 * the struct fields the game touches and the drawing semantics follow Allegro
 * 4.4.1 (giftware licence, see third_party/licenses/allegro-license.txt).
 *
 * Implementation: compat/allegro4-sdl3/src/.  The platform side (window,
 * events, audio device, clock, presentation) lives in port/ and is reached
 * through small internal interfaces; nothing in this header depends on SDL.
 */
#ifndef A4COMPAT_ALLEGRO_H
#define A4COMPAT_ALLEGRO_H

#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_VERSION          4
#define ALLEGRO_SUB_VERSION      4
#define ALLEGRO_WIP_VERSION      1
#define ALLEGRO_VERSION_STR      "4.4.1 (SDL3 compat subset)"
#define A4COMPAT                 1

#define AL_CONST                 const
#define AL_ID(a,b,c,d)           (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))

#ifndef TRUE
#define TRUE                     -1
#define FALSE                    0
#endif

#undef MIN
#undef MAX
#undef MID
#undef ABS
#undef SGN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   ((x) > (y) ? ((y) > (z) ? (y) : ((x) > (z) ? (z) : (x))) : ((y) > (z) ? ((z) > (x) ? (z) : (x)) : (y)))
#define ABS(x)       (((x) >= 0) ? (x) : (-(x)))
#define SGN(x)       (((x) >= 0) ? 1 : -1)

/* ------------------------------------------------------------------ */
/* system                                                              */
/* ------------------------------------------------------------------ */
extern int *allegro_errno;
int  allegro_init(void);
void allegro_exit(void);
void allegro_message(const char *msg, ...);
void set_window_title(const char *name);
int  set_close_button_callback(void (*proc)(void));
int  alert(const char *s1, const char *s2, const char *s3,
           const char *b1, const char *b2, int c1, int c2);
extern int gui_fg_color, gui_bg_color;
/* The game defines _mangled_main itself; the real main() is in port/. */
#define END_OF_MAIN()
int _mangled_main(int argc, char **argv);

/* ------------------------------------------------------------------ */
/* graphics mode / screen                                              */
/* ------------------------------------------------------------------ */
#define GFX_TEXT                   -1
#define GFX_AUTODETECT             0
#define GFX_AUTODETECT_FULLSCREEN  1
#define GFX_AUTODETECT_WINDOWED    2

typedef struct GFX_DRIVER {
   int id;
   const char *name;
   int w, h;
   int windowed;
} GFX_DRIVER;
extern GFX_DRIVER *gfx_driver;

#define SCREEN_W     (gfx_driver ? gfx_driver->w : 0)
#define SCREEN_H     (gfx_driver ? gfx_driver->h : 0)

#define SWITCH_NONE           0
#define SWITCH_PAUSE          1
#define SWITCH_AMNESIA        2
#define SWITCH_BACKGROUND     3
#define SWITCH_BACKAMNESIA    4
#define SWITCH_IN             0
#define SWITCH_OUT            1

int  set_gfx_mode(int card, int w, int h, int v_w, int v_h);
int  set_display_switch_mode(int mode);
int  set_display_switch_callback(int dir, void (*cb)(void));
void set_color_depth(int depth);
int  get_color_depth(void);
int  desktop_color_depth(void);
void vsync(void);

/* ------------------------------------------------------------------ */
/* bitmaps                                                             */
/* ------------------------------------------------------------------ */
typedef int32_t fixed;

typedef struct BITMAP {
   int w, h;                     /* width and height in pixels */
   int clip;                     /* flag if clipping is turned on */
   int cl, cr, ct, cb;           /* clip rectangle: left, right (exclusive), top, bottom (exclusive) */
   int depth;                    /* 8, 15, 16, 24 or 32 */
   int pitch;                    /* bytes between rows */
   void *dat;                    /* owned pixel memory (NULL for sub-bitmaps) */
   struct BITMAP *parent;        /* non-NULL for sub-bitmaps */
   int x_ofs, y_ofs;             /* offset inside the root bitmap */
   uint32_t serial;              /* unique id, for texture caches */
   uint32_t generation;          /* bumped by every drawing op that targets this bitmap */
   unsigned char *line[];        /* row pointers, as in Allegro */
} BITMAP;

extern BITMAP *screen;

BITMAP *create_bitmap(int width, int height);
BITMAP *create_bitmap_ex(int color_depth, int width, int height);
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height);
void destroy_bitmap(BITMAP *bitmap);
int  bitmap_color_depth(BITMAP *bmp);
int  is_memory_bitmap(BITMAP *bmp);
void acquire_bitmap(BITMAP *bmp);
void release_bitmap(BITMAP *bmp);
void acquire_screen(void);
void release_screen(void);
void set_clip_rect(BITMAP *bitmap, int x1, int y1, int x2, int y2);
void clear_bitmap(BITMAP *bitmap);
void clear_to_color(BITMAP *bitmap, int color);
#define clear(bmp) clear_bitmap(bmp)

/* ------------------------------------------------------------------ */
/* colour                                                              */
/* ------------------------------------------------------------------ */
typedef struct RGB {
   unsigned char r, g, b;        /* 0..63 (VGA 6-bit) */
   unsigned char filler;
} RGB;
#define PAL_SIZE     256
typedef RGB PALETTE[PAL_SIZE];

#define MASK_COLOR_8       0
#define MASK_COLOR_15      0x7C1F
#define MASK_COLOR_16      0xF81F
#define MASK_COLOR_24      0xFF00FF
#define MASK_COLOR_32      0xFF00FF

extern PALETTE desktop_palette;
extern int _rgb_scale_1[2];
extern int _rgb_scale_4[16];
extern int _rgb_scale_5[32];
extern int _rgb_scale_6[64];
int  _color_load_depth(int depth, int hasalpha);

int  makecol(int r, int g, int b);
int  makecol8(int r, int g, int b);
int  makecol15(int r, int g, int b);
int  makecol16(int r, int g, int b);
int  makecol24(int r, int g, int b);
int  makecol32(int r, int g, int b);
int  makeacol32(int r, int g, int b, int a);
int  makecol_depth(int color_depth, int r, int g, int b);
int  getr(int c);
int  getg(int c);
int  getb(int c);
int  getr_depth(int color_depth, int c);
int  getg_depth(int color_depth, int c);
int  getb_depth(int color_depth, int c);
int  getr8(int c);  int getg8(int c);  int getb8(int c);
int  getr15(int c); int getg15(int c); int getb15(int c);
int  getr16(int c); int getg16(int c); int getb16(int c);
int  getr24(int c); int getg24(int c); int getb24(int c);
int  getr32(int c); int getg32(int c); int getb32(int c); int geta32(int c);
int  bitmap_mask_color(BITMAP *bmp);

void set_palette(const RGB *p);
void get_palette(RGB *p);
void select_palette(const RGB *p);
void unselect_palette(void);
void generate_332_palette(RGB *pal);

#define COLORCONV_NONE              0
#define COLORCONV_8_TO_15           1
#define COLORCONV_8_TO_16           2
#define COLORCONV_8_TO_24           4
#define COLORCONV_8_TO_32           8
#define COLORCONV_15_TO_8           0x10
#define COLORCONV_15_TO_16          0x20
#define COLORCONV_15_TO_24          0x40
#define COLORCONV_15_TO_32          0x80
#define COLORCONV_16_TO_8           0x100
#define COLORCONV_16_TO_15          0x200
#define COLORCONV_16_TO_24          0x400
#define COLORCONV_16_TO_32          0x800
#define COLORCONV_24_TO_8           0x1000
#define COLORCONV_24_TO_15          0x2000
#define COLORCONV_24_TO_16          0x4000
#define COLORCONV_24_TO_32          0x8000
#define COLORCONV_32_TO_8           0x10000
#define COLORCONV_32_TO_15          0x20000
#define COLORCONV_32_TO_16          0x40000
#define COLORCONV_32_TO_24          0x80000
#define COLORCONV_32A_TO_8          0x100000
#define COLORCONV_32A_TO_15         0x200000
#define COLORCONV_32A_TO_16         0x400000
#define COLORCONV_32A_TO_24         0x800000
#define COLORCONV_DITHER_PAL        0x1000000
#define COLORCONV_DITHER_HI         0x2000000
#define COLORCONV_KEEP_TRANS        0x4000000
#define COLORCONV_TOTAL             0x00FFFFFF
void set_color_conversion(int mode);
int  get_color_conversion(void);

/* ------------------------------------------------------------------ */
/* drawing                                                             */
/* ------------------------------------------------------------------ */
#define DRAW_MODE_SOLID             0
#define DRAW_MODE_XOR               1
#define DRAW_MODE_COPY_PATTERN      2
#define DRAW_MODE_SOLID_PATTERN     3
#define DRAW_MODE_MASKED_PATTERN    4
#define DRAW_MODE_TRANS             5

void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor);
void solid_mode(void);
void set_trans_blender(int r, int g, int b, int a);
void set_alpha_blender(void);

void putpixel(BITMAP *bmp, int x, int y, int color);
int  getpixel(BITMAP *bmp, int x, int y);
void hline(BITMAP *bmp, int x1, int y, int x2, int color);
void vline(BITMAP *bmp, int x, int y1, int y2, int color);
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);

void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y,
          int dest_x, int dest_y, int width, int height);
void masked_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y,
                 int dest_x, int dest_y, int width, int height);
void stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y,
                  int source_width, int source_height, int dest_x, int dest_y,
                  int dest_width, int dest_height);
void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y);
void draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, int y);
void draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, int y);
void draw_trans_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y);
void stretch_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int w, int h);
void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle);
void rotate_scaled_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y,
                          fixed angle, fixed scale);

/* ------------------------------------------------------------------ */
/* fonts and text                                                      */
/* ------------------------------------------------------------------ */
typedef struct FONT FONT;
extern FONT *font;
void textout_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int bg);
void textout_centre_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int bg);
void textout_right_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int bg);
void textprintf_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int bg, const char *format, ...);
void textprintf_centre_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int bg, const char *format, ...);
void textprintf_right_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int bg, const char *format, ...);
int  text_length(const FONT *f, const char *str);
int  text_height(const FONT *f);
void destroy_font(FONT *f);

/* ------------------------------------------------------------------ */
/* fixed point (Allegro C semantics, see src/a4_fixed.c)               */
/* ------------------------------------------------------------------ */
extern const fixed _cos_tbl[512];
extern const fixed _tan_tbl[256];
extern const fixed _acos_tbl[513];
fixed ftofix(double x);
double fixtof(fixed x);
fixed fixmul(fixed x, fixed y);
fixed fixdiv(fixed x, fixed y);
static inline fixed itofix(int x) { return (fixed)((uint32_t)x << 16); }
static inline int fixfloor(fixed x) { return (x >= 0) ? (x >> 16) : ~((~x) >> 16); }
static inline int fixtoi(fixed x) { return fixfloor(x) + ((x & 0x8000) >> 15); }
static inline fixed fixcos(fixed x) { return _cos_tbl[((x + 0x4000) >> 15) & 0x1FF]; }
static inline fixed fixsin(fixed x) { return _cos_tbl[((x - 0x400000 + 0x4000) >> 15) & 0x1FF]; }

/* ------------------------------------------------------------------ */
/* files, packfiles, config                                            */
/* ------------------------------------------------------------------ */
#define FA_NONE           0
#define FA_RDONLY         1
#define FA_HIDDEN         2
#define FA_SYSTEM         4
#define FA_LABEL          8
#define FA_DIREC          16
#define FA_ARCH           32
#define FA_ALL            (~FA_NONE)

typedef struct PACKFILE PACKFILE;
#define F_READ            "r"
#define F_WRITE           "w"
#define F_READ_PACKED     "rp"
#define F_WRITE_PACKED    "wp"
#define F_WRITE_NOPACK    "w!"

void packfile_password(const char *password);
PACKFILE *pack_fopen(const char *filename, const char *mode);
int  pack_fclose(PACKFILE *f);
long pack_fread(void *p, long n, PACKFILE *f);
long pack_fwrite(const void *p, long n, PACKFILE *f);
int  pack_getc(PACKFILE *f);
int  pack_putc(int c, PACKFILE *f);
int  pack_feof(PACKFILE *f);
int  pack_ferror(PACKFILE *f);
int  pack_fseek(PACKFILE *f, int offset);
int  pack_igetw(PACKFILE *f);
long pack_igetl(PACKFILE *f);
int  pack_mgetw(PACKFILE *f);
long pack_mgetl(PACKFILE *f);
int  pack_iputw(int w, PACKFILE *f);
long pack_iputl(long l, PACKFILE *f);
int  pack_mputw(int w, PACKFILE *f);
long pack_mputl(long l, PACKFILE *f);
/* compat extension: read-only packfile over a memory block (not owned) */
PACKFILE *a4_pack_fopen_memory(const void *data, long size);

int  exists(const char *filename);
int  file_exists(const char *filename, int attrib, int *aret);
int64_t file_size_ex(const char *filename);
int  delete_file(const char *filename);
char *get_filename(const char *path);
char *get_extension(const char *filename);
char *replace_extension(char *dest, const char *filename, const char *ext, int size);
char *replace_filename(char *dest, const char *path, const char *filename, int size);
void get_executable_name(char *output, int size);
int  for_each_file_ex(const char *name, int in_attrib, int out_attrib,
                      int (*callback)(const char *filename, int attrib, void *param),
                      void *param);

void set_config_file(const char *filename);
const char *get_config_string(const char *section, const char *name, const char *def);
int  get_config_int(const char *section, const char *name, int def);

/* ------------------------------------------------------------------ */
/* datafiles and image files                                           */
/* ------------------------------------------------------------------ */
#define DAT_ID(a,b,c,d)    AL_ID(a,b,c,d)
#define DAT_MAGIC          DAT_ID('A','L','L','.')
#define DAT_FILE           DAT_ID('F','I','L','E')
#define DAT_DATA           DAT_ID('D','A','T','A')
#define DAT_FONT           DAT_ID('F','O','N','T')
#define DAT_SAMPLE         DAT_ID('S','A','M','P')
#define DAT_MIDI           DAT_ID('M','I','D','I')
#define DAT_PATCH          DAT_ID('P','A','T',' ')
#define DAT_FLI            DAT_ID('F','L','I','C')
#define DAT_BITMAP         DAT_ID('B','M','P',' ')
#define DAT_RLE_SPRITE     DAT_ID('R','L','E',' ')
#define DAT_C_SPRITE       DAT_ID('C','M','P',' ')
#define DAT_XC_SPRITE      DAT_ID('X','C','M','P')
#define DAT_PALETTE        DAT_ID('P','A','L',' ')
#define DAT_PROPERTY       DAT_ID('p','r','o','p')
#define DAT_NAME           DAT_ID('N','A','M','E')
#define DAT_END            -1

typedef struct DATAFILE_PROPERTY {
   char *dat;
   int type;
} DATAFILE_PROPERTY;

typedef struct DATAFILE {
   void *dat;
   int type;
   long size;
   DATAFILE_PROPERTY *prop;
} DATAFILE;

DATAFILE *load_datafile(const char *filename);
DATAFILE *load_datafile_callback(const char *filename, void (*callback)(DATAFILE *));
void unload_datafile(DATAFILE *dat);
void register_datafile_object(int id, void *(*load)(PACKFILE *f, long size),
                              void (*destroy)(void *data));
BITMAP *_fixup_loaded_bitmap(BITMAP *bmp, RGB *pal, int bpp);

BITMAP *load_bitmap(const char *filename, RGB *pal);
int  save_bitmap(const char *filename, BITMAP *bmp, const RGB *pal);
void register_bitmap_file_type(const char *ext,
                               BITMAP *(*load)(const char *filename, RGB *pal),
                               int (*save)(const char *filename, BITMAP *bmp, const RGB *pal));

/* ------------------------------------------------------------------ */
/* keyboard (Allegro 4.4.1 scancodes; stored in profiles, so fixed)    */
/* ------------------------------------------------------------------ */
#define KEY_A            1
#define KEY_B            2
#define KEY_C            3
#define KEY_D            4
#define KEY_E            5
#define KEY_F            6
#define KEY_G            7
#define KEY_H            8
#define KEY_I            9
#define KEY_J            10
#define KEY_K            11
#define KEY_L            12
#define KEY_M            13
#define KEY_N            14
#define KEY_O            15
#define KEY_P            16
#define KEY_Q            17
#define KEY_R            18
#define KEY_S            19
#define KEY_T            20
#define KEY_U            21
#define KEY_V            22
#define KEY_W            23
#define KEY_X            24
#define KEY_Y            25
#define KEY_Z            26
#define KEY_0            27
#define KEY_1            28
#define KEY_2            29
#define KEY_3            30
#define KEY_4            31
#define KEY_5            32
#define KEY_6            33
#define KEY_7            34
#define KEY_8            35
#define KEY_9            36
#define KEY_0_PAD        37
#define KEY_1_PAD        38
#define KEY_2_PAD        39
#define KEY_3_PAD        40
#define KEY_4_PAD        41
#define KEY_5_PAD        42
#define KEY_6_PAD        43
#define KEY_7_PAD        44
#define KEY_8_PAD        45
#define KEY_9_PAD        46
#define KEY_F1           47
#define KEY_F2           48
#define KEY_F3           49
#define KEY_F4           50
#define KEY_F5           51
#define KEY_F6           52
#define KEY_F7           53
#define KEY_F8           54
#define KEY_F9           55
#define KEY_F10          56
#define KEY_F11          57
#define KEY_F12          58
#define KEY_ESC          59
#define KEY_TILDE        60
#define KEY_MINUS        61
#define KEY_EQUALS       62
#define KEY_BACKSPACE    63
#define KEY_TAB          64
#define KEY_OPENBRACE    65
#define KEY_CLOSEBRACE   66
#define KEY_ENTER        67
#define KEY_COLON        68
#define KEY_QUOTE        69
#define KEY_BACKSLASH    70
#define KEY_BACKSLASH2   71
#define KEY_COMMA        72
#define KEY_STOP         73
#define KEY_SLASH        74
#define KEY_SPACE        75
#define KEY_INSERT       76
#define KEY_DEL          77
#define KEY_HOME         78
#define KEY_END          79
#define KEY_PGUP         80
#define KEY_PGDN         81
#define KEY_LEFT         82
#define KEY_RIGHT        83
#define KEY_UP           84
#define KEY_DOWN         85
#define KEY_SLASH_PAD    86
#define KEY_ASTERISK     87
#define KEY_MINUS_PAD    88
#define KEY_PLUS_PAD     89
#define KEY_DEL_PAD      90
#define KEY_ENTER_PAD    91
#define KEY_PRTSCR       92
#define KEY_PAUSE        93
#define KEY_ABNT_C1      94
#define KEY_YEN          95
#define KEY_KANA         96
#define KEY_CONVERT      97
#define KEY_NOCONVERT    98
#define KEY_AT           99
#define KEY_CIRCUMFLEX   100
#define KEY_COLON2       101
#define KEY_KANJI        102
#define KEY_EQUALS_PAD   103
#define KEY_BACKQUOTE    104
#define KEY_SEMICOLON    105
#define KEY_COMMAND      106
#define KEY_UNKNOWN1     107
#define KEY_UNKNOWN2     108
#define KEY_UNKNOWN3     109
#define KEY_UNKNOWN4     110
#define KEY_UNKNOWN5     111
#define KEY_UNKNOWN6     112
#define KEY_UNKNOWN7     113
#define KEY_UNKNOWN8     114
#define KEY_MODIFIERS    115
#define KEY_LSHIFT       115
#define KEY_RSHIFT       116
#define KEY_LCONTROL     117
#define KEY_RCONTROL     118
#define KEY_ALT          119
#define KEY_ALTGR        120
#define KEY_LWIN         121
#define KEY_RWIN         122
#define KEY_MENU         123
#define KEY_SCRLOCK      124
#define KEY_NUMLOCK      125
#define KEY_CAPSLOCK     126
#define KEY_MAX          127

#define KB_SHIFT_FLAG         0x0001
#define KB_CTRL_FLAG          0x0002
#define KB_ALT_FLAG           0x0004

int  install_keyboard(void);
void remove_keyboard(void);
/* Allegro's key[] is written asynchronously by its keyboard thread.  SDL
 * delivers input on the main thread, so every read of key[] pumps pending
 * events first (rate limited); busy loops such as while(key[KEY_F1]); keep
 * working.  Writes (key[k] = 0) go to the same array. */
volatile char *a4_key_state(void);
#define key (a4_key_state())
extern volatile int key_shifts;
int  keypressed(void);
int  readkey(void);
void clear_keybuf(void);
void simulate_keypress(int keycode);

/* ------------------------------------------------------------------ */
/* mouse                                                               */
/* ------------------------------------------------------------------ */
#define MOUSE_CURSOR_NONE       0
#define MOUSE_CURSOR_ALLEGRO    1
#define MOUSE_CURSOR_ARROW      2
#define MOUSE_CURSOR_BUSY       3
#define MOUSE_CURSOR_QUESTION   4
#define MOUSE_CURSOR_EDIT       5
#define A4_MOUSE_CURSOR_HAND    100   /* compat extension; the game used a raw Win32 cursor */
extern volatile int mouse_x, mouse_y, mouse_z, mouse_b;
int  install_mouse(void);
void show_mouse(BITMAP *bmp);
void select_mouse_cursor(int cursor);
void enable_hardware_cursor(void);

/* ------------------------------------------------------------------ */
/* joystick                                                            */
/* ------------------------------------------------------------------ */
#define JOY_TYPE_AUTODETECT      -1
#define JOY_TYPE_NONE            0
#define MAX_JOYSTICKS            8
#define MAX_JOYSTICK_AXIS        3
#define MAX_JOYSTICK_STICKS      5
#define MAX_JOYSTICK_BUTTONS     32

typedef struct JOYSTICK_AXIS_INFO {
   int pos;
   int d1, d2;
   const char *name;
} JOYSTICK_AXIS_INFO;

typedef struct JOYSTICK_STICK_INFO {
   int flags;
   int num_axis;
   JOYSTICK_AXIS_INFO axis[MAX_JOYSTICK_AXIS];
   const char *name;
} JOYSTICK_STICK_INFO;

typedef struct JOYSTICK_BUTTON_INFO {
   int b;
   const char *name;
} JOYSTICK_BUTTON_INFO;

typedef struct JOYSTICK_INFO {
   int flags;
   int num_sticks;
   int num_buttons;
   JOYSTICK_STICK_INFO stick[MAX_JOYSTICK_STICKS];
   JOYSTICK_BUTTON_INFO button[MAX_JOYSTICK_BUTTONS];
} JOYSTICK_INFO;

extern JOYSTICK_INFO joy[MAX_JOYSTICKS];
extern int num_joysticks;
int  install_joystick(int type);
int  poll_joystick(void);

/* ------------------------------------------------------------------ */
/* timers                                                              */
/* ------------------------------------------------------------------ */
int  install_timer(void);
int  install_int(void (*proc)(void), long speed_ms);
void remove_int(void (*proc)(void));
void rest(unsigned int time_ms);

/* ------------------------------------------------------------------ */
/* sound                                                               */
/* ------------------------------------------------------------------ */
#define DIGI_AUTODETECT       -1
#define DIGI_NONE             0
#define MIDI_AUTODETECT       -1
#define MIDI_NONE             0

typedef struct SAMPLE {
   int bits;                     /* 8 or 16; data is unsigned, as in Allegro */
   int stereo;                   /* interleaved stereo flag */
   int freq;                     /* sample frequency */
   int priority;                 /* 0-255 */
   unsigned long len;            /* length in sample frames */
   unsigned long loop_start;
   unsigned long loop_end;
   unsigned long param;          /* internal */
   void *data;
} SAMPLE;

typedef struct MIDI MIDI;

int  install_sound(int digi, int midi, const char *cfg_path);
void remove_sound(void);
void set_volume(int digi_volume, int midi_volume);
SAMPLE *create_sample(int bits, int stereo, int freq, int len);
void destroy_sample(SAMPLE *spl);
SAMPLE *load_sample(const char *filename);
SAMPLE *load_wav(const char *filename);
SAMPLE *load_wav_pf(PACKFILE *f);
int  play_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop);
void adjust_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop);
void stop_sample(const SAMPLE *spl);
void voice_stop(int voice);
int  voice_get_position(int voice);
MIDI *load_midi(const char *filename);
void destroy_midi(MIDI *midi);
int  play_midi(MIDI *midi, int loop);
void stop_midi(void);

#ifdef __cplusplus
}
#endif

#endif /* A4COMPAT_ALLEGRO_H */
