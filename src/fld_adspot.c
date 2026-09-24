extern void log2file(const char*, ...);
/* Partial historical fld_adspot.c recovery. */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include "directories.h"
#include "csv.h"

#include "recovered/FLDAdSpot.h"

#include "recovered/HTTPHeader.h"
typedef struct HTTPResponse {
    int iStatusCode;
    unsigned int iNumHeaders;
    HTTPHeader *pHeaders;
    unsigned char *pPayload;
    unsigned int iPayloadSize;
} HTTPResponse;

extern HTTPResponse *HTTPHead(const char *pURL);
extern HTTPResponse *HTTPGet(const char *pURL);
extern time_t httpGetLastModified(HTTPResponse *pResponse);
extern void destroyHTTPResponse(HTTPResponse *pResponse);

pthread_t gFLDADThread;
pthread_mutex_t gFLDADMutex;
int giAdCacheSize;
FLDAdSpot *gpAdCache;

void fldads_update_local_adimg(const char *pRemoteName);
void *fldads_threadmain(void *data);

void fldads_destroy_cache(void)
{
    if (gpAdCache) {
        int i;
        for (i = 0; i < giAdCacheSize; i++) {
            free(gpAdCache[i].pLocalImagePath);
            free(gpAdCache[i].pRemoteImageURL);
            free(gpAdCache[i].pVisitURL);
        }
        free(gpAdCache);
        gpAdCache = NULL;
        giAdCacheSize = 0;
    }
}

const char *fldads_get_local_cache_name(const char *pFileName)
{
    static char localFilename[256];
    get_adcache_dir(localFilename, sizeof(localFilename));
    mkdir(localFilename);
    strcat(localFilename, pFileName);
    return localFilename;
}

void fldads_dump_local_cache(void)
{
    FILE *fp = fopen(fldads_get_local_cache_name("ads.csv"), "wb");
    if (fp) {
        int i;
        pthread_mutex_lock(&gFLDADMutex);
        for (i = 0; i < giAdCacheSize; i++) {
            fprintf(fp, "%s,%s,%.1f\n", gpAdCache[i].pRemoteImageURL,
                    gpAdCache[i].pVisitURL, gpAdCache[i].fFrequency);
        }
        pthread_mutex_unlock(&gFLDADMutex);
        fclose(fp);
    }
}

const char *get_url_filename(const char *pURL)
{
    char *p;
    p = pURL + strlen(pURL) - 1;
    while (*(p - 1) != '/') {
        p--;
    }
    return p;
}

const char *fldads_get_local_filename_from_url(const char *pRemoteName)
{
    return fldads_get_local_cache_name(get_url_filename(pRemoteName));
}

void fldads_load_cache_from_csv(CSVParseContext *pCsv)
{
    FLDAdSpot *pCache = NULL;
    int iCacheSize = 0;

    if (pCsv) {
        while (csv_next(pCsv) == 3) {
            char *localFilename = fldads_get_local_filename_from_url(pCsv->pFieldPtrs[0]);
            struct stat statFile;
            if (stat(localFilename, &statFile)) {
                log2file("Warning: local file missing for %s, ad will not be shown", pCsv->pFieldPtrs[0]);
            } else {
                FLDAdSpot *pAd;
                pCache = realloc(pCache, sizeof(*pCache) * (iCacheSize + 1));
                pAd = pCache + iCacheSize;
                pAd->pRemoteImageURL = strdup(pCsv->pFieldPtrs[0]);
                pAd->pLocalImagePath = strdup(fldads_get_local_filename_from_url(pAd->pRemoteImageURL));
                pAd->pVisitURL = strdup(pCsv->pFieldPtrs[1]);
                pAd->fFrequency = strtof(pCsv->pFieldPtrs[2], NULL);
                iCacheSize++;
            }
        }
        pthread_mutex_lock(&gFLDADMutex);
        fldads_destroy_cache();
        gpAdCache = pCache;
        giAdCacheSize = iCacheSize;
        pthread_mutex_unlock(&gFLDADMutex);
    }
}

