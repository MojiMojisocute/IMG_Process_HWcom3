#include <cstring>
#include <vector>
#ifdef __AVX2__
#include <immintrin.h>
#endif
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
#include "CCL.h"

static int findRoot(std::vector<int>& parent, int x) {
    while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
    return x;
}
static void unionSet(std::vector<int>& parent, int a, int b) {
    a = findRoot(parent, a); b = findRoot(parent, b);
    if (a != b) parent[b] = a;
}

Image CCL::thresholdInv(const Image& src, unsigned char t) {
    int w = src.getWidth();
    int h = src.getHeight();
    Image result(w, h, 1);
    const unsigned char* S = src.getRawPixels();
    unsigned char* D = result.getRawPixels();
    size_t total = (size_t)w * h;
    size_t i = 0;
#ifdef __AVX2__
    __m256i thresh = _mm256_set1_epi8((char)t);
    // __m256i zero   = _mm256_setzero_si256();
    __m256i ff     = _mm256_set1_epi8((char)255);
    for (; i + 32 <= total; i += 32) {
        __m256i v    = _mm256_loadu_si256((__m256i*)(S+i));
        __m256i vs   = _mm256_xor_si256(v,      _mm256_set1_epi8((char)0x80));
        __m256i ts   = _mm256_xor_si256(thresh, _mm256_set1_epi8((char)0x80));
        __m256i gt   = _mm256_cmpgt_epi8(vs, ts);   
        __m256i mask = _mm256_xor_si256(gt, ff);
        _mm256_storeu_si256((__m256i*)(D+i), _mm256_and_si256(mask, ff));
    }
#elif defined(__SSE4_1__)
    __m128i thresh = _mm_set1_epi8((char)t);
    __m128i zero   = _mm_setzero_si128();
    __m128i ff     = _mm_set1_epi8((char)255);
    for (; i + 16 <= total; i += 16) {
        __m128i v    = _mm_loadu_si128((__m128i*)(S+i));
        __m128i vs   = _mm_xor_si128(v,      _mm_set1_epi8((char)0x80));
        __m128i ts   = _mm_xor_si128(thresh, _mm_set1_epi8((char)0x80));
        __m128i gt   = _mm_cmpgt_epi8(vs, ts);
        __m128i mask = _mm_xor_si128(gt, ff);
        _mm_storeu_si128((__m128i*)(D+i), _mm_and_si128(mask, ff));
    }
#endif
    for (; i < total; i++) D[i] = (S[i] <= t) ? 255 : 0;
    return result;
}

static void erodeRow(const unsigned char* row, unsigned char* out, int w, int half) {
    for (int x = 0; x < w; x++) {
        unsigned char mn = 255;
        for (int k = -half; k <= half; k++) {
            int nx = x + k;
            if (nx < 0) nx = 0; else if (nx >= w) nx = w - 1;
            if (row[nx] < mn) mn = row[nx];
        }
        out[x] = mn;
    }
}
static void dilateRow(const unsigned char* row, unsigned char* out, int w, int half) {
    for (int x = 0; x < w; x++) {
        unsigned char mx = 0;
        for (int k = -half; k <= half; k++) {
            int nx = x + k;
            if (nx < 0) nx = 0; else if (nx >= w) nx = w - 1;
            if (row[nx] > mx) mx = row[nx];
        }
        out[x] = mx;
    }
}
static void erodeCol(const unsigned char* src, unsigned char* dst, int w, int h, int half) {
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            unsigned char mn = 255;
            for (int k = -half; k <= half; k++) {
                int ny = y + k;
                if (ny < 0) ny = 0; else if (ny >= h) ny = h - 1;
                if (src[ny*w+x] < mn) mn = src[ny*w+x];
            }
            dst[y*w+x] = mn;
        }
    }
}
static void dilateCol(const unsigned char* src, unsigned char* dst, int w, int h, int half) {
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            unsigned char mx = 0;
            for (int k = -half; k <= half; k++) {
                int ny = y + k;
                if (ny < 0) ny = 0; else if (ny >= h) ny = h - 1;
                if (src[ny*w+x] > mx) mx = src[ny*w+x];
            }
            dst[y*w+x] = mx;
        }
    }
}

