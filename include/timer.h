/* Reconstructed interface. Original header basename is not asserted. */
#ifndef ICYTOWER_RECON_TIMER_H
#define ICYTOWER_RECON_TIMER_H

extern volatile int frame_count;
extern volatile int fps;
extern volatile int logic_count;
extern volatile int lps;
extern volatile int cycle_count;

void fps_counter(void);
void cycle_counter(void);
int install_timers(void);

#endif
