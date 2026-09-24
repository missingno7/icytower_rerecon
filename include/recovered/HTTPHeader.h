/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_HTTPHEADER_H
#define RECOVERED_HTTPHEADER_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct HTTPHeader {
    char *pHeader;
    char *pValue;
} HTTPHeader;
RECOVERED_STATIC_ASSERT(sizeof(HTTPHeader) == 8, HTTPHeader_size);
RECOVERED_STATIC_ASSERT(offsetof(HTTPHeader, pHeader) == 0, HTTPHeader_offset_pHeader);
RECOVERED_STATIC_ASSERT(offsetof(HTTPHeader, pValue) == 4, HTTPHeader_offset_pValue);
#endif
