#include "recovered/Tprofile.h"
extern int my_alert(char*, char*, int, int);
/* Historical CU: menu.c; current status: src/recovery.json.
 * Historical function: update_game_menu @ 0x00417adc, 583 bytes
 */
#include <allegro.h>
#include <string.h>
#include "control.h"
#include "timer.h"
#include "recovered/Tmenu_slider.h"

#include "recovered/Tmenu_selection.h"

#include "recovered/Tmenu_floor_selection.h"

#include "recovered/Tmenu_char_selection.h"

#include "recovered/Tmenu.h"

#include "recovered/Tmenu_params.h"

int stepIn;

extern void key_to_str(int key, char *dest);
extern void draw_menu(BITMAP *bmp, Tmenu *m, Tmenu_params *mp, int x, int y,
                      int step_in);
extern void play_menu_move(void);
extern void play_menu_select(void);
extern void checkMenuFocus(void);
extern void blit_to_screen(BITMAP *bmp);
extern void line_alert(char *text);
extern int view_profile(Tprofile*);
extern void change_profile(void);
extern void *profile;
extern int closeButtonClicked;
extern int stepIn;
extern volatile char key[];
/* Icy Tower was built against an older Allegro scancode layout. */
#ifdef KEY_F1
#undef KEY_F1
#endif
#define KEY_F1 59

void build_menu_string(Tmenu *m, char *dest);

/* Forward declarations; definitions follow in their original source order. */
void build_menu_string(Tmenu *m, char *dest);
void draw_menu(BITMAP *bmp, Tmenu *m, Tmenu_params *mp, int cx, int y, int dx);
void reset_menu(Tmenu *m, Tmenu_params *mp, int sel_pos);
int update_game_menu(BITMAP*, Tmenu*, Tmenu_params*, Tcontrol*, int, int, void**);
int handle_menu(Tmenu *menu, Tmenu_params *mp, Tcontrol *ctrl, BITMAP *bmp, void (*callback)(void), int x, int y, int dx);
void key_to_str(int k, char *dest);
int get_slider_value(Tmenu_slider *s);
int set_slider_value(Tmenu_slider *s, int v);
int get_selection_value(Tmenu_selection *s);
int set_selection_value(Tmenu_selection *s, int v);

void build_menu_string(Tmenu *m, char *dest)
{
    int v;
    int t;
    int i;

    if (m->flags & 2) {
        Tmenu_slider *s;
        s = (Tmenu_slider *)m->data;
        v = (s->value - s->min) / s->step;
        t = (s->max - s->min) / s->step;
        sprintf(dest, "%s: ", m->caption);
        for (i = 0; i < v; i++)
            strcat(dest, "}");
        for (; i < t; i++)
            strcat(dest, "{");
    } else if (m->flags & 8) {
        Tmenu_selection *s;
        s = (Tmenu_selection *)m->data;
        sprintf(dest, "%s: %s", m->caption, s->caption[s->value]);
    } else if (m->flags & 4) {
        int *v;
        v = (int *)m->data;
        sprintf(dest, "%s: %s", m->caption, *v ? "YES" : "NO");
    } else if (m->flags & 64) {
        char str[32];
        key_to_str(*(int *)m->data, str);
        sprintf(dest, "%s: (%s)", m->caption, str);
    } else if (m->flags & 16) {
        sprintf(dest, "%s:", m->caption);
    } else if (m->flags & 32) {
        sprintf(dest, "%s:", m->caption);
    } else
        strcpy(dest, m->caption);
}

/* The original keeps the second slider bitmap in a small menu-specific
 * pointer table; it is not a Tmenu_slider despite sharing the menu data
 * field. */
void draw_menu(BITMAP *bmp, Tmenu *m, Tmenu_params *mp, int cx, int y, int dx)
{
    int pos;
    char str[256];
    int h;

    stepIn = dx;
    h = mp->font_height - 12;
    pos = -1;
    do {
        int x;

        pos++;
        x = cx + dx * pos;
        build_menu_string(m, str);
        if (m->flags & 64) {
            char key_str[32];

            key_to_str(*(int *)m->data, key_str);
            textprintf_ex(bmp, (FONT *)mp->font, x, y, -1, -1, "%s:", m->caption);
            textprintf_ex(bmp, (FONT *)mp->font, x + 101, y, -1, -1, "%s", key_str);
        }
        if (m->flags & 1)
            draw_sprite(bmp, (BITMAP *)mp->bullet,
                        x - 3 - ((BITMAP *)mp->bullet)->w, y - 3);
        textout_ex(bmp, (FONT *)mp->font, str, x, y, -1, -1);
        if (m->flags & 16) {
            DATAFILE *assets;

            assets = (DATAFILE *)mp->data + mp->fo;
            draw_sprite(bmp, (BITMAP *)assets[0].dat, x + 215, y + 10);
            draw_sprite(bmp, (BITMAP *)assets[1].dat, x + 236, y + 10);
            draw_sprite(bmp, (BITMAP *)assets[2].dat, x + 252, y + 10);
        }
        if (m->flags & 32) {
            BITMAP *b;

            b = ((BITMAP **)m->data)[2];
            draw_sprite(bmp, b, x + 244 - b->w / 2, y + h - b->h + 10);
        }
        m++;
        y += h;
    } while ((signed char)m[-1].flags >= 0);
}

