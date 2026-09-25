{
    int shouldDownloadAds;
    struct stat statCsv;

    fldads_load_local_cache();
    shouldDownloadAds = stat(fldads_get_local_cache_name("ads.csv"), &statCsv);
    if (!shouldDownloadAds && statCsv.st_mtime + 259200 < time(NULL)) {
        shouldDownloadAds = 1;
    }
    if (!shouldDownloadAds) {
        log2file("Cached ads are up to date");
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
    }
    log2file("There are %d available ad spots.", giAdCacheSize);
    return NULL;
}