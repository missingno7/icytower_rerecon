/* Historical CU: F:\projects\icytower\trunk\source\timecompat.c
 * Ownership: VENDORED_UPSTREAM
 * Partial recovery: timegm is independently matched.
 */
#include <time.h>

time_t timegm(struct tm *ptm)
{
    time_t now = time(NULL);
    time_t nowlocal = mktime(localtime(&now));
    time_t nowgm = mktime(gmtime(&now));
    int diff = nowlocal - nowgm;

    return mktime(ptm) + diff;
}
