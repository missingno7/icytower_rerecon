{
    int pos;
    char tempBuf[256];
    int tempPos;

    tempPos = 0;
    pos = 0;
    while (buffer[pos]) {
        if (buffer[pos] == '\n') {
            tempBuf[tempPos] = 0;
            textprintf_ex(bmp, data[53].dat, x, y, makecol(30, 20, 10),
                          -1, "%s", tempBuf);
            y += 10;
            tempPos = 0;
        } else {
            tempBuf[tempPos] = buffer[pos];
            tempPos++;
        }
        pos++;
    }
    return y;
}