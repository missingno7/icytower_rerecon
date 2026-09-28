/*
 * Ogg Vorbis sample loading for the portable build.
 *
 * Port of the two logg entry points the game uses (third_party/recovered/
 * logg.c: logg_load and the historical logg_load_memory extension; logg is
 * (c) 2007 Trent Gamblin, MIT licence, third_party/licenses/logg-license.txt).
 * The streaming half of logg is not used by the game and depends on
 * Allegro's AUDIOSTREAM, so it is not carried over.  Decoding is unchanged:
 * the same vendored libvorbis, 16-bit unsigned little-endian PCM (Allegro's
 * sample convention), interleaved when stereo.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/vorbisfile.h>
#include <allegro.h>
#include "port/game/port_game.h"

SAMPLE *logg_load(const char *filename);
SAMPLE *logg_load_memory(void *pData, size_t iSize);

struct memfile {
    const unsigned char *data;
    size_t size;
    size_t cursor;
};

static size_t mem_read(void *ptr, size_t size, size_t nmemb, void *ctx)
{
    struct memfile *m = ctx;
    size_t n = size * nmemb;
    if (m->cursor + n > m->size)
        n = m->size - m->cursor;
    if (n) {
        memcpy(ptr, m->data + m->cursor, n);
        m->cursor += n;
    }
    return n;
}

/* The historical seek ignored the offset for SEEK_END and always reported
 * success; libvorbis only uses it to find the stream end, so this is kept. */
static int mem_seek(void *ctx, ogg_int64_t offset, int whence)
{
    struct memfile *m = ctx;
    if (whence == SEEK_SET) m->cursor = (size_t)offset;
    else if (whence == SEEK_CUR) m->cursor += (size_t)offset;
    else if (whence == SEEK_END) m->cursor = m->size;
    return 0;
}

static long mem_tell(void *ctx)
{
    return (long)((struct memfile *)ctx)->cursor;
}

static int mem_close(void *ctx)
{
    free(ctx);
    return 0;
}

static SAMPLE *load_internal(OggVorbis_File *vf)
{
    vorbis_info *vi = ov_info(vf, -1);
    SAMPLE *samp;
    long total, n;
    size_t offset = 0, bytes;
    int bitstream;
    char buf[64 * 1024];

    total = (long)ov_pcm_total(vf, -1);
    if (!vi || total < 0) {
        ov_clear(vf);
        return NULL;
    }
    samp = create_sample(16, vi->channels > 1, (int)vi->rate, (int)total);
    if (!samp) {
        ov_clear(vf);
        return NULL;
    }
    bytes = (size_t)total * 2 * (vi->channels > 1 ? 2 : 1);
    while ((n = ov_read(vf, buf, (int)sizeof(buf), 0, 2, 0, &bitstream)) > 0) {
        size_t take = (size_t)n;
        if (offset + take > bytes)
            take = bytes - offset;
        memcpy((unsigned char *)samp->data + offset, buf, take);
        offset += take;
        if (offset >= bytes)
            break;
    }
    ov_clear(vf);
    return samp;
}

SAMPLE *logg_load_memory(void *pData, size_t iSize)
{
    OggVorbis_File ovf;
    ov_callbacks cb;
    struct memfile *m = malloc(sizeof(*m));
    if (!m)
        return NULL;
    m->data = pData;
    m->size = iSize;
    m->cursor = 0;
    cb.read_func = mem_read;
    cb.seek_func = mem_seek;
    cb.tell_func = mem_tell;
    cb.close_func = mem_close;
    if (ov_open_callbacks(m, &ovf, NULL, 0, cb) != 0) {
        free(m);
        return NULL;
    }
    return load_internal(&ovf);
}

SAMPLE *logg_load(const char *filename)
{
    OggVorbis_File ovf;
    FILE *f = port_fopen(filename, "rb");
    if (!f)
        return NULL;
    if (ov_open_callbacks(f, &ovf, NULL, 0, OV_CALLBACKS_DEFAULT) != 0) {
        fclose(f);
        return NULL;
    }
    return load_internal(&ovf);
}
