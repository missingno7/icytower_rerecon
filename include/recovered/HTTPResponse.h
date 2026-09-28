/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_HTTPRESPONSE_H
#define RECOVERED_HTTPRESPONSE_H
#include <stddef.h>
#include "HTTPHeader.h"
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
typedef struct HTTPResponse {
    int iStatusCode;
    unsigned int iNumHeaders;
    HTTPHeader *pHeaders;
    unsigned char *pPayload;
    unsigned int iPayloadSize;
} HTTPResponse;
RECOVERED_ILP32_ASSERT(sizeof(HTTPResponse) == 20, HTTPResponse_size);
RECOVERED_ILP32_ASSERT(offsetof(HTTPResponse, iStatusCode) == 0, HTTPResponse_offset_iStatusCode);
RECOVERED_ILP32_ASSERT(offsetof(HTTPResponse, iNumHeaders) == 4, HTTPResponse_offset_iNumHeaders);
RECOVERED_ILP32_ASSERT(offsetof(HTTPResponse, pHeaders) == 8, HTTPResponse_offset_pHeaders);
RECOVERED_ILP32_ASSERT(offsetof(HTTPResponse, pPayload) == 12, HTTPResponse_offset_pPayload);
RECOVERED_ILP32_ASSERT(offsetof(HTTPResponse, iPayloadSize) == 16, HTTPResponse_offset_iPayloadSize);
#endif
