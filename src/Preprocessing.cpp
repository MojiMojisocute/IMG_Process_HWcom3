#include <cstring>
#ifdef __AVX2__
#include <immintrin.h>
#endif
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
#include "Preprocessing.h"

Image Preprocessing::toGray(const Image& src) {
    int w = src.getWidth();
    int h = src.getHeight();
    Image gray(w, h, 1);
    const unsigned char* S = src.getRawPixels();
    unsigned char* D = gray.getRawPixels();
    size_t total = (size_t)w * h;
    size_t i = 0;
#ifdef __AVX2__
    __m256i coeff_r = _mm256_set1_epi32(306);
    __m256i coeff_g = _mm256_set1_epi32(601);
    __m256i coeff_b = _mm256_set1_epi32(117);
    for (; i + 8 <= total; i += 8) {
        __m256i r = _mm256_set_epi32(S[21],S[18],S[15],S[12],S[9],S[6],S[3],S[0]);
        __m256i g = _mm256_set_epi32(S[22],S[19],S[16],S[13],S[10],S[7],S[4],S[1]);
        __m256i b = _mm256_set_epi32(S[23],S[20],S[17],S[14],S[11],S[8],S[5],S[2]);
        __m256i val = _mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(r,coeff_r),_mm256_mullo_epi32(g,coeff_g)),_mm256_mullo_epi32(b,coeff_b));
        val = _mm256_srli_epi32(val, 10);
        int buf[8];
        _mm256_storeu_si256((__m256i*)buf, val);
        for (int j = 0; j < 8; j++) D[i+j] = (unsigned char)buf[j];
        S += 24;
    }
#elif defined(__SSE4_1__)
    __m128i coeff_r = _mm_set1_epi32(306);
    __m128i coeff_g = _mm_set1_epi32(601);
    __m128i coeff_b = _mm_set1_epi32(117);
    for (; i + 4 <= total; i += 4) {
        __m128i r = _mm_set_epi32(S[9],S[6],S[3],S[0]);
        __m128i g = _mm_set_epi32(S[10],S[7],S[4],S[1]);
        __m128i b = _mm_set_epi32(S[11],S[8],S[5],S[2]);
        __m128i val = _mm_add_epi32(_mm_add_epi32(_mm_mullo_epi32(r,coeff_r),_mm_mullo_epi32(g,coeff_g)),_mm_mullo_epi32(b,coeff_b));
        val = _mm_srli_epi32(val, 10);
        int buf[4];
        _mm_storeu_si128((__m128i*)buf, val);
        for (int j = 0; j < 4; j++) D[i+j] = (unsigned char)buf[j];
        S += 12;
    }
#endif
    for (; i < total; i++) {
        D[i] = (unsigned char)((306*S[0] + 601*S[1] + 117*S[2]) >> 10);
        S += 3;
    }
    return gray;
}

Image Preprocessing::gaussianBlur(const Image& src) {
    int w = src.getWidth();
    int h = src.getHeight();
    Image temp(w, h, 1);
    Image result(w, h, 1);
    const unsigned char* S = src.getRawPixels();
    unsigned char* T = temp.getRawPixels();
    unsigned char* R = result.getRawPixels();
    static const int k1d[5] = {1,4,6,4,1};
    for (int y = 0; y < h; y++) {
        const unsigned char* row = S + y*w;
        unsigned char* out = T + y*w;
        for (int x = 0; x < 2; x++) {
            int sum = 0;
            for (int k = 0; k < 5; k++) { int nx=x+k-2; if(nx<0)nx=0; sum+=k1d[k]*row[nx]; }
            out[x] = (unsigned char)(sum >> 4);
        }
#ifdef __AVX2__
        int x = 2;
        for (; x <= w-2-16; x += 16) {
            __m256i s0 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*)(row+x-2)));
            __m256i s1 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*)(row+x-1)));
            __m256i s2 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*)(row+x  )));
            __m256i s3 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*)(row+x+1)));
            __m256i s4 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*)(row+x+2)));
            __m256i sum16 = _mm256_add_epi16(_mm256_add_epi16(_mm256_add_epi16(s0,_mm256_slli_epi16(s1,2)),_mm256_add_epi16(_mm256_add_epi16(_mm256_slli_epi16(s2,2),_mm256_slli_epi16(s2,1)),_mm256_slli_epi16(s3,2))),s4);
            sum16 = _mm256_srli_epi16(sum16, 4);
            __m128i packed = _mm_packus_epi16(_mm256_castsi256_si128(sum16),_mm256_extracti128_si256(sum16,1));
            _mm_storeu_si128((__m128i*)(out+x), packed);
        }
        for (; x < w-2; x++) {
            int sum = k1d[0]*row[x-2]+k1d[1]*row[x-1]+k1d[2]*row[x]+k1d[3]*row[x+1]+k1d[4]*row[x+2];
            out[x] = (unsigned char)(sum >> 4);
        }