void reset_menu(Tmenu *m, Tmenu_params *mp, int sel_pos)
{
    int i;
    int flags;

    i = 0;
    do {
        flags = m[i].flags;
        flags &= ~1;
        m[i].flags = flags;
        i++;
    } while ((signed char)flags >= 0);
    m[sel_pos].flags |= 1;
    mp->font_height = text_height(mp->font);
}

int update_game_menu(BITMAP *bmp, Tmenu *m, Tmenu_params *mp, Tcontrol *ctrl,
                     int x, int y, void **data)
{
    int num_posts;
    int old_pos;
    int pos;
    int return_value;

    pos = 0;
    num_posts = -1;
    do {
        num_posts++;
        if (m[num_posts].flags & 1)
            pos = num_posts;
    } while ((signed char)m[num_posts].flags >= 0);
    old_pos = pos;
    draw_menu(bmp, m, mp, x, y, stepIn);
    if (ctrl) {
        if (is_up(ctrl) || is_up(&mp->ctrl)) {
            pos--;
            if (pos < 0)
                pos = num_posts;
        }
        if (is_down(ctrl) || is_down(&mp->ctrl)) {
            pos++;
            if (pos > num_posts)
                pos = 0;
        }
        if (key[KEY_F1] && pos != num_posts)
            pos = num_posts;
    }
    return_value = 0;
    if (old_pos != pos) {
        m[old_pos].flags &= ~1;
        m[pos].flags |= 1;
        play_menu_move();
    }
    if (ctrl) {
        if (is_fire(ctrl) || is_enter(&mp->ctrl) ||
            is_fire(&mp->ctrl))
            return_value = m[pos].return_select;
        if (is_left(ctrl) || is_left(&mp->ctrl))
            return_value = m[pos].return_left;
        if (is_right(ctrl) || is_right(&mp->ctrl))
            return_value = m[pos].return_right;
    }
    *data = (int)m[pos].data;
    mp->pos = pos;
    return return_value;
}

