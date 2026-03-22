#ifndef CCL_H
#define CCL_H

#include <vector>
#include "Image.h"
#include "WormHole.h"

struct CCLDebug {
    Image threshold;
    Image clean;
    Image filtered;
    std::vector<WormHole> holes;
};

class CCL {
public:
    static std::vector<WormHole> detect(const Image& src);
    static CCLDebug detectDebug(const Image& src);

private:
    static Image thresholdInv(const Image& src, unsigned char t);

    static Image morphOpen (const Image& src, int kernelSize, int iterations);

    static Image morphClose(const Image& src, int kernelSize, int iterations);

    static Image erosion (const Image& src, int kernelSize);
    static Image dilation(const Image& src, int kernelSize);

    static std::vector<int> labelComponents(
        const Image& binary,
        int& numLabels
    );

    static std::vector<WormHole> computeHoles(
        const std::vector<int>& labels,
        int numLabels,
        int width,
        int height
    );

    static std::vector<WormHole> filter(
        const std::vector<WormHole>& holes,
        int minArea,
        int maxArea
    );

    constexpr static unsigned char THRESHOLD   = 55;
    constexpr static int MORPH_KERNEL_SIZE     = 3;
    constexpr static int OPEN_ITERATIONS       = 2;
    constexpr static int CLOSE_ITERATIONS      = 1;
    constexpr static int MIN_AREA              = 50;
    constexpr static int MAX_AREA              = 300;
};

#endif // CCL_H