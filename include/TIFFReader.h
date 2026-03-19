#ifndef TIFFREADER_H
#define TIFFREADER_H

#include <cstdio>
#include <cstring>
#include "Image.h"

class TIFFReader {
public:
    static Image load(const char* filename);

private:
    struct IFDEntry {
        unsigned short tag;
        unsigned short type;
        unsigned int   count;
        unsigned int   value;
    };

    inline static unsigned short parseU16(const unsigned char* buf) {
        return (unsigned short)(buf[0] | (buf[1] << 8));
    }

    inline static unsigned int parseU32(const unsigned char* buf) {
        return (unsigned int)buf[0]
             | ((unsigned int)buf[1] << 8)
             | ((unsigned int)buf[2] << 16)
             | ((unsigned int)buf[3] << 24);
    }

    inline static IFDEntry parseEntry(const unsigned char* buf) {
        IFDEntry e;
        memcpy(&e, buf, 12);
        return e;
    }

    static unsigned int resolveValue(
        const unsigned char* fileBuf,
        const IFDEntry& e
    );
};

#endif // TIFFREADER_H