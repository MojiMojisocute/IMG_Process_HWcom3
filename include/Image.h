#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

class Image {
private:
    int width;
    int height;
    int channels;
    std::vector<unsigned char> pixels;

    inline int getIndex(int x, int y, int ch = 0) const {
        return (y * width + x) * channels + ch;
    }

public:
    Image();
    Image(int w, int h, int ch = 1);

    inline int getWidth()    const { return width;    }
    inline int getHeight()   const { return height;   }
    inline int getChannels() const { return channels; }

    inline unsigned char* getRawPixels() {
        return pixels.data();
    }

    inline const unsigned char* getRawPixels() const {
        return pixels.data();
    }

    inline unsigned char getPixel(int x, int y, int ch = 0) const {
        return pixels[(y * width + x) * channels + ch];
    }

    inline void setPixel(int x, int y, unsigned char value, int ch = 0) {
        pixels[(y * width + x) * channels + ch] = value;
    }

    inline bool isValid(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    Image clone() const;
    void fill(unsigned char value);
};

#endif // IMAGE_H