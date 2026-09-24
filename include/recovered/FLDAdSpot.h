/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_FLDADSPOT_H
#define RECOVERED_FLDADSPOT_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct FLDAdSpot {
    char *pRemoteImageURL;
    char *pLocalImagePath;
    char *pVisitURL;
    float fFrequency;
} FLDAdSpot;
RECOVERED_STATIC_ASSERT(sizeof(FLDAdSpot) == 16, FLDAdSpot_size);
RECOVERED_STATIC_ASSERT(offsetof(FLDAdSpot, pRemoteImageURL) == 0, FLDAdSpot_offset_pRemoteImageURL);
RECOVERED_STATIC_ASSERT(offsetof(FLDAdSpot, pLocalImagePath) == 4, FLDAdSpot_offset_pLocalImagePath);
RECOVERED_STATIC_ASSERT(offsetof(FLDAdSpot, pVisitURL) == 8, FLDAdSpot_offset_pVisitURL);
RECOVERED_STATIC_ASSERT(offsetof(FLDAdSpot, fFrequency) == 12, FLDAdSpot_offset_fFrequency);
#endif
