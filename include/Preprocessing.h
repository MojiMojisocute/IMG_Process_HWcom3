#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include "Image.h"

class Preprocessing {
public:
    static Image toGray(const Image& src);
    static Image gaussianBlur(const Image& src);
    static Image medianBlur(const Image& src, int kernelSize);
private:
    constexpr static const int KERNEL_SIZE = 5;
    constexpr static int kernel[5][5] = {
        { 1,  4,  6,  4,  1 },
        { 4, 16, 24, 16,  4 },
        { 6, 24, 36, 24,  6 },
        { 4, 16, 24, 16,  4 },
        { 1,  4,  6,  4,  1 }
    };
    constexpr static int kernelSum = 256;
};

#endif // PREPROCESSING_H