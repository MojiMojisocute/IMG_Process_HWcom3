#include <cstdio>
#include "TIFFReader.h"
#include "Preprocessing.h"
#include "CCL.h"


static void processImage(const char* filename) {
    printf("=== %s ===\n", filename);
    Image rgb = TIFFReader::load(filename);
    if (rgb.getWidth() == 0) {
        printf("ERROR: cannot load %s\n", filename);
        return;
    }
    printf("Loaded: %d x %d\n", rgb.getWidth(), rgb.getHeight());

    Image gray   = Preprocessing::toGray(rgb);
    Image median = Preprocessing::medianBlur(gray, 11); 

    std::vector<WormHole> holes = CCL::detect(median);

    printf("WormHoles found: %d\n", (int)holes.size());
    for (int i = 0; i < (int)holes.size(); i++) {
        printf("  Hole %d : cx=%d cy=%d area=%d\n",
            i+1, holes[i].cx, holes[i].cy, holes[i].area);
    }
    printf("\n");
}

int main() {
    processImage("Data/WormHole_1H.tif");
    processImage("Data/WormHole_2H.tif");
    return 0;
}