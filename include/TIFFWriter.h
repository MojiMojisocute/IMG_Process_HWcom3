#ifndef TIFFWRITER_H
#define TIFFWRITER_H

#include "Image.h"

class TIFFWriter {
public:
    static bool save(const char* filename, const Image& img);
};

#endif // TIFFWRITER_H