int handle_menu(Tmenu *menu, Tmenu_params *mp, Tcontrol *ctrl, BITMAP *bmp,
                void (*callback)(void), int x, int y, int dx)
{
    int menu_return;
    int handle_keys;
    int done;
    void *data;
    int key_counter;

    stepIn = dx;
    data = 0;
    reset_menu(menu, mp, mp->pos);
    handle_keys = 0;
    key_counter = 0;
    menu_return = 0;
    while (!closeButtonClicked) {
        cycle_count = 0;
        checkMenuFocus();
        if (callback)
            callback();
        else
            clear(bmp);
        menu_return = update_game_menu(bmp, menu, mp, handle_keys ? ctrl : 0,
                                       x, y, &data);
        blit_to_screen(bmp);

        if (is_any(ctrl) || is_any(&mp->ctrl) || key[KEY_ESC]) {
            if (key_counter)
                key_counter--;
            else
                key_counter = 39;
            handle_keys = key_counter ? 0 : -1;
        } else {
            key_counter = 0;
            handle_keys = -1;
        }
        poll_control(ctrl, 1);
        poll_control(&mp->ctrl, 0);

        if (menu_return)
            play_menu_select();
        switch (menu_return) {
        case 0:
            break;
        case 101:
            return menu_return;
        case 104:
            return menu_return;
        case 105:
            return menu_return;
        case 107:
            return menu_return;
        case 108:
            return menu_return;
        case 122:
            return menu_return;
        case 123:
            return menu_return;
        case 124:
            return menu_return;
        case 133:
            return menu_return;
        case 119:
        case 121: {
            Tmenu_char_selection *sel = (Tmenu_char_selection *)data;

            sel->value++;
            if (sel->value > sel->max)
                sel->value = sel->max;
            break;
        }
        case 113: {
            int *box = (int *)data;

            *box = *box < 1 ? -1 : 0;
            break;
        }
        case 111:
        case 118:
        case 120: {
            Tmenu_char_selection *sel = (Tmenu_char_selection *)data;

            sel->value--;
            if (sel->value < 1)
                sel->value = 0;
            break;
        }
        case 103: {
            int sub_ret;

            mp->pos = 0;
            sub_ret = handle_menu((Tmenu *)data, mp, ctrl, bmp, callback,
                                  x, y, dx);
            if (sub_ret && sub_ret != 108)
                return sub_ret;
            break;
        }
        case 114: {
            int kp;
            int k;
            char txt[256];

            sprintf(txt, "press key for %s", menu[mp->pos].caption);
            line_alert(txt);
            for (k = 0; k < 128; k++)
                key[k] = 0;
            for (;;) {
                k = 0;
                for (kp = 0; kp < 128; kp++)
                    if (key[kp])
                        k = kp;
                if (k == KEY_ESC)
                    k = *(int *)data;
                rest(2);
                if (k)
                    break;
            }
            for (kp = 0; kp < 128; kp++)
                key[kp] = 0;
            *(int *)data = k;
            play_menu_select();
            break;
        }
        case 112: {
            Tmenu_selection *sel = (Tmenu_selection *)data;

            sel->value++;
            if (sel->value > sel->size - 1)
                sel->value = sel->size - 1;
            break;
        }
        case 109: {
            Tmenu_slider *sld = (Tmenu_slider *)data;

            sld->value += sld->step;
            if (sld->value > sld->max)
                sld->value = sld->max;
            break;
        }
        case 110: {
            Tmenu_slider *sld = (Tmenu_slider *)data;

            sld->value -= sld->step;
            if (sld->value < sld->min)
                sld->value = sld->min;
            break;
        }
        case 131:
            view_profile(profile);
            break;
        case 132:
            change_profile();
            break;
        default: {
            char buf[128];

            sprintf(buf, "unknown return value: %d", menu_return);
            my_alert("handle_menu", buf, 0, 0);
            break;
        }
        }
        if (!(key[KEY_TAB] && key[KEY_LSHIFT]))
            while (!cycle_count)
                rest(2);
    }
    return menu_return;
}