Image CCL::erosion(const Image& src, int kernelSize) {
    int w = src.getWidth(), h = src.getHeight(), half = kernelSize / 2;
    Image temp(w, h, 1), result(w, h, 1);
    const unsigned char* S = src.getRawPixels();
    unsigned char* T = temp.getRawPixels();
    unsigned char* D = result.getRawPixels();
#ifdef __AVX2__
    for (int y = 0; y < h; y++) {
        const unsigned char* row = S + y*w;
        unsigned char* out = T + y*w;
        int x = half;
        for (; x <= w - half - 32; x += 32) {
            __m256i mn = _mm256_set1_epi8((char)255);
            for (int k = -half; k <= half; k++) {
                __m256i v = _mm256_loadu_si256((__m256i*)(row+x+k));
                mn = _mm256_min_epu8(mn, v);
            }
            _mm256_storeu_si256((__m256i*)(out+x), mn);
        }
        for (; x < w - half; x++) {
            unsigned char mn = 255;
            for (int k = -half; k <= half; k++) { if(row[x+k]<mn) mn=row[x+k]; }
            out[x] = mn;
        }
        for (int bx = 0; bx < half; bx++) erodeRow(row, out, half, half); 
        erodeRow(row, out, w, half);  
    }
    erodeCol(T, D, w, h, half);
#elif defined(__SSE4_1__)
    for (int y = 0; y < h; y++) {
        const unsigned char* row = S + y*w;
        unsigned char* out = T + y*w;
        int x = half;
        for (; x <= w - half - 16; x += 16) {
            __m128i mn = _mm_set1_epi8((char)255);
            for (int k = -half; k <= half; k++) {
                __m128i v = _mm_loadu_si128((__m128i*)(row+x+k));
                mn = _mm_min_epu8(mn, v);
            }
            _mm_storeu_si128((__m128i*)(out+x), mn);
        }
        for (; x < w - half; x++) {
            unsigned char mn = 255;
            for (int k = -half; k <= half; k++) { if(row[x+k]<mn) mn=row[x+k]; }
            out[x] = mn;
        }
        erodeRow(row, out, w, half);
    }
    erodeCol(T, D, w, h, half);
#else
    for (int y = 0; y < h; y++) erodeRow(S + y*w, T + y*w, w, half);
    erodeCol(T, D, w, h, half);
#endif
    return result;
}

Image CCL::dilation(const Image& src, int kernelSize) {
    int w = src.getWidth(), h = src.getHeight(), half = kernelSize / 2;
    Image temp(w, h, 1), result(w, h, 1);
    const unsigned char* S = src.getRawPixels();
    unsigned char* T = temp.getRawPixels();
    unsigned char* D = result.getRawPixels();
#ifdef __AVX2__
    for (int y = 0; y < h; y++) {
        const unsigned char* row = S + y*w;
        unsigned char* out = T + y*w;
        int x = half;
        for (; x <= w - half - 32; x += 32) {
            __m256i mx = _mm256_setzero_si256();
            for (int k = -half; k <= half; k++) {
                __m256i v = _mm256_loadu_si256((__m256i*)(row+x+k));
                mx = _mm256_max_epu8(mx, v);
            }
            _mm256_storeu_si256((__m256i*)(out+x), mx);
        }
        for (; x < w - half; x++) {
            unsigned char mx = 0;
            for (int k = -half; k <= half; k++) { if(row[x+k]>mx) mx=row[x+k]; }
            out[x] = mx;
        }
        dilateRow(row, out, w, half);
    }
    dilateCol(T, D, w, h, half);
#elif defined(__SSE4_1__)
    for (int y = 0; y < h; y++) {
        const unsigned char* row = S + y*w;
        unsigned char* out = T + y*w;
        int x = half;
        for (; x <= w - half - 16; x += 16) {
            __m128i mx = _mm_setzero_si128();
            for (int k = -half; k <= half; k++) {
                __m128i v = _mm_loadu_si128((__m128i*)(row+x+k));
                mx = _mm_max_epu8(mx, v);
            }
            _mm_storeu_si128((__m128i*)(out+x), mx);
        }
        for (; x < w - half; x++) {
            unsigned char mx = 0;
            for (int k = -half; k <= half; k++) { if(row[x+k]>mx) mx=row[x+k]; }
            out[x] = mx;
        }
        dilateRow(row, out, w, half);
    }
    dilateCol(T, D, w, h, half);
#else
    for (int y = 0; y < h; y++) dilateRow(S + y*w, T + y*w, w, half);
    dilateCol(T, D, w, h, half);
#endif
    return result;
}