#elif defined(__SSE4_1__)
        int x = 2;
        for (; x <= w-2-8; x += 8) {
            __m128i s0 = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*)(row+x-2)));
            __m128i s1 = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*)(row+x-1)));
            __m128i s2 = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*)(row+x  )));
            __m128i s3 = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*)(row+x+1)));
            __m128i s4 = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*)(row+x+2)));
            __m128i sum16 = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(s0,_mm_slli_epi16(s1,2)),_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(s2,2),_mm_slli_epi16(s2,1)),_mm_slli_epi16(s3,2))),s4);
            sum16 = _mm_srli_epi16(sum16, 4);
            __m128i packed = _mm_packus_epi16(sum16, _mm_setzero_si128());
            _mm_storel_epi64((__m128i*)(out+x), packed);
        }
        for (; x < w-2; x++) {
            int sum = k1d[0]*row[x-2]+k1d[1]*row[x-1]+k1d[2]*row[x]+k1d[3]*row[x+1]+k1d[4]*row[x+2];
            out[x] = (unsigned char)(sum >> 4);
        }
#else
        for (int x = 2; x < w-2; x++) {
            int sum = k1d[0]*row[x-2]+k1d[1]*row[x-1]+k1d[2]*row[x]+k1d[3]*row[x+1]+k1d[4]*row[x+2];
            out[x] = (unsigned char)(sum >> 4);
        }
#endif
        for (int x = w-2; x < w; x++) {
            int sum = 0;
            for (int k = 0; k < 5; k++) { int nx=x+k-2; if(nx>=w)nx=w-1; sum+=k1d[k]*row[nx]; }
            out[x] = (unsigned char)(sum >> 4);
        }
    }
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < 2; y++) {
            int sum = 0;
            for (int k = 0; k < 5; k++) { int ny=y+k-2; if(ny<0)ny=0; sum+=k1d[k]*T[ny*w+x]; }
            R[y*w+x] = (unsigned char)(sum >> 4);
        }
        for (int y = 2; y < h-2; y++) {
            const unsigned char* col = T+y*w+x;
            int sum = k1d[0]*col[-2*w]+k1d[1]*col[-w]+k1d[2]*col[0]+k1d[3]*col[w]+k1d[4]*col[2*w];
            R[y*w+x] = (unsigned char)(sum >> 4);
        }
        for (int y = h-2; y < h; y++) {
            int sum = 0;
            for (int k = 0; k < 5; k++) { int ny=y+k-2; if(ny>=h)ny=h-1; sum+=k1d[k]*T[ny*w+x]; }
            R[y*w+x] = (unsigned char)(sum >> 4);
        }
    }
    return result;
}

Image Preprocessing::medianBlur(const Image& src, int kernelSize) {
    int w = src.getWidth();
    int h = src.getHeight();
    int half = kernelSize / 2;
    int winLen = kernelSize * kernelSize;
    int medPos = winLen / 2;

    Image result(w, h, 1);
    const unsigned char* S = src.getRawPixels();
    unsigned char* D = result.getRawPixels();

    std::vector<unsigned char> win(winLen);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = 0;
            for (int ky = -half; ky <= half; ky++) {
                int ny = y + ky;
                if (ny < 0) ny = 0; else if (ny >= h) ny = h - 1;
                const unsigned char* row = S + ny * w;
                for (int kx = -half; kx <= half; kx++) {
                    int nx = x + kx;
                    if (nx < 0) nx = 0; else if (nx >= w) nx = w - 1;
                    win[idx++] = row[nx];
                }
            }
            for (int i = 0; i <= medPos; i++) {
                int minIdx = i;
                for (int j = i + 1; j < winLen; j++)
                    if (win[j] < win[minIdx]) minIdx = j;
                unsigned char tmp = win[i]; win[i] = win[minIdx]; win[minIdx] = tmp;
            }
            D[y * w + x] = win[medPos];
        }
    }
    return result;
}