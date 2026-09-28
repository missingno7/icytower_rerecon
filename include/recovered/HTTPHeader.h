/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_HTTPHEADER_H
#define RECOVERED_HTTPHEADER_H
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
typedef struct HTTPHeader {
    char *pHeader;
    char *pValue;
} HTTPHeader;
RECOVERED_ILP32_ASSERT(sizeof(HTTPHeader) == 8, HTTPHeader_size);
RECOVERED_ILP32_ASSERT(offsetof(HTTPHeader, pHeader) == 0, HTTPHeader_offset_pHeader);
RECOVERED_ILP32_ASSERT(offsetof(HTTPHeader, pValue) == 4, HTTPHeader_offset_pValue);
#endif