Image CCL::morphOpen(const Image& src, int kernelSize, int iterations) {
    Image img = src.clone();
    for (int i = 0; i < iterations; i++) img = erosion(img, kernelSize);
    for (int i = 0; i < iterations; i++) img = dilation(img, kernelSize);
    return img;
}

Image CCL::morphClose(const Image& src, int kernelSize, int iterations) {
    Image img = src.clone();
    for (int i = 0; i < iterations; i++) img = dilation(img, kernelSize);
    for (int i = 0; i < iterations; i++) img = erosion(img, kernelSize);
    return img;
}

std::vector<int> CCL::labelComponents(const Image& binary, int& numLabels) {
    int w = binary.getWidth();
    int h = binary.getHeight();
    const unsigned char* B = binary.getRawPixels();
    size_t total = (size_t)w * h;
    std::vector<int> labels(total, 0);
    std::vector<int> parent(total + 1);
    for (size_t i = 0; i <= total; i++) parent[i] = (int)i;
    int nextLabel = 1;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (B[y*w+x] == 0) continue;
            int up   = (y > 0 && B[(y-1)*w+x]) ? labels[(y-1)*w+x] : 0;
            int left = (x > 0 && B[y*w+x-1])   ? labels[y*w+x-1]   : 0;
            if (!up && !left) { labels[y*w+x] = nextLabel++; continue; }
            if ( up && !left) { labels[y*w+x] = up;   continue; }
            if (!up &&  left) { labels[y*w+x] = left; continue; }
            int root = (up < left) ? up : left;
            labels[y*w+x] = root;
            if (up != left) unionSet(parent, up, left);
        }
    }
    std::vector<int> labelMap(nextLabel, 0);
    numLabels = 0;
    for (size_t i = 0; i < total; i++) {
        if (labels[i] == 0) continue;
        int root = findRoot(parent, labels[i]);
        if (labelMap[root] == 0) labelMap[root] = ++numLabels;
        labels[i] = labelMap[root];
    }
    return labels;
}

std::vector<WormHole> CCL::computeHoles(
    const std::vector<int>& labels, int numLabels, int w, int h)
{
    std::vector<long long> sumX(numLabels+1, 0);
    std::vector<long long> sumY(numLabels+1, 0);
    std::vector<int>       area(numLabels+1, 0);
    for (int y = 0; y < h; y++) {
        const int* row = labels.data() + y * w;
        for (int x = 0; x < w; x++) {
            int lb = row[x];
            if (lb) { sumX[lb] += x; sumY[lb] += y; area[lb]++; }
        }
    }
    std::vector<WormHole> holes;
    holes.reserve(numLabels);
    for (int lb = 1; lb <= numLabels; lb++) {
        if (area[lb] == 0) continue;
        WormHole wh;
        wh.cx   = (int)(sumX[lb] / area[lb]);
        wh.cy   = (int)(sumY[lb] / area[lb]);
        wh.area = area[lb];
        holes.push_back(wh);
    }
    return holes;
}

std::vector<WormHole> CCL::filter(
    const std::vector<WormHole>& holes, int minArea, int maxArea)
{
    std::vector<WormHole> result;
    result.reserve(holes.size());
    for (size_t i = 0; i < holes.size(); i++)
        if (holes[i].area > minArea && holes[i].area < maxArea)
            result.push_back(holes[i]);
    return result;
}

std::vector<WormHole> CCL::detect(const Image& src) {
    Image binary  = thresholdInv(src, THRESHOLD);
    Image opened  = morphOpen (binary, MORPH_KERNEL_SIZE, OPEN_ITERATIONS);
    Image cleaned = morphClose(opened, MORPH_KERNEL_SIZE, CLOSE_ITERATIONS);

    int numLabels = 0;
    std::vector<int> labels = labelComponents(cleaned, numLabels);
    std::vector<WormHole> holes = computeHoles(labels, numLabels,
                                               src.getWidth(), src.getHeight());
    return filter(holes, MIN_AREA, MAX_AREA);
}