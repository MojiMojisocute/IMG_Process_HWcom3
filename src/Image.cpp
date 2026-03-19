#include <algorithm>
#include <cstring>
#include "Image.h"

Image::Image() 
    : width(0), height(0), channels(1) {}

Image::Image(int w, int h, int ch)
    : width(w), height(h), channels(ch),
      pixels(w * h * ch, 0) {}

Image Image::clone() const{
    return *this;
}

void Image::fill(unsigned char value){
    memset(pixels.data(), value, pixels.size());
}