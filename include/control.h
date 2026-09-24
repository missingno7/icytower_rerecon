/* Historical filename and layouts: control CU DIEs 0x44c0..0x45f1.
 * Natural i386 C alignment supplies Tcontrol's three trailing pad bytes.
 */
#ifndef ICYTOWER_CONTROL_H
#define ICYTOWER_CONTROL_H
#include <stdio.h>

#include "recovered/Tcontrol.h"

#include "recovered/Tgamepad.h"

extern Tgamepad gamepad;
void init_control(Tcontrol *c);
void set_control(Tcontrol *c, int up, int down, int left, int right, int fire);
Tgamepad *get_gamepad(void);
void poll_control(Tcontrol *c, int joystick_only);
int check_control_key(Tcontrol *c, int key);
int is_up(Tcontrol *c);
int is_down(Tcontrol *c);
int is_left(Tcontrol *c);
int is_right(Tcontrol *c);
int is_fire(Tcontrol *c);
int is_pause(Tcontrol *c);
int is_enter(Tcontrol *c);
int is_any(Tcontrol *c);
void load_control(Tcontrol *c, FILE *fp);
void save_control(Tcontrol *c, FILE *fp);
#endif
