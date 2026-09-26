{
    HTTPResponse *pResponse = malloc(sizeof(HTTPResponse));
    char slaskbuf[1024];
    char linebuf[1024];
    int i;

    memset(pResponse, 0, sizeof(HTTPResponse));
    i = extractLine(pHTTPData, iResponseBytesCount, linebuf, sizeof(linebuf));
    if (sscanf(linebuf, "HTTP/%s %d", slaskbuf, &pResponse->iStatusCode) != 2) {
        log2file("Malformed HTTP response:\n%s", pHTTPData);
        destroyHTTPResponse(pResponse);
        return NULL;
    }

    while (1) {
        int bytesRead = extractLine(pHTTPData + i, iResponseBytesCount - i,
                                    linebuf, sizeof(linebuf));
        HTTPHeader *header;
        int j;

        i += bytesRead;
        if (bytesRead == 2)
            break;
        pResponse->iNumHeaders++;
        pResponse->pHeaders = realloc(pResponse->pHeaders,
                                      pResponse->iNumHeaders * sizeof(HTTPHeader));
        header = &pResponse->pHeaders[pResponse->iNumHeaders - 1];
        if (linebuf[0] == ':') {
            j = 0;
        } else {
            for (j = 1; j < bytesRead; j++) {
                if (linebuf[j] == ':')
                    break;
            }
        }
        header->pHeader = malloc(j + 1);
        memcpy(header->pHeader, linebuf, j);
        header->pHeader[j] = 0;
        header->pValue = malloc(bytesRead - j - 1);
        memcpy(header->pValue, linebuf + j + 2, bytesRead - j - 2);
        header->pValue[bytesRead - j - 2] = 0;
    }

    pResponse->iPayloadSize = iResponseBytesCount - i;
    if (pResponse->iPayloadSize) {
        pResponse->pPayload = malloc(pResponse->iPayloadSize + 1);
        memcpy(pResponse->pPayload, pHTTPData + i, pResponse->iPayloadSize);
        pResponse->pPayload[pResponse->iPayloadSize] = 0;
    }
    return pResponse;
}