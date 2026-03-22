#include <cstdio>
#include <cstring>
#include <cmath>
#include "TIFFReader.h"
#include "TIFFWriter.h"
#include "Preprocessing.h"
#include "CCL.h"

static void drawCircle(Image& img, int cx, int cy, int radius) {
    int w = img.getWidth();
    int h = img.getHeight();
    for (int i = 0; i < 360; i++) {
        double angle = i * 3.14159265358979 / 180.0;
        for (int t = -1; t <= 1; t++) {
            int r = radius + t;
            int x = cx + (int)(r * cos(angle));
            int y = cy + (int)(r * sin(angle));
            if (x >= 0 && x < w && y >= 0 && y < h) {
                img.setPixel(x, y, 255, 0);
                img.setPixel(x, y,   0, 1);
                img.setPixel(x, y,   0, 2);
            }
        }
    }
}

static void processImage(const char* filename, const char* prefix) {
    printf("=== %s ===\n", filename);

    Image rgb = TIFFReader::load(filename);
    if (rgb.getWidth() == 0) {
        printf("ERROR: cannot load %s\n", filename);
        return;
    }
    printf("Loaded: %d x %d\n", rgb.getWidth(), rgb.getHeight());

    Image gray   = Preprocessing::toGray(rgb);
    Image median = Preprocessing::medianBlur(gray, 11);

    CCLDebug dbg = CCL::detectDebug(median);

    printf("WormHoles found: %d\n", (int)dbg.holes.size());
    for (int i = 0; i < (int)dbg.holes.size(); i++)
        printf("  Hole %d : cx=%d cy=%d area=%d\n",
               i+1, dbg.holes[i].cx, dbg.holes[i].cy, dbg.holes[i].area);
    printf("\n");

    Image final_img(rgb.getWidth(), rgb.getHeight(), 3);
    memcpy(final_img.getRawPixels(), rgb.getRawPixels(),
           rgb.getWidth() * rgb.getHeight() * 3);
    for (int i = 0; i < (int)dbg.holes.size(); i++)
        drawCircle(final_img, dbg.holes[i].cx, dbg.holes[i].cy, 6);

    char path[256];
    snprintf(path, sizeof(path), "output/%s_1_Original.tif",  prefix); TIFFWriter::save(path, gray);
    snprintf(path, sizeof(path), "output/%s_2_Median.tif",    prefix); TIFFWriter::save(path, median);
    snprintf(path, sizeof(path), "output/%s_3_Threshold.tif", prefix); TIFFWriter::save(path, dbg.threshold);
    snprintf(path, sizeof(path), "output/%s_4_Clean.tif",     prefix); TIFFWriter::save(path, dbg.clean);
    snprintf(path, sizeof(path), "output/%s_5_Filtered.tif",  prefix); TIFFWriter::save(path, dbg.filtered);
    snprintf(path, sizeof(path), "output/%s_6_Final.tif",     prefix); TIFFWriter::save(path, final_img);
}

int main() {
    processImage("Data/WormHole_1H.tif", "WH1");
    processImage("Data/WormHole_2H.tif", "WH2");
    return 0;
}