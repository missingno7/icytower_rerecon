/*
 * port_save_state (port/game/port_game.h).
 * The game writes tower.cfg (options, last profile, high scores) only after a
 * game and when it quits.  Mobile systems kill backgrounded apps without any
 * exit path (and freeze the game thread before a "backgrounded" event could
 * be handled on it), so the port also saves whenever that data changes: after
 * creating or changing a profile and when a submenu such as Options closes.
 * Called on the game thread between screens; nothing is written before the
 * game has loaded its data, so defaults never overwrite the player's files. */
#include "recovered/Tprofile.h"
#include "recovered/Tmenu_selection.h"
#include "port/config/port_config.h"

extern Tprofile *profile;
extern void save_config(void);
extern int save_profile(Tprofile *p);
extern void syncProfileFromOptions(void);

void port_save_state(void)
{
#ifdef __ANDROID__
   {
      extern Tmenu_selection touch_selection;   /* Options > Controls > Touch */
      port_config_set_touch_scheme(touch_selection.value);
   }
#endif
   if (!profile)
      return;
   save_config();
   syncProfileFromOptions();
   save_profile(profile);
}