void key_to_str(int k, char *dest)
{
    if (k == 1)
        strcpy(dest, "A");
    else if (k == 2)
        strcpy(dest, "B");
    else if (k == 3)
        strcpy(dest, "C");
    else if (k == 4)
        strcpy(dest, "D");
    else if (k == 5)
        strcpy(dest, "E");
    else if (k == 6)
        strcpy(dest, "F");
    else if (k == 7)
        strcpy(dest, "G");
    else if (k == 8)
        strcpy(dest, "H");
    else if (k == 9)
        strcpy(dest, "I");
    else if (k == 10)
        strcpy(dest, "J");
    else if (k == 11)
        strcpy(dest, "K");
    else if (k == 12)
        strcpy(dest, "L");
    else if (k == 13)
        strcpy(dest, "M");
    else if (k == 14)
        strcpy(dest, "N");
    else if (k == 15)
        strcpy(dest, "O");
    else if (k == 16)
        strcpy(dest, "P");
    else if (k == 17)
        strcpy(dest, "Q");
    else if (k == 18)
        strcpy(dest, "r");
    else if (k == 19)
        strcpy(dest, "S");
    else if (k == 20)
        strcpy(dest, "T");
    else if (k == 21)
        strcpy(dest, "U");
    else if (k == 22)
        strcpy(dest, "V");
    else if (k == 23)
        strcpy(dest, "W");
    else if (k == 24)
        strcpy(dest, "X");
    else if (k == 25)
        strcpy(dest, "Y");
    else if (k == 26)
        strcpy(dest, "Z");
    else if (k == 27)
        strcpy(dest, "0");
    else if (k == 28)
        strcpy(dest, "1");
    else if (k == 29)
        strcpy(dest, "2");
    else if (k == 30)
        strcpy(dest, "3");
    else if (k == 31)
        strcpy(dest, "4");
    else if (k == 32)
        strcpy(dest, "5");
    else if (k == 33)
        strcpy(dest, "6");
    else if (k == 34)
        strcpy(dest, "7");
    else if (k == 35)
        strcpy(dest, "8");
    else if (k == 36)
        strcpy(dest, "9");
    else if (k == 37)
        strcpy(dest, "0 (Pad)");
    else if (k == 38)
        strcpy(dest, "1 (Pad)");
    else if (k == 39)
        strcpy(dest, "2 (Pad)");
    else if (k == 40)
        strcpy(dest, "3 (Pad)");
    else if (k == 41)
        strcpy(dest, "4 (Pad)");
    else if (k == 42)
        strcpy(dest, "5 (Pad)");
    else if (k == 43)
        strcpy(dest, "6 (Pad)");
    else if (k == 44)
        strcpy(dest, "7 (Pad)");
    else if (k == 45)
        strcpy(dest, "8 (Pad)");
    else if (k == 46)
        strcpy(dest, "9 (Pad)");
    else if (k == 47)
        strcpy(dest, "F1");
    else if (k == 48)
        strcpy(dest, "F2");
    else if (k == 49)
        strcpy(dest, "F3");
    else if (k == 50)
        strcpy(dest, "F4");
    else if (k == 51)
        strcpy(dest, "F5");
    else if (k == 52)
        strcpy(dest, "F6");
    else if (k == 53)
        strcpy(dest, "F7");
    else if (k == 54)
        strcpy(dest, "F8");
    else if (k == 55)
        strcpy(dest, "F9");
    else if (k == 56)
        strcpy(dest, "F10");
    else if (k == 57)
        strcpy(dest, "F11");
    else if (k == 58)
        strcpy(dest, "F12");
    else if (k == 59)
        strcpy(dest, "ESC");
    else if (k == 60)
        strcpy(dest, "TILDE");
    else if (k == 61)
        strcpy(dest, "-");
    else if (k == 62)
        strcpy(dest, "=");
    else if (k == 63)
        strcpy(dest, "Backspace");
    else if (k == 64)
        strcpy(dest, "Tab");
    else if (k == 65)
        strcpy(dest, "{");
    else if (k == 66)
        strcpy(dest, "}");
    else if (k == 67)
        strcpy(dest, "Enter");
    else if (k == 68)
        strcpy(dest, ":");
    else if (k == 105)
        strcpy(dest, ";");
    else if (k == 69)
        strcpy(dest, "'");
    else if (k == 70)
        strcpy(dest, "\\");
    else if (k == 71)
        strcpy(dest, "\\");
    else if (k == 72)
        strcpy(dest, ",");
    else if (k == 73)
        strcpy(dest, ".");
    else if (k == 74)
        strcpy(dest, "/");
    else if (k == 75)
        strcpy(dest, "Space");
    else if (k == 76)
        strcpy(dest, "Insert");
    else if (k == 77)
        strcpy(dest, "Delete");
    else if (k == 78)
        strcpy(dest, "Home");
    else if (k == 79)
        strcpy(dest, "End");
    else if (k == 80)
        strcpy(dest, "Pg Up");
    else if (k == 81)
        strcpy(dest, "Pg Down");
    else if (k == 82)
        strcpy(dest, "Left");
    else if (k == 83)
        strcpy(dest, "Right");
    else if (k == 84)
        strcpy(dest, "Up");
    else if (k == 85)
        strcpy(dest, "Down");
    else if (k == 86)
        strcpy(dest, "/ (Pad)");
    else if (k == 87)
        strcpy(dest, "*");
    else if (k == 88)
        strcpy(dest, "- (Pad)");
    else if (k == 89)
        strcpy(dest, "+ (Pad)");
    else if (k == 90)
        strcpy(dest, "Del (Pad)");
    else if (k == 91)
        strcpy(dest, "Enter (Pad)");
    else if (k == 92)
        strcpy(dest, "Print Screen");
    else if (k == 93)
        strcpy(dest, "Pause");
    else if (k == 95)
        strcpy(dest, "Yen");
    else if (k == 96)
        strcpy(dest, "Kana");
    else if (k == 115)
        strcpy(dest, "L Shift");
    else if (k == 116)
        strcpy(dest, "R Shift");
    else if (k == 117)
        strcpy(dest, "L Ctrl");
    else if (k == 118)
        strcpy(dest, "R Ctrl");
    else if (k == 119)
        strcpy(dest, "Alt");
    else if (k == 120)
        strcpy(dest, "Alt Gr");
    else if (k == 121)
        strcpy(dest, "Left Win");
    else if (k == 122)
        strcpy(dest, "Right Win");
    else if (k == 123)
        strcpy(dest, "Menu");
    else if (k == 124)
        strcpy(dest, "Scroll Lock");
    else if (k == 125)
        strcpy(dest, "Num Lock");
    else if (k == 126)
        strcpy(dest, "Caps Lock");
    else
        strcpy(dest, "undefined");
}

int get_slider_value(Tmenu_slider *s) { return s->value; }

int set_slider_value(Tmenu_slider *s, int v)
{
    if (v < s->min || v > s->max) return 0;
    s->value = v;
    return -1;
}

int get_selection_value(Tmenu_selection *s) { return s->value; }

int set_selection_value(Tmenu_selection *s, int v)
{
    if (v < 0 || v >= s->size) return 0;
    s->value = v;
    return -1;
}
