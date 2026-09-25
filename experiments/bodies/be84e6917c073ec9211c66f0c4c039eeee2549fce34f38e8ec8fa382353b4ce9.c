{
    int pos;
    char tempBuf[256];
    int tempPos;

    pos = 0;
    tempPos = 0;
    while (*buffer) {
        if (*buffer == '\n') {
            tempBuf[tempPos] = 0;
            textprintf_ex(bmp, data[53].dat, x, y, makecol(30, 20, 10),
                          -1, "%s", tempBuf);
            y += 10;
            tempPos = 0;
        } else {
            tempBuf[tempPos] = *buffer;
            tempPos++;
        }
        buffer++;
    }
    return y;
}