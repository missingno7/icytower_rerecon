/* F:\projects\icytower\trunk\source\control.c -- complete 15-function CU.
 * Existing recovered bodies reunited at the historical CU boundary.
 * load_control/save_control transfer one sizeof(Tcontrol) record with
 * fread/fwrite, including historical natural tail padding.
 * Proof and experiments: docs/control-experiment.md.
 */
#include <allegro.h>
#include "control.h"

#define CTRL_LEFT  0x01
#define CTRL_RIGHT 0x02
#define CTRL_UP    0x04
#define CTRL_DOWN  0x08
#define CTRL_FIRE  0x10
#define CTRL_ENTER 0x20
#define CTRL_PAUSE 0x40

Tgamepad gamepad;

void init_control(Tcontrol *c)
{
    set_control(c, 0x54, 0x55, 0x52, 0x53, 0x4b);
    c->key_enter = 0x43;
    c->key_pause = 0x10;
    c->flags = 0;
    c->use_joy = 0;
}

void set_control(Tcontrol *c, int up, int down, int left, int right, int fire)
{
    c->key_up = up;
    c->key_down = down;
    c->key_left = left;
    c->key_right = right;
    c->key_fire = fire;
}

Tgamepad *get_gamepad(void) { return &gamepad; }

void poll_control(Tcontrol *c, int joystick_only)
{
    int b;
    c->flags = 0;
    if (c->use_joy) {
        poll_joystick();
        if (joy[0].stick[0].axis[1].d1) c->flags |= gamepad.up;
        if (joy[0].stick[0].axis[1].d2) c->flags |= gamepad.down;
        if (joy[0].stick[0].axis[0].d1) c->flags |= gamepad.left;
        if (joy[0].stick[0].axis[0].d2) c->flags |= gamepad.right;
        for (b = 0; b < joy[0].num_buttons && b < 32; b++)
            if (joy[0].button[b].b) c->flags |= gamepad.b[b];
    }
    if (!joystick_only) {
        if (key[c->key_up]) c->flags |= CTRL_UP;
        if (key[c->key_down]) c->flags |= CTRL_DOWN;
        if (key[c->key_left]) c->flags |= CTRL_LEFT;
        if (key[c->key_right]) c->flags |= CTRL_RIGHT;
        if (key[c->key_fire]) c->flags |= CTRL_FIRE;
        if (key[c->key_enter]) c->flags |= CTRL_ENTER;
        if (key[c->key_pause]) c->flags |= CTRL_PAUSE;
    }
}

int check_control_key(Tcontrol *c, int key)
{
    if (key == c->key_left || key == c->key_right ||
        key == c->key_up || key == c->key_down ||
        key == c->key_fire || key == c->key_enter || key == c->key_pause)
        return -1;
    return 0;
}

int is_up(Tcontrol *c) { return c->flags & CTRL_UP ? -1 : 0; }
int is_down(Tcontrol *c) { return c->flags & CTRL_DOWN ? -1 : 0; }
int is_left(Tcontrol *c) { return c->flags & CTRL_LEFT ? -1 : 0; }
int is_right(Tcontrol *c) { return c->flags & CTRL_RIGHT ? -1 : 0; }
int is_fire(Tcontrol *c) { return c->flags & CTRL_FIRE ? -1 : 0; }
int is_pause(Tcontrol *c) { return c->flags & CTRL_PAUSE ? -1 : 0; }
int is_enter(Tcontrol *c) { return c->flags & CTRL_ENTER ? -1 : 0; }
int is_any(Tcontrol *c) { return c->flags & ~CTRL_PAUSE ? -1 : 0; }

void save_control(Tcontrol *c, FILE *fp) { fwrite(c, sizeof(*c), 1, fp); }
void load_control(Tcontrol *c, FILE *fp) { fread(c, sizeof(*c), 1, fp); }

