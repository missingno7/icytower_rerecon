/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_FLDADSPOT_H
#define RECOVERED_FLDADSPOT_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
#ifndef RECOVERED_ILP32_ASSERT
/* portable build: pointer-bearing layouts are only fixed on 32-bit targets */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ != 4
#define RECOVERED_ILP32_ASSERT(expr, name) typedef char recovered_ilp32_unchecked_##name
#else
#define RECOVERED_ILP32_ASSERT(expr, name) RECOVERED_STATIC_ASSERT(expr, name)
#endif
#endif
typedef struct FLDAdSpot {
    char *pRemoteImageURL;
    char *pLocalImagePath;
    char *pVisitURL;
    float fFrequency;
} FLDAdSpot;
RECOVERED_ILP32_ASSERT(sizeof(FLDAdSpot) == 16, FLDAdSpot_size);
RECOVERED_ILP32_ASSERT(offsetof(FLDAdSpot, pRemoteImageURL) == 0, FLDAdSpot_offset_pRemoteImageURL);
RECOVERED_ILP32_ASSERT(offsetof(FLDAdSpot, pLocalImagePath) == 4, FLDAdSpot_offset_pLocalImagePath);
RECOVERED_ILP32_ASSERT(offsetof(FLDAdSpot, pVisitURL) == 8, FLDAdSpot_offset_pVisitURL);
RECOVERED_ILP32_ASSERT(offsetof(FLDAdSpot, fFrequency) == 12, FLDAdSpot_offset_fFrequency);
#endif
