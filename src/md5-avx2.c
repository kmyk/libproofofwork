#define _BSD_SOURCE
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <string.h>
#include "proofofwork-private.h"

static inline __m256i md5_f(__m256i x, __m256i y, __m256i z) { return (x & y) | (~ x & z); }
static inline __m256i md5_g(__m256i x, __m256i y, __m256i z) { return (x & z) | (y & ~ z); }
static inline __m256i md5_h(__m256i x, __m256i y, __m256i z) { return x ^ y ^ z; }
static inline __m256i md5_i(__m256i x, __m256i y, __m256i z) { return y ^ (x | ~ z); }
static inline void md5_roundv(__m256i *a, __m256i b, __m256i c, __m256i d, __m256i x, unsigned s, uint32_t t, __m256i (*f)(__m256i, __m256i, __m256i)) {
    __m256i acc = _mm256_add_epi32(_mm256_add_epi32(*a, f(b, c, d)), _mm256_add_epi32(x, _mm256_set1_epi32(t)));
    *a = _mm256_add_epi32(b, mm256_rol_epi32(acc, s));
}
static inline void md5_round(__m256i *a, __m256i b, __m256i c, __m256i d, uint32_t x, unsigned s, uint32_t t, __m256i (*f)(__m256i, __m256i, __m256i)) {
    __m256i acc = _mm256_add_epi32(_mm256_add_epi32(*a, f(b, c, d)), _mm256_set1_epi32(x + t));
    *a = _mm256_add_epi32(b, mm256_rol_epi32(acc, s));
}
static inline __m256i md5_cmp(__m256i x, uint32_t x0, uint32_t mask, uint32_t target) {
    return _mm256_cmpeq_epi32(_mm256_add_epi32(x, _mm256_set1_epi32(x0)) & _mm256_set1_epi32(mask), _mm256_set1_epi32(target));
}

static const uint32_t md5_a0 = 0x67452301;
static const uint32_t md5_b0 = 0xefcdab89;
static const uint32_t md5_c0 = 0x98badcfe;
static const uint32_t md5_d0 = 0x10325476;

