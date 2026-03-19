#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "TIFFReader.h"

Image TIFFReader::load(const char* filename){
    FILE* f = fopen(filename, "rb");
    if (!f) return Image();

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fileSize <= 0){ fclose(f); return Image(); }

    unsigned char* buf = (unsigned char*)malloc(fileSize);
    if (!buf){ fclose(f); return Image(); }

    if ((long)fread(buf, 1, fileSize, f) != fileSize){
        free(buf); fclose(f); return Image();
    }
    fclose(f);

    if (buf[0] != 0x49 || buf[1] != 0x49) { free(buf); return Image(); }
    if (parseU16(buf + 2) != 42) { free(buf); return Image(); }

    unsigned int ifdOffset = parseU32(buf + 4);
    if (ifdOffset >= (unsigned int)fileSize) { free(buf); return Image(); }

    unsigned short numEntries = parseU16(buf + ifdOffset);
    const unsigned char* entryPtr = buf + ifdOffset + 2;

    int width = 0;
    int height = 0;
    int samplesPerPixel = 3;
    int bitsPerSample = 8;
    int compression = 1;
    unsigned int rowsPerStrip = 0;
    unsigned int stripOffsetsOffset = 0;
    unsigned int stripByteCountsOffset = 0;
    unsigned int stripOffsetsCount = 1;
    unsigned int stripByteCountsCount = 1;

    for (int i = 0; i < numEntries; i++, entryPtr += 12){
        IFDEntry e = parseEntry(entryPtr);
        switch (e.tag){
            case 256: width             = resolveValue(buf, e); break;
            case 257: height            = resolveValue(buf, e); break;
            case 258: bitsPerSample     = resolveValue(buf, e); break;
            case 259: compression       = resolveValue(buf, e); break;
            case 262: break;
            case 273:
                stripOffsetsOffset      = e.value;
                stripOffsetsCount       = e.count;
                break;
            case 277: samplesPerPixel   = resolveValue(buf, e); break;
            case 278: rowsPerStrip      = resolveValue(buf, e); break;
            case 279:
                stripByteCountsOffset   = e.value;
                stripByteCountsCount    = e.count;
                break;
        }
    }

    if (width <= 0 || height <= 0) { free(buf); return Image(); }
    if (compression != 1) { free(buf); return Image(); }
    if (bitsPerSample != 8) { free(buf); return Image(); }
    if (samplesPerPixel != 1 &&
        samplesPerPixel != 3) { free(buf); return Image(); }

    if (rowsPerStrip == 0) rowsPerStrip = height;

    unsigned int numStrips = (height + rowsPerStrip - 1) / rowsPerStrip;

    Image img(width, height, samplesPerPixel);
    unsigned char* dest = img.getRawPixels();

    for (unsigned int s = 0; s < numStrips; s++){
        unsigned int sOffset = (stripOffsetsCount == 1)
            ? stripOffsetsOffset
            : parseU32(buf + stripOffsetsOffset + s * 4);

        unsigned int sByteCount = (stripByteCountsCount == 1)
            ? stripByteCountsOffset
            : parseU32(buf + stripByteCountsOffset + s * 4);

        if (sOffset + sByteCount > (unsigned int)fileSize) {
            free(buf); return Image();
        }

        memcpy(dest, buf + sOffset, sByteCount);
        dest += sByteCount;
    }

    free(buf);
    return img;
}

unsigned int TIFFReader::resolveValue(const unsigned char* buf, const IFDEntry& e){
    if (e.count == 1){
        if (e.type == 3) return e.value & 0xFFFF;
        if (e.type == 4) return e.value;
    }
    if (e.type == 3) return parseU16(buf + e.value);
    return parseU32(buf + e.value);
}