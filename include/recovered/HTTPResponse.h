/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_HTTPRESPONSE_H
#define RECOVERED_HTTPRESPONSE_H
#include <stddef.h>
#include "HTTPHeader.h"
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct HTTPResponse {
    int iStatusCode;
    unsigned int iNumHeaders;
    HTTPHeader *pHeaders;
    unsigned char *pPayload;
    unsigned int iPayloadSize;
} HTTPResponse;
RECOVERED_STATIC_ASSERT(sizeof(HTTPResponse) == 20, HTTPResponse_size);
RECOVERED_STATIC_ASSERT(offsetof(HTTPResponse, iStatusCode) == 0, HTTPResponse_offset_iStatusCode);
RECOVERED_STATIC_ASSERT(offsetof(HTTPResponse, iNumHeaders) == 4, HTTPResponse_offset_iNumHeaders);
RECOVERED_STATIC_ASSERT(offsetof(HTTPResponse, pHeaders) == 8, HTTPResponse_offset_pHeaders);
RECOVERED_STATIC_ASSERT(offsetof(HTTPResponse, pPayload) == 12, HTTPResponse_offset_pPayload);
RECOVERED_STATIC_ASSERT(offsetof(HTTPResponse, iPayloadSize) == 16, HTTPResponse_offset_iPayloadSize);
#endif