uint64_t pow_md5_count = 0; // for benchmark
bool pow_md5_mine(uint8_t const *mask, uint8_t const *target, uint8_t *buffer, uint64_t size, int32_t const *indices) {
    // check arguments
    static_assert (__BYTE_ORDER == __LITTLE_ENDIAN, "");
    if (mask    == NULL) return false;
    if (target  == NULL) return false;
    if (buffer  == NULL) return false;
    if (indices == NULL) return false;
    for (int i = 0; i < pow_indices_length; ++ i) {
        if (indices[i] < -1 or (int64_t)size <= indices[i]) return false;
    }
    if (indices[0] == -1) return false;
    if (size > pow_md5_block_length - sizeof(uint64_t) / CHAR_BIT - 1) return false;

    // load hash
    const uint32_t mask_a = ((uint32_t *)mask)[0];
    const uint32_t mask_b = ((uint32_t *)mask)[1];
    const uint32_t mask_c = ((uint32_t *)mask)[2];
    const uint32_t mask_d = ((uint32_t *)mask)[3];
    const uint32_t target_a = ((uint32_t *)target)[0] & mask_a;
    const uint32_t target_b = ((uint32_t *)target)[1] & mask_b;
    const uint32_t target_c = ((uint32_t *)target)[2] & mask_c;
    const uint32_t target_d = ((uint32_t *)target)[3] & mask_d;

    // load text
    uint8_t local[pow_md5_block_length];
    memcpy(local, buffer, pow_md5_block_length);
    local[size] = '\x80';
    for (int i = size+1; i < pow_md5_block_length - sizeof(uint64_t) / CHAR_BIT; ++ i) local[i] = '\0';
    ((uint32_t *)local)[14] = size * CHAR_BIT;
    const uint32_t x14 = size * CHAR_BIT;
    static const uint32_t x15 = 0;

    // load indices and alphabet to modify the text
    const int index0 = indices[0];
    const int index1 = indices[1];
    const int index2 = indices[2];
    const int index3 = indices[3];
    const int index4 = indices[4];
    const int index5 = indices[5];
    const int index6 = indices[6];
    const int index7 = indices[7];
    static_assert (pow_indices_length == 8, "");
    repeat (i,pow_indices_length) {
        if (indices[i] != -1) {
            local[indices[i]] = 0;
        }
    }
    uint32_t *padded_alphabet = malloc(alphabet_size * sizeof(uint32_t));
    repeat (i,alphabet_size) {
        uint32_t c = alphabet[i];
        if (i - vector_width >= 0) c ^= alphabet[i - vector_width];
        padded_alphabet[i] = c << (index0 % 4 * CHAR_BIT);
    }

    // search
    bool found = false;
    uint64_t cnt = 0;
#pragma omp parallel for shared(found) firstprivate(local) reduction(+:cnt)
    repeat (i1, alphabet_size) { if (index1 != -1) local[index1] = alphabet[i1]; if (found or (index1 == -1 and i1 != 0)) continue;
    repeat (i7, alphabet_size) { if (index7 != -1) local[index7] = alphabet[i7];
    repeat (i6, alphabet_size) { if (index6 != -1) local[index6] = alphabet[i6];
    repeat (i5, alphabet_size) { if (index5 != -1) local[index5] = alphabet[i5];
    repeat (i4, alphabet_size) { if (index4 != -1) local[index4] = alphabet[i4];
    repeat (i3, alphabet_size) { if (index3 != -1) local[index3] = alphabet[i3];
        cnt += alphabet_size * (alphabet_size / vector_width * vector_width);
    repeat (i2, alphabet_size) { if (index2 != -1) local[index2] = alphabet[i2];
        __m256i y0  = _mm256_set1_epi32(((uint32_t *)local)[0 ]);
        __m256i y1  = _mm256_set1_epi32(((uint32_t *)local)[1 ]);
        __m256i y2  = _mm256_set1_epi32(((uint32_t *)local)[2 ]);
        __m256i y3  = _mm256_set1_epi32(((uint32_t *)local)[3 ]);
        __m256i y4  = _mm256_set1_epi32(((uint32_t *)local)[4 ]);
        __m256i y5  = _mm256_set1_epi32(((uint32_t *)local)[5 ]);
        __m256i y6  = _mm256_set1_epi32(((uint32_t *)local)[6 ]);
        __m256i y7  = _mm256_set1_epi32(((uint32_t *)local)[7 ]);
        __m256i y8  = _mm256_set1_epi32(((uint32_t *)local)[8 ]);
        __m256i y9  = _mm256_set1_epi32(((uint32_t *)local)[9 ]);
        __m256i y10 = _mm256_set1_epi32(((uint32_t *)local)[10]);
        __m256i y11 = _mm256_set1_epi32(((uint32_t *)local)[11]);
        __m256i y12 = _mm256_set1_epi32(((uint32_t *)local)[12]);
        __m256i y13 = _mm256_set1_epi32(((uint32_t *)local)[13]);
    for (int i0 = 0; i0 + vector_width - 1 < alphabet_size; i0 += vector_width) {
        // set last byte
        switch (index0 / 4) {
            case 0 : y0  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 1 : y1  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 2 : y2  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 3 : y3  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 4 : y4  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 5 : y5  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 6 : y6  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 7 : y7  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 8 : y8  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 9 : y9  ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 10: y10 ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 11: y11 ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 12: y12 ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
            case 13: y13 ^= _mm256_loadu_si256((__m256i *)(padded_alphabet + i0)); break;
        }

        // initialize vector
        __m256i a = _mm256_set1_epi32(md5_a0);
        __m256i b = _mm256_set1_epi32(md5_b0);
        __m256i c = _mm256_set1_epi32(md5_c0);
        __m256i d = _mm256_set1_epi32(md5_d0);

        // round [0, 16)
        md5_roundv(&a,b,c,d, y0 ,  7, 0xd76aa478, md5_f);
        md5_roundv(&d,a,b,c, y1 , 12, 0xe8c7b756, md5_f);
        md5_roundv(&c,d,a,b, y2 , 17, 0x242070db, md5_f);
        md5_roundv(&b,c,d,a, y3 , 22, 0xc1bdceee, md5_f);
        md5_roundv(&a,b,c,d, y4 ,  7, 0xf57c0faf, md5_f);
        md5_roundv(&d,a,b,c, y5 , 12, 0x4787c62a, md5_f);
        md5_roundv(&c,d,a,b, y6 , 17, 0xa8304613, md5_f);
        md5_roundv(&b,c,d,a, y7 , 22, 0xfd469501, md5_f);
        md5_roundv(&a,b,c,d, y8 ,  7, 0x698098d8, md5_f);
        md5_roundv(&d,a,b,c, y9 , 12, 0x8b44f7af, md5_f);
        md5_roundv(&c,d,a,b, y10, 17, 0xffff5bb1, md5_f);
        md5_roundv(&b,c,d,a, y11, 22, 0x895cd7be, md5_f);
        md5_roundv(&a,b,c,d, y12,  7, 0x6b901122, md5_f);
        md5_roundv(&d,a,b,c, y13, 12, 0xfd987193, md5_f);
        md5_round (&c,d,a,b, x14, 17, 0xa679438e, md5_f);
        md5_round (&b,c,d,a, x15, 22, 0x49b40821, md5_f);

        // round [17, 32)
        md5_roundv(&a,b,c,d, y1 ,  5, 0xf61e2562, md5_g);
        md5_roundv(&d,a,b,c, y6 ,  9, 0xc040b340, md5_g);
        md5_roundv(&c,d,a,b, y11, 14, 0x265e5a51, md5_g);
        md5_roundv(&b,c,d,a, y0 , 20, 0xe9b6c7aa, md5_g);
        md5_roundv(&a,b,c,d, y5 ,  5, 0xd62f105d, md5_g);
        md5_roundv(&d,a,b,c, y10,  9, 0x02441453, md5_g);
        md5_round (&c,d,a,b, x15, 14, 0xd8a1e681, md5_g);
        md5_roundv(&b,c,d,a, y4 , 20, 0xe7d3fbc8, md5_g);
        md5_roundv(&a,b,c,d, y9 ,  5, 0x21e1cde6, md5_g);
        md5_round (&d,a,b,c, x14,  9, 0xc33707d6, md5_g);
        md5_roundv(&c,d,a,b, y3 , 14, 0xf4d50d87, md5_g);
        md5_roundv(&b,c,d,a, y8 , 20, 0x455a14ed, md5_g);
        md5_roundv(&a,b,c,d, y13,  5, 0xa9e3e905, md5_g);
        md5_roundv(&d,a,b,c, y2 ,  9, 0xfcefa3f8, md5_g);
        md5_roundv(&c,d,a,b, y7 , 14, 0x676f02d9, md5_g);
        md5_roundv(&b,c,d,a, y12, 20, 0x8d2a4c8a, md5_g);

        // round [33, 48)
        md5_roundv(&a,b,c,d, y5 ,  4, 0xfffa3942, md5_h);
        md5_roundv(&d,a,b,c, y8 , 11, 0x8771f681, md5_h);
        md5_roundv(&c,d,a,b, y11, 16, 0x6d9d6122, md5_h);
        md5_round (&b,c,d,a, x14, 23, 0xfde5380c, md5_h);
        md5_roundv(&a,b,c,d, y1 ,  4, 0xa4beea44, md5_h);
        md5_roundv(&d,a,b,c, y4 , 11, 0x4bdecfa9, md5_h);
        md5_roundv(&c,d,a,b, y7 , 16, 0xf6bb4b60, md5_h);
        md5_roundv(&b,c,d,a, y10, 23, 0xbebfbc70, md5_h);
        md5_roundv(&a,b,c,d, y13,  4, 0x289b7ec6, md5_h);
        md5_roundv(&d,a,b,c, y0 , 11, 0xeaa127fa, md5_h);
        md5_roundv(&c,d,a,b, y3 , 16, 0xd4ef3085, md5_h);
        md5_roundv(&b,c,d,a, y6 , 23, 0x04881d05, md5_h);
        md5_roundv(&a,b,c,d, y9 ,  4, 0xd9d4d039, md5_h);
        md5_roundv(&d,a,b,c, y12, 11, 0xe6db99e5, md5_h);
        md5_round (&c,d,a,b, x15, 16, 0x1fa27cf8, md5_h);
        md5_roundv(&b,c,d,a, y2 , 23, 0xc4ac5665, md5_h);

        // round [48, 61)
        md5_roundv(&a,b,c,d, y0 ,  6, 0xf4292244, md5_i);
        md5_roundv(&d,a,b,c, y7 , 10, 0x432aff97, md5_i);
        md5_round (&c,d,a,b, x14, 15, 0xab9423a7, md5_i);
        md5_roundv(&b,c,d,a, y5 , 21, 0xfc93a039, md5_i);
        md5_roundv(&a,b,c,d, y12,  6, 0x655b59c3, md5_i);
        md5_roundv(&d,a,b,c, y3 , 10, 0x8f0ccc92, md5_i);
        md5_roundv(&c,d,a,b, y10, 15, 0xffeff47d, md5_i);
        md5_roundv(&b,c,d,a, y1 , 21, 0x85845dd1, md5_i);
        md5_roundv(&a,b,c,d, y8 ,  6, 0x6fa87e4f, md5_i);
        md5_round (&d,a,b,c, x15, 10, 0xfe2ce6e0, md5_i);
        md5_roundv(&c,d,a,b, y6 , 15, 0xa3014314, md5_i);
        md5_roundv(&b,c,d,a, y13, 21, 0x4e0811a1, md5_i);
        md5_roundv(&a,b,c,d, y4 ,  6, 0xf7537e82, md5_i);

        // compare result
        const __m256i cmp_a = md5_cmp(a, md5_a0, mask_a, target_a); // specialize the first 4 byte
        if (unlikely(not _mm256_testz_si256(cmp_a, cmp_a))) {
            md5_roundv(&d,a,b,c, y11, 10, 0xbd3af235, md5_i);
            md5_roundv(&c,d,a,b, y2 , 15, 0x2ad7d2bb, md5_i);
            md5_roundv(&b,c,d,a, y9 , 21, 0xeb86d391, md5_i);
            const __m256i cmp_d = md5_cmp(d, md5_d0, mask_d, target_d);
            const __m256i cmp_c = md5_cmp(c, md5_c0, mask_c, target_c);
            const __m256i cmp_b = md5_cmp(b, md5_b0, mask_b, target_b);
            const __m256i cmp_ad = cmp_a & cmp_d;
            const __m256i cmp_bc = cmp_b & cmp_c;
            if (unlikely(not _mm256_testz_si256(cmp_ad, cmp_bc))) {
                uint32_t cmp[vector_width]; _mm256_storeu_si256((__m256i *)cmp, cmp_ad & cmp_bc);
                repeat (i, vector_width) if (not found and cmp[i]) {
#pragma omp critical
                    {
                        if (not found) {
                            found = true;
                            memcpy(buffer, local, pow_md5_block_length);
                            buffer[index0] = alphabet[i0 + i];
                            buffer[size] = 0;
                        }
                    }
                }
            }
        }

    // break
    } if (index2 == -1 or found) break;
    } if (index3 == -1 or found) break;
    } if (index4 == -1 or found) break;
    } if (index5 == -1 or found) break;
    } if (index6 == -1 or found) break;
    } if (index7 == -1 or found) break;
    }
    }

    // leave
    free(padded_alphabet);
    pow_md5_count = cnt;
    return found;
}
