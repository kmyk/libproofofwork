#define _BSD_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>
#include <iso646.h>
#include <stdbool.h>
#include <endian.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#define unlikely(x) __builtin_expect(!!(x), 0)
#define repeat(i,n) for (int i = 0; (i) < (int)n; ++(i))

static inline __m256i leftrotate(__m256i x, unsigned c) { return _mm256_slli_epi32(x, c) | _mm256_srli_epi32(x, 32 - c); }
static inline __m256i md5_f(__m256i x, __m256i y, __m256i z) { return (x & y) | (~ x & z); }
static inline __m256i md5_g(__m256i x, __m256i y, __m256i z) { return (x & z) | (y & ~ z); }
static inline __m256i md5_h(__m256i x, __m256i y, __m256i z) { return x ^ y ^ z; }
static inline __m256i md5_i(__m256i x, __m256i y, __m256i z) { return y ^ (x | ~ z); }
static inline void md5_roundv(__m256i *a, __m256i b, __m256i c, __m256i d, __m256i x, unsigned s, uint32_t t, __m256i (*f)(__m256i, __m256i, __m256i)) {
    __m256i acc = _mm256_add_epi32(_mm256_add_epi32(*a, f(b, c, d)), _mm256_add_epi32(x, _mm256_set1_epi32(t)));
    *a = _mm256_add_epi32(b, leftrotate(acc, s));
}
static inline void md5_round(__m256i *a, __m256i b, __m256i c, __m256i d, uint32_t x, unsigned s, uint32_t t, __m256i (*f)(__m256i, __m256i, __m256i)) {
    __m256i acc = _mm256_add_epi32(_mm256_add_epi32(*a, f(b, c, d)), _mm256_set1_epi32(x + t));
    *a = _mm256_add_epi32(b, leftrotate(acc, s));
}
static inline __m256i md5_cmp(__m256i x, uint32_t x0, uint32_t mask, uint32_t target) {
    return _mm256_cmpeq_epi32(_mm256_and_si256(_mm256_add_epi32(x, _mm256_set1_epi32(x0)), _mm256_set1_epi32(mask)), _mm256_set1_epi32(target));
}
static inline uint32_t leftrotate1(uint32_t x, unsigned c) { return (x << c) | (x >> (32 - c)); }
static inline uint32_t md5_f1(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~ x & z); }
static inline void md5_round1(uint32_t *a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, unsigned s, uint32_t t, uint32_t (*f)(uint32_t, uint32_t, uint32_t)) {
    *a = b + leftrotate1(*a + f(b, c, d) + x + t, s);
}
static const uint32_t md5_a0 = le32toh(0x67452301);
static const uint32_t md5_b0 = le32toh(0xefcdab89);
static const uint32_t md5_c0 = le32toh(0x98badcfe);
static const uint32_t md5_d0 = le32toh(0x10325476);
__attribute__((__aligned__(32))) static const uint32_t indices[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

uint64_t pow_md5_count = 0; // for benchmark
bool pow_md5_mine(uint8_t *mask, uint8_t *target, uint8_t *buffer, uint64_t *size) {
#ifdef _OPENMP
    int saved_omp_num_threads = -1;
    if (getenv("OMP_NUM_THREADS") == NULL) {
        saved_omp_num_threads = omp_get_max_threads();
        omp_set_num_threads(omp_get_max_threads());
    }
#endif
    const uint32_t mask_a = ((uint32_t *)mask)[0];
    const uint32_t mask_b = ((uint32_t *)mask)[1];
    const uint32_t mask_c = ((uint32_t *)mask)[2];
    const uint32_t mask_d = ((uint32_t *)mask)[3];
    const uint32_t target_a = ((uint32_t *)target)[0] & mask_a;
    const uint32_t target_b = ((uint32_t *)target)[1] & mask_b;
    const uint32_t target_c = ((uint32_t *)target)[2] & mask_c;
    const uint32_t target_d = ((uint32_t *)target)[3] & mask_d;
    enum { message_bytes = 55 };
    if (*size > 44) return false;
    for (int i = *size; i < 44; ++ i) buffer[i] = 0x41;
    const uint32_t x0  = ((uint32_t *)buffer)[0];
    const uint32_t x1  = ((uint32_t *)buffer)[1];
    const uint32_t x2  = ((uint32_t *)buffer)[2];
    const uint32_t x3  = ((uint32_t *)buffer)[3];
    const uint32_t x4  = ((uint32_t *)buffer)[4];
    const uint32_t x5  = ((uint32_t *)buffer)[5];
    const uint32_t x6  = ((uint32_t *)buffer)[6];
    const uint32_t x7  = ((uint32_t *)buffer)[7];
    const uint32_t x8  = ((uint32_t *)buffer)[8];
    const uint32_t x9  = ((uint32_t *)buffer)[9];
    const uint32_t x10 = ((uint32_t *)buffer)[10];
    static const uint32_t x14 = le32toh(message_bytes * 8);
    static const uint32_t x15 = 0x00000000;
    bool found = false;
    uint32_t a1 = md5_a0;
    uint32_t b1 = md5_b0;
    uint32_t c1 = md5_c0;
    uint32_t d1 = md5_d0;
    md5_round1(&a1,b1,c1,d1, x0 ,  7, le32toh(0xd76aa478), md5_f1);
    md5_round1(&d1,a1,b1,c1, x1 , 12, le32toh(0xe8c7b756), md5_f1);
    md5_round1(&c1,d1,a1,b1, x2 , 17, le32toh(0x242070db), md5_f1);
    md5_round1(&b1,c1,d1,a1, x3 , 22, le32toh(0xc1bdceee), md5_f1);
    md5_round1(&a1,b1,c1,d1, x4 ,  7, le32toh(0xf57c0faf), md5_f1);
    md5_round1(&d1,a1,b1,c1, x5 , 12, le32toh(0x4787c62a), md5_f1);
    md5_round1(&c1,d1,a1,b1, x6 , 17, le32toh(0xa8304613), md5_f1);
    md5_round1(&b1,c1,d1,a1, x7 , 22, le32toh(0xfd469501), md5_f1);
    md5_round1(&a1,b1,c1,d1, x8 ,  7, le32toh(0x698098d8), md5_f1);
    md5_round1(&d1,a1,b1,c1, x9 , 12, le32toh(0x8b44f7af), md5_f1);
    md5_round1(&c1,d1,a1,b1, x10, 17, le32toh(0xffff5bb1), md5_f1);
    uint64_t cnt = 0;
#define repeat_ascii(c) for (uint8_t c = '!'; c <= '~'; ++ c)
#pragma omp parallel for shared(found) reduction(+:cnt)
    repeat_ascii (i11) { if (found) continue;
    repeat_ascii (i10) { if (found) break;
    repeat_ascii (i9 ) { if (found) break;
    repeat_ascii (i8 ) { if (found) break;
        const __m256i y11 = _mm256_set1_epi32(le32toh(i11 | ((uint32_t)i10 << 8) | ((uint32_t)i9 << 16) | ((uint32_t)i8 << 24)));
    repeat_ascii (i7 ) { if (found) break;
    repeat_ascii (i6 ) { if (found) break;
    repeat_ascii (i5 ) { if (found) break;
    repeat_ascii (i4 ) {
        const __m256i y12 = _mm256_set1_epi32(le32toh(i7 | ((uint32_t)i6 << 8) | ((uint32_t)i5 << 16) | ((uint32_t)i4 << 24)));
        cnt += ('z'-'#'+1)*(uint64_t)('~'-'!'+1)*('~'-'!'+1);
    repeat_ascii (i1 ) {
    repeat_ascii (i2 ) {
#undef repeat_ascii
        __m256i y13 = _mm256_set1_epi32(le32toh('#' | ((uint32_t)i2 << 8) | ((uint32_t)i1 << 16) | (0x80 << 24))) + _mm256_load_si256((__m256i const *)indices);
    for (uint8_t i3 = '#'; i3+7 < 'z'+1; i3 += 8, y13 += _mm256_set1_epi32(8)) {
        __m256i a = _mm256_set1_epi32(a1);
        __m256i b = _mm256_set1_epi32(b1);
        __m256i c = _mm256_set1_epi32(c1);
        __m256i d = _mm256_set1_epi32(d1);
        md5_roundv(&b,c,d,a, y11, 22, le32toh(0x895cd7be), md5_f);
        md5_roundv(&a,b,c,d, y12,  7, le32toh(0x6b901122), md5_f);
        md5_roundv(&d,a,b,c, y13, 12, le32toh(0xfd987193), md5_f);
        md5_round (&c,d,a,b, x14, 17, le32toh(0xa679438e), md5_f);
        md5_round (&b,c,d,a, x15, 22, le32toh(0x49b40821), md5_f);
        md5_round (&a,b,c,d, x1 ,  5, le32toh(0xf61e2562), md5_g);
        md5_round (&d,a,b,c, x6 ,  9, le32toh(0xc040b340), md5_g);
        md5_roundv(&c,d,a,b, y11, 14, le32toh(0x265e5a51), md5_g);
        md5_round (&b,c,d,a, x0 , 20, le32toh(0xe9b6c7aa), md5_g);
        md5_round (&a,b,c,d, x5 ,  5, le32toh(0xd62f105d), md5_g);
        md5_round (&d,a,b,c, x10,  9, le32toh(0x02441453), md5_g);
        md5_round (&c,d,a,b, x15, 14, le32toh(0xd8a1e681), md5_g);
        md5_round (&b,c,d,a, x4 , 20, le32toh(0xe7d3fbc8), md5_g);
        md5_round (&a,b,c,d, x9 ,  5, le32toh(0x21e1cde6), md5_g);
        md5_round (&d,a,b,c, x14,  9, le32toh(0xc33707d6), md5_g);
        md5_round (&c,d,a,b, x3 , 14, le32toh(0xf4d50d87), md5_g);
        md5_round (&b,c,d,a, x8 , 20, le32toh(0x455a14ed), md5_g);
        md5_roundv(&a,b,c,d, y13,  5, le32toh(0xa9e3e905), md5_g);
        md5_round (&d,a,b,c, x2 ,  9, le32toh(0xfcefa3f8), md5_g);
        md5_round (&c,d,a,b, x7 , 14, le32toh(0x676f02d9), md5_g);
        md5_roundv(&b,c,d,a, y12, 20, le32toh(0x8d2a4c8a), md5_g);
        md5_round (&a,b,c,d, x5 ,  4, le32toh(0xfffa3942), md5_h);
        md5_round (&d,a,b,c, x8 , 11, le32toh(0x8771f681), md5_h);
        md5_roundv(&c,d,a,b, y11, 16, le32toh(0x6d9d6122), md5_h);
        md5_round (&b,c,d,a, x14, 23, le32toh(0xfde5380c), md5_h);
        md5_round (&a,b,c,d, x1 ,  4, le32toh(0xa4beea44), md5_h);
        md5_round (&d,a,b,c, x4 , 11, le32toh(0x4bdecfa9), md5_h);
        md5_round (&c,d,a,b, x7 , 16, le32toh(0xf6bb4b60), md5_h);
        md5_round (&b,c,d,a, x10, 23, le32toh(0xbebfbc70), md5_h);
        md5_roundv(&a,b,c,d, y13,  4, le32toh(0x289b7ec6), md5_h);
        md5_round (&d,a,b,c, x0 , 11, le32toh(0xeaa127fa), md5_h);
        md5_round (&c,d,a,b, x3 , 16, le32toh(0xd4ef3085), md5_h);
        md5_round (&b,c,d,a, x6 , 23, le32toh(0x04881d05), md5_h);
        md5_round (&a,b,c,d, x9 ,  4, le32toh(0xd9d4d039), md5_h);
        md5_roundv(&d,a,b,c, y12, 11, le32toh(0xe6db99e5), md5_h);
        md5_round (&c,d,a,b, x15, 16, le32toh(0x1fa27cf8), md5_h);
        md5_round (&b,c,d,a, x2 , 23, le32toh(0xc4ac5665), md5_h);
        md5_round (&a,b,c,d, x0 ,  6, le32toh(0xf4292244), md5_i);
        md5_round (&d,a,b,c, x7 , 10, le32toh(0x432aff97), md5_i);
        md5_round (&c,d,a,b, x14, 15, le32toh(0xab9423a7), md5_i);
        md5_round (&b,c,d,a, x5 , 21, le32toh(0xfc93a039), md5_i);
        md5_roundv(&a,b,c,d, y12,  6, le32toh(0x655b59c3), md5_i);
        md5_round (&d,a,b,c, x3 , 10, le32toh(0x8f0ccc92), md5_i);
        md5_round (&c,d,a,b, x10, 15, le32toh(0xffeff47d), md5_i);
        md5_round (&b,c,d,a, x1 , 21, le32toh(0x85845dd1), md5_i);
        md5_round (&a,b,c,d, x8 ,  6, le32toh(0x6fa87e4f), md5_i);
        md5_round (&d,a,b,c, x15, 10, le32toh(0xfe2ce6e0), md5_i);
        md5_round (&c,d,a,b, x6 , 15, le32toh(0xa3014314), md5_i);
        md5_roundv(&b,c,d,a, y13, 21, le32toh(0x4e0811a1), md5_i);
        md5_round (&a,b,c,d, x4 ,  6, le32toh(0xf7537e82), md5_i);
        const __m256i cmp_a = md5_cmp(a, md5_a0, mask_a, target_a); // specialize the first 4 byte
        if (unlikely(not _mm256_testz_si256(cmp_a, cmp_a))) {
            md5_roundv(&d,a,b,c, y11, 10, le32toh(0xbd3af235), md5_i);
            md5_round (&c,d,a,b, x2 , 15, le32toh(0x2ad7d2bb), md5_i);
            md5_round (&b,c,d,a, x9 , 21, le32toh(0xeb86d391), md5_i);
            const __m256i cmp_d = md5_cmp(d, md5_d0, mask_d, target_d);
            const __m256i cmp_c = md5_cmp(c, md5_c0, mask_c, target_c);
            const __m256i cmp_b = md5_cmp(b, md5_b0, mask_b, target_b);
            const __m256i cmp_ad = _mm256_and_si256(cmp_a, cmp_d);
            const __m256i cmp_bc = _mm256_and_si256(cmp_b, cmp_c);
            if (unlikely(not _mm256_testz_si256(cmp_ad, cmp_bc))) {
                __attribute__((__aligned__(32))) uint32_t cmp[8]; _mm256_store_si256((__m256i *)cmp, _mm256_and_si256(cmp_ad, cmp_bc));
                __attribute__((__aligned__(32))) uint32_t z11[8]; _mm256_store_si256((__m256i *)z11, y11);
                __attribute__((__aligned__(32))) uint32_t z12[8]; _mm256_store_si256((__m256i *)z12, y12);
                __attribute__((__aligned__(32))) uint32_t z13[8]; _mm256_store_si256((__m256i *)z13, y13);
                repeat (i, 8) if (not found and cmp[i]) {
#pragma omp critical
                    {
                        if (not found) {
                            found = true;
                            uint32_t *x = (uint32_t *)buffer;
                            x[ 0] = htole32(x0);
                            x[ 1] = htole32(x1);
                            x[ 2] = htole32(x2);
                            x[ 3] = htole32(x3);
                            x[ 4] = htole32(x4);
                            x[ 5] = htole32(x5);
                            x[ 6] = htole32(x6);
                            x[ 7] = htole32(x7);
                            x[ 8] = htole32(x8);
                            x[ 9] = htole32(x9);
                            x[10] = htole32(x10);
                            x[11] = htole32(z11[i]);
                            x[12] = htole32(z12[i]);
                            x[13] = htole32(z13[i]);
                            x[14] = htole32(x14);
                            x[15] = htole32(x15);
                            buffer[message_bytes] = '\0';
                            *size = message_bytes;
                        }
                    }
                }
            }
        }
    }}}}}}}}}}}
#ifdef _OPENMP
    if (saved_omp_num_threads != -1) {
        omp_set_num_threads(saved_omp_num_threads);
    }
#endif
    pow_md5_count = cnt;
    return found;
}