void fldads_load_local_cache(void)
{
    CSVParseContext *pCsv = csv_open(fldads_get_local_cache_name("ads.csv"));
    if (pCsv) {
        fldads_load_cache_from_csv(pCsv);
        csv_destroy(pCsv);
    }
}

void fldads_update_local_adimg(const char *pRemoteName)
{
    char *localFilename = fldads_get_local_filename_from_url(pRemoteName);
    struct stat localStat;
    HTTPResponse *pResponse;

    if (!stat(localFilename, &localStat)) {
        time_t lastModified;
        HTTPResponse *pHead = HTTPHead(pRemoteName);
        if (pHead && pHead->iStatusCode == 200) {
            lastModified = httpGetLastModified(pHead);
            if (lastModified && localStat.st_mtime >= lastModified) {
                log2file("Local file %s is newer (%d) than server (%d), using local",
                         localFilename, localStat.st_mtime, lastModified);
                return;
            }
        }
    }

    pResponse = HTTPGet(pRemoteName);
    log2file("Downloading %s -> %s", pRemoteName, localFilename);
    if (pResponse && pResponse->iStatusCode == 200) {
        FILE *fp = fopen(localFilename, "wb");
        if (fp) {
            fwrite(pResponse->pPayload, 1, pResponse->iPayloadSize, fp);
            fclose(fp);
        }
    }
    destroyHTTPResponse(pResponse);
}

void fldads_update_cache(unsigned char *pData, int iDataSize)
{
    CSVParseContext *pCsv = csv_begin(pData, iDataSize);
    if (!pCsv) {
        log2file("Failed to start parsing CSV");
    }
    while (csv_next(pCsv) == 3) {
        fldads_update_local_adimg(pCsv->pFieldPtrs[0]);
    }
    csv_rewind(pCsv);
    fldads_load_cache_from_csv(pCsv);
    csv_destroy(pCsv);
    fldads_dump_local_cache();
}

void fldads_start(void)
{
    pthread_create(&gFLDADThread, NULL, fldads_threadmain, NULL);
}

const FLDAdSpot *fldads_get_random_ad(void)
{
    FLDAdSpot *pAd = NULL;

    pthread_mutex_lock(&gFLDADMutex);
    if (giAdCacheSize > 0) {
        float fCumulativeProbability = 0.0f;
        int i;
        for (i = 0; i < giAdCacheSize; i++) {
            fCumulativeProbability += gpAdCache[i].fFrequency;
        }
        {
            float f = ((float)rand() / RAND_MAX) * fCumulativeProbability;
            int iChoice;
            iChoice = 0;
            while (iChoice < giAdCacheSize && f >= 0.0f) {
                pAd = &gpAdCache[iChoice++];
                f -= pAd->fFrequency;
            }
        }
    }
    pthread_mutex_unlock(&gFLDADMutex);
    return pAd;
}

void *fldads_threadmain(void *data)
{
    int shouldDownloadAds;
    struct stat statCsv;

    fldads_load_local_cache();
    shouldDownloadAds = stat(fldads_get_local_cache_name("ads.csv"), &statCsv);
    if (!shouldDownloadAds && statCsv.st_mtime + 259200 < time(NULL)) {
        shouldDownloadAds = 1;
    }
    if (shouldDownloadAds) {
        HTTPResponse *pResponse;
        log2file("Downloading ad listing");
        pResponse = HTTPGet("http://www.icytower.com/icytower_pc.csv");
        if (pResponse && pResponse->iStatusCode == 200 && pResponse->pPayload) {
            fldads_update_cache(pResponse->pPayload, pResponse->iPayloadSize);
        } else {
            log2file("Could not fetch ad listing from http://www.icytower.com/icytower_pc.csv (%d), skipping ad update",
                     pResponse ? pResponse->iStatusCode : 0);
        }
        destroyHTTPResponse(pResponse);
    } else {
        log2file("Cached ads are up to date");
    }
    log2file("There are %d available ad spots.", giAdCacheSize);
    return NULL;
}
