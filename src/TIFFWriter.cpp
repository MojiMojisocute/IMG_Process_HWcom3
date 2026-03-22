#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "TIFFWriter.h"

static void writeU16(unsigned char* buf, unsigned short v) {
    buf[0] = (unsigned char)(v & 0xFF);
    buf[1] = (unsigned char)((v >> 8) & 0xFF);
}
static void writeU32(unsigned char* buf, unsigned int v) {
    buf[0] = (unsigned char)(v & 0xFF);
    buf[1] = (unsigned char)((v >> 8) & 0xFF);
    buf[2] = (unsigned char)((v >> 16) & 0xFF);
    buf[3] = (unsigned char)((v >> 24) & 0xFF);
}
static void writeEntry(unsigned char* buf,
                       unsigned short tag, unsigned short type,
                       unsigned int count, unsigned int value) {
    writeU16(buf + 0, tag);
    writeU16(buf + 2, type);
    writeU32(buf + 4, count);
    writeU32(buf + 8, value);
}

bool TIFFWriter::save(const char* filename, const Image& img) {
    int w   = img.getWidth();
    int h   = img.getHeight();
    int ch  = img.getChannels();

    unsigned int imageDataSize = (unsigned int)(w * h * ch);
    unsigned int imageDataOffset = 8;

    int numEntries = 10;
    if (ch == 3) numEntries = 11;

    unsigned int bitsPerSampleOffset = 0;
    unsigned int extraDataOffset = imageDataOffset + imageDataSize;
    if (extraDataOffset & 1) extraDataOffset++;

    unsigned int ifdOffset = extraDataOffset;
    if (ch == 3) {
        bitsPerSampleOffset = extraDataOffset;
        ifdOffset = bitsPerSampleOffset + 6;
    }
    if (ifdOffset & 1) ifdOffset++;

    unsigned int totalSize = ifdOffset + 2 + numEntries * 12 + 4;
    unsigned char* buf = (unsigned char*)calloc(totalSize, 1);
    if (!buf) return false;

    buf[0] = 0x49; buf[1] = 0x49;
    writeU16(buf + 2, 42);
    writeU32(buf + 4, ifdOffset);

    memcpy(buf + imageDataOffset, img.getRawPixels(), imageDataSize);

    if (ch == 3) {
        writeU16(buf + bitsPerSampleOffset + 0, 8);
        writeU16(buf + bitsPerSampleOffset + 2, 8);
        writeU16(buf + bitsPerSampleOffset + 4, 8);
    }

    unsigned char* ifd = buf + ifdOffset;
    writeU16(ifd, (unsigned short)numEntries);
    ifd += 2;

    unsigned int idx = 0;
    writeEntry(ifd + idx*12, 256, 4, 1, (unsigned int)w);           idx++;
    writeEntry(ifd + idx*12, 257, 4, 1, (unsigned int)h);           idx++;
    if (ch == 1) {
        writeEntry(ifd + idx*12, 258, 3, 1, 8);                     idx++;
    } else {
        writeEntry(ifd + idx*12, 258, 3, 3, bitsPerSampleOffset);   idx++;
    }
    writeEntry(ifd + idx*12, 259, 3, 1, 1);                         idx++;
    writeEntry(ifd + idx*12, 262, 3, 1, (unsigned int)(ch == 1 ? 1 : 2)); idx++;
    writeEntry(ifd + idx*12, 273, 4, 1, imageDataOffset);           idx++;
    writeEntry(ifd + idx*12, 277, 3, 1, (unsigned int)ch);          idx++;
    writeEntry(ifd + idx*12, 278, 4, 1, (unsigned int)h);           idx++;
    writeEntry(ifd + idx*12, 279, 4, 1, imageDataSize);             idx++;
    writeEntry(ifd + idx*12, 282, 5, 1, 0);                         idx++;

    writeU32(ifd + numEntries * 12, 0);

    FILE* f = fopen(filename, "wb");
    if (!f) { free(buf); return false; }
    fwrite(buf, 1, totalSize, f);
    fclose(f);
    free(buf);
    return true;
}