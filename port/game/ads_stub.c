/*
 * Replacement for the historical advertising module (src/fld_adspot.c,
 * src/httpget.c, src/csv.c, src/strptime.c, src/timecompat.c).
 *
 * The original downloaded a CSV of banner ads from freelunchdesign.com on a
 * background thread (Winsock + pthreads) and showed a cached banner in the
 * main menu.  The servers no longer exist and a source port should not
 * phone home, so the portable build never has an ad: fldads_get_random_ad()
 * returns NULL, which the menu code already handles (no banner is drawn).
 */
#include <stddef.h>

typedef struct FLDAdSpot FLDAdSpot;

void fldads_start(void) { }

const FLDAdSpot *fldads_get_random_ad(void) { return NULL; }
