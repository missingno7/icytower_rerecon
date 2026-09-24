#ifndef ICYTOWER_CSV_H
#define ICYTOWER_CSV_H
#include <stddef.h>
typedef struct CSVParseContext {
    unsigned char *pDoc;
    size_t iDocSize;
    unsigned char *pLineStart;
    int iFields;
    int iFieldCapacity;
    char **pFieldPtrs;
} CSVParseContext;
CSVParseContext *csv_open(const char *pFilename);
CSVParseContext *csv_begin(const unsigned char *pData, size_t iDataSize);
void csv_destroy(CSVParseContext *pCtx);
void csv_add_field(CSVParseContext *pCtx, char *pStart);
int csv_next(CSVParseContext *pCtx);
void csv_rewind(CSVParseContext *pCtx);
#endif
