/* Historical csv.c recovered from DWARF and disassembly; upstream origin unresolved. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

CSVParseContext *csv_open(const char *pFilename)
{
    FILE *fp = fopen(pFilename, "rb");
    CSVParseContext *pCtx;
    if (!fp) return NULL;
    pCtx = malloc(sizeof(CSVParseContext));
    pCtx->iDocSize = 0;
    pCtx->pDoc = NULL;
    while (!feof(fp)) {
        int bytesRead;
        pCtx->pDoc = realloc(pCtx->pDoc, pCtx->iDocSize + 1024);
        bytesRead = fread(pCtx->pDoc + pCtx->iDocSize, 1, 1024, fp);
        pCtx->iDocSize += bytesRead;
    }
    pCtx->pLineStart = pCtx->pDoc;
    pCtx->iFields = 0;
    pCtx->iFieldCapacity = 0;
    pCtx->pFieldPtrs = NULL;
    fclose(fp);
    return pCtx;
}

CSVParseContext *csv_begin(const unsigned char *pData, size_t iDataSize)
{
    CSVParseContext *pCtx;
    if (!iDataSize || !pData) return NULL;
    pCtx = malloc(sizeof(CSVParseContext));
    pCtx->pDoc = malloc(iDataSize);
    memcpy(pCtx->pDoc, pData, iDataSize);
    pCtx->iDocSize = iDataSize;
    pCtx->pLineStart = pCtx->pDoc;
    pCtx->iFields = 0;
    pCtx->iFieldCapacity = 0;
    pCtx->pFieldPtrs = NULL;
    return pCtx;
}

void csv_destroy(CSVParseContext *pCtx)
{
    if (!pCtx) return;
    free(pCtx->pDoc);
    free(pCtx->pFieldPtrs);
    free(pCtx);
}

void csv_add_field(CSVParseContext *pCtx, char *pStart)
{
    if (pCtx->iFields >= pCtx->iFieldCapacity) {
        pCtx->iFieldCapacity = pCtx->iFields + 4;
        pCtx->pFieldPtrs = realloc(pCtx->pFieldPtrs, pCtx->iFieldCapacity * sizeof(char *));
    }
    pCtx->pFieldPtrs[pCtx->iFields++] = pStart;
}

int csv_next(CSVParseContext *pCtx)
{
    const unsigned char *pDocEnd;
    unsigned char *p;
    if (!pCtx) return 0;
    pDocEnd = pCtx->pDoc + pCtx->iDocSize;
    pCtx->iFields = 0;
    if (pCtx->pLineStart >= pDocEnd) return 0;
    if (pCtx->pLineStart != pCtx->pDoc) pCtx->pLineStart[-1] = '\n';
    p = pCtx->pLineStart;
    while (p < pDocEnd && (*p == '#' || *p == '\n' || *p == '\r')) {
        while (p < pDocEnd && *p != '\n') p++;
        p++;
    }
    if (p >= pDocEnd) return 0;
    csv_add_field(pCtx, (char *)p);
    while (p < pDocEnd && *p != '\n') {
        if (*p == ',' || *p == 0) {
            *p = 0;
            csv_add_field(pCtx, (char *)(p + 1));
        }
        p++;
    }
    *p = 0;
    pCtx->pLineStart = p + 1;
    return pCtx->iFields;
}

void csv_rewind(CSVParseContext *pCtx)
{
    if (!pCtx) return;
    pCtx->iFields = 0;
    pCtx->pLineStart = pCtx->pDoc;
}
