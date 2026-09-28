/* F:\projects\icytower\trunk\source\control.c -- complete 15-function CU.
 * Existing recovered bodies reunited at the historical CU boundary.
 * load_control/save_control transfer one sizeof(Tcontrol) record with
 * fread/fwrite, including historical natural tail padding.
 * Proof and experiments: docs/control-experiment.md.
 */
#include <allegro.h>
#include "control.h"
#include "port/input/input.h"
/* compat: pump pending input events (rate limited) */
void a4_input_service(void);

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
    /* port: all devices are read by the portable input layer
     * (port/input/input.h); this keeps the historical bindings and flag
     * semantics, with the gamepad Start button also pausing. */
    input_bindings bnd;
    int b;
    a4_input_service();
    bnd.key_left = c->key_left;
    bnd.key_right = c->key_right;
    bnd.key_up = c->key_up;
    bnd.key_down = c->key_down;
    bnd.key_fire = c->key_fire;
    bnd.key_enter = c->key_enter;
    bnd.key_pause = c->key_pause;
    bnd.use_pad = c->use_joy;
    bnd.pad_up = gamepad.up;
    bnd.pad_down = gamepad.down;
    bnd.pad_left = gamepad.left;
    bnd.pad_right = gamepad.right;
    for (b = 0; b < 32; b++)
        bnd.pad_button[b] = gamepad.b[b];
    c->flags = (unsigned char)input_control_flags(&bnd, joystick_only);
}

int check_control_key(Tcontrol *c, int k)
{
    if (k == c->key_left || k == c->key_right ||
        k == c->key_up || k == c->key_down ||
        k == c->key_fire || k == c->key_enter || k == c->key_pause)
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

