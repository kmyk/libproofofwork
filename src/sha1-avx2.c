#define _BSD_SOURCE
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <string.h>
#include "proofofwork-private.h"

static inline __m256i sha1_f00(__m256i b, __m256i c, __m256i d) { return (b & c) | (~ b & d); }
static inline __m256i sha1_f20(__m256i b, __m256i c, __m256i d) { return b ^ c ^ d; }
static inline __m256i sha1_f40(__m256i b, __m256i c, __m256i d) { return (b & c) | (c & d) | (d & b); }
static inline __m256i sha1_f60(__m256i b, __m256i c, __m256i d) { return b ^ c ^ d; }
static const uint32_t sha1_k00 = 0x5a827999;
static const uint32_t sha1_k20 = 0x6ed9eba1;
static const uint32_t sha1_k40 = 0x8f1bbcdc;
static const uint32_t sha1_k60 = 0xca62c1d6;
static const uint32_t sha1_a0 = 0x67452301;
static const uint32_t sha1_b0 = 0xefcdab89;
static const uint32_t sha1_c0 = 0x98badcfe;
static const uint32_t sha1_d0 = 0x10325476;
static const uint32_t sha1_e0 = 0xc3d2e1f0;

static inline void sha1_roundv(__m256i a, __m256i *b, __m256i c,  __m256i d,  __m256i *e, __m256i w, uint32_t k, __m256i (*f)(__m256i, __m256i, __m256i)) {
    *e = _mm256_add_epi32( *e,
         _mm256_add_epi32( mm256_rol_epi32(a, 5),
         _mm256_add_epi32( f(*b, c, d),
         _mm256_add_epi32( w,
                           _mm256_set1_epi32(k) ))));
    *b = mm256_rol_epi32(*b, 30);
}
static inline void sha1_round(__m256i a, __m256i *b, __m256i c,  __m256i d,  __m256i *e, uint32_t w, uint32_t k, __m256i (*f)(__m256i, __m256i, __m256i)) {
    *e = _mm256_add_epi32( *e,
         _mm256_add_epi32( mm256_rol_epi32(a, 5),
         _mm256_add_epi32( f(*b, c, d),
                           _mm256_set1_epi32(w + k) )));
    *b = mm256_rol_epi32(*b, 30);
}

uint64_t pow_sha1_count = 0; // for benchmark
bool pow_sha1_mine(uint8_t const *mask, uint8_t const *target, uint8_t *buffer, uint64_t size, int32_t const *indices) {
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
    if (size > pow_sha1_block_length - sizeof(uint64_t) / CHAR_BIT - 1) return false;

    // load hash
    const uint32_t mask_a = be32toh(((uint32_t *)mask)[0]);
    const uint32_t mask_b = be32toh(((uint32_t *)mask)[1]);
    const uint32_t mask_c = be32toh(((uint32_t *)mask)[2]);
    const uint32_t mask_d = be32toh(((uint32_t *)mask)[3]);
    const uint32_t mask_e = be32toh(((uint32_t *)mask)[4]);
    const uint32_t target_a = be32toh(((uint32_t *)target)[0]) & mask_a;
    const uint32_t target_b = be32toh(((uint32_t *)target)[1]) & mask_b;
    const uint32_t target_c = be32toh(((uint32_t *)target)[2]) & mask_c;
    const uint32_t target_d = be32toh(((uint32_t *)target)[3]) & mask_d;
    const uint32_t target_e = be32toh(((uint32_t *)target)[4]) & mask_e;

    // load text
    uint8_t local[pow_sha1_block_length];
    memcpy(local, buffer, pow_sha1_block_length);
    local[size] = '\x80';
    for (int i = size+1; i < pow_sha1_block_length - sizeof(uint64_t) / CHAR_BIT; ++ i) local[i] = '\0';
    static const uint32_t x14 = 0x00000000;
    static_assert (x14 == 0, "unused (optimized out)");
    const uint32_t x15 = size * 8;

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
        padded_alphabet[i] = be32toh(c << (index0 % 4 * CHAR_BIT));
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
        __m256i y0  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[0 ]));
        __m256i y1  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[1 ]));
        __m256i y2  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[2 ]));
        __m256i y3  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[3 ]));
        __m256i y4  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[4 ]));
        __m256i y5  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[5 ]));
        __m256i y6  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[6 ]));
        __m256i y7  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[7 ]));
        __m256i y8  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[8 ]));
        __m256i y9  = _mm256_set1_epi32(be32toh(((uint32_t *)local)[9 ]));
        __m256i y10 = _mm256_set1_epi32(be32toh(((uint32_t *)local)[10]));
        __m256i y11 = _mm256_set1_epi32(be32toh(((uint32_t *)local)[11]));
        __m256i y12 = _mm256_set1_epi32(be32toh(((uint32_t *)local)[12]));
        __m256i y13 = _mm256_set1_epi32(be32toh(((uint32_t *)local)[13]));
        const __m256i y15 = _mm256_set1_epi32(x15);
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
        __m256i a = _mm256_set1_epi32(sha1_a0);
        __m256i b = _mm256_set1_epi32(sha1_b0);
        __m256i c = _mm256_set1_epi32(sha1_c0);
        __m256i d = _mm256_set1_epi32(sha1_d0);
        __m256i e = _mm256_set1_epi32(sha1_e0);

        // round [0, 16)
        sha1_roundv(a,&b,c,d,&e, y0 , sha1_k00, sha1_f00);
        sha1_roundv(e,&a,b,c,&d, y1 , sha1_k00, sha1_f00);
        sha1_roundv(d,&e,a,b,&c, y2 , sha1_k00, sha1_f00);
        sha1_roundv(c,&d,e,a,&b, y3 , sha1_k00, sha1_f00);
        sha1_roundv(b,&c,d,e,&a, y4 , sha1_k00, sha1_f00);
        sha1_roundv(a,&b,c,d,&e, y5 , sha1_k00, sha1_f00);
        sha1_roundv(e,&a,b,c,&d, y6 , sha1_k00, sha1_f00);
        sha1_roundv(d,&e,a,b,&c, y7 , sha1_k00, sha1_f00);
        sha1_roundv(c,&d,e,a,&b, y8 , sha1_k00, sha1_f00);
        sha1_roundv(b,&c,d,e,&a, y9 , sha1_k00, sha1_f00);
        sha1_roundv(a,&b,c,d,&e, y10, sha1_k00, sha1_f00);
        sha1_roundv(e,&a,b,c,&d, y11, sha1_k00, sha1_f00);
        sha1_roundv(d,&e,a,b,&c, y12, sha1_k00, sha1_f00);
        sha1_roundv(c,&d,e,a,&b, y13, sha1_k00, sha1_f00);
        sha1_round (b,&c,d,e,&a,   0, sha1_k00, sha1_f00);
        sha1_round (a,&b,c,d,&e, x15, sha1_k00, sha1_f00);

        // round [16, 20)
        const __m256i y16 = mm256_rol_epi32( y13 ^ y8  ^ y2 ^ y0, 1 );
        const __m256i y17 = mm256_rol_epi32(       y9  ^ y3 ^ y1, 1 );
        const __m256i y18 = mm256_rol_epi32( y15 ^ y10 ^ y4 ^ y2, 1 );
        const __m256i y19 = mm256_rol_epi32( y16 ^ y11 ^ y5 ^ y3, 1 );
        sha1_roundv(e,&a,b,c,&d, y16, sha1_k00, sha1_f00);
        sha1_roundv(d,&e,a,b,&c, y17, sha1_k00, sha1_f00);
        sha1_roundv(c,&d,e,a,&b, y18, sha1_k00, sha1_f00);
        sha1_roundv(b,&c,d,e,&a, y19, sha1_k00, sha1_f00);

        // round [20, 40)
        const __m256i y20 = mm256_rol_epi32( y17 ^ y12 ^ y6  ^ y4 , 1 );
        const __m256i y21 = mm256_rol_epi32( y18 ^ y13 ^ y7  ^ y5 , 1 );
        const __m256i y22 = mm256_rol_epi32( y19       ^ y8  ^ y6 , 1 );
        const __m256i y23 = mm256_rol_epi32( y20 ^ y15 ^ y9  ^ y7 , 1 );
        const __m256i y24 = mm256_rol_epi32( y21 ^ y16 ^ y10 ^ y8 , 1 );
        const __m256i y25 = mm256_rol_epi32( y22 ^ y17 ^ y11 ^ y9 , 1 );
        const __m256i y26 = mm256_rol_epi32( y23 ^ y18 ^ y12 ^ y10, 1 );
        const __m256i y27 = mm256_rol_epi32( y24 ^ y19 ^ y13 ^ y11, 1 );
        const __m256i y28 = mm256_rol_epi32( y25 ^ y20       ^ y12, 1 );
        const __m256i y29 = mm256_rol_epi32( y26 ^ y21 ^ y15 ^ y13, 1 );
        const __m256i y30 = mm256_rol_epi32( y27 ^ y22 ^ y16      , 1 );
        const __m256i y31 = mm256_rol_epi32( y28 ^ y23 ^ y17 ^ y15, 1 );
        const __m256i y32 = mm256_rol_epi32( y29 ^ y24 ^ y18 ^ y16, 1 );
        const __m256i y33 = mm256_rol_epi32( y30 ^ y25 ^ y19 ^ y17, 1 );
        const __m256i y34 = mm256_rol_epi32( y31 ^ y26 ^ y20 ^ y18, 1 );
        const __m256i y35 = mm256_rol_epi32( y32 ^ y27 ^ y21 ^ y19, 1 );
        const __m256i y36 = mm256_rol_epi32( y33 ^ y28 ^ y22 ^ y20, 1 );
        const __m256i y37 = mm256_rol_epi32( y34 ^ y29 ^ y23 ^ y21, 1 );
        const __m256i y38 = mm256_rol_epi32( y35 ^ y30 ^ y24 ^ y22, 1 );
        const __m256i y39 = mm256_rol_epi32( y36 ^ y31 ^ y25 ^ y23, 1 );
        sha1_roundv(a,&b,c,d,&e, y20, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y21, sha1_k20, sha1_f20);
        sha1_roundv(d,&e,a,b,&c, y22, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y23, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y24, sha1_k20, sha1_f20);
        sha1_roundv(a,&b,c,d,&e, y25, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y26, sha1_k20, sha1_f20);
        sha1_roundv(d,&e,a,b,&c, y27, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y28, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y29, sha1_k20, sha1_f20);
        sha1_roundv(a,&b,c,d,&e, y30, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y31, sha1_k20, sha1_f20);
        sha1_roundv(d,&e,a,b,&c, y32, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y33, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y34, sha1_k20, sha1_f20);
        sha1_roundv(a,&b,c,d,&e, y35, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y36, sha1_k20, sha1_f20);
        sha1_roundv(d,&e,a,b,&c, y37, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y38, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y39, sha1_k20, sha1_f20);

        // round [40, 60)
        const __m256i y40 = mm256_rol_epi32( y37 ^ y32 ^ y26 ^ y24, 1 );
        const __m256i y41 = mm256_rol_epi32( y38 ^ y33 ^ y27 ^ y25, 1 );
        const __m256i y42 = mm256_rol_epi32( y39 ^ y34 ^ y28 ^ y26, 1 );
        const __m256i y43 = mm256_rol_epi32( y40 ^ y35 ^ y29 ^ y27, 1 );
        const __m256i y44 = mm256_rol_epi32( y41 ^ y36 ^ y30 ^ y28, 1 );
        const __m256i y45 = mm256_rol_epi32( y42 ^ y37 ^ y31 ^ y29, 1 );
        const __m256i y46 = mm256_rol_epi32( y43 ^ y38 ^ y32 ^ y30, 1 );
        const __m256i y47 = mm256_rol_epi32( y44 ^ y39 ^ y33 ^ y31, 1 );
        const __m256i y48 = mm256_rol_epi32( y45 ^ y40 ^ y34 ^ y32, 1 );
        const __m256i y49 = mm256_rol_epi32( y46 ^ y41 ^ y35 ^ y33, 1 );
        const __m256i y50 = mm256_rol_epi32( y47 ^ y42 ^ y36 ^ y34, 1 );
        const __m256i y51 = mm256_rol_epi32( y48 ^ y43 ^ y37 ^ y35, 1 );
        const __m256i y52 = mm256_rol_epi32( y49 ^ y44 ^ y38 ^ y36, 1 );
        const __m256i y53 = mm256_rol_epi32( y50 ^ y45 ^ y39 ^ y37, 1 );
        const __m256i y54 = mm256_rol_epi32( y51 ^ y46 ^ y40 ^ y38, 1 );
        const __m256i y55 = mm256_rol_epi32( y52 ^ y47 ^ y41 ^ y39, 1 );
        const __m256i y56 = mm256_rol_epi32( y53 ^ y48 ^ y42 ^ y40, 1 );
        const __m256i y57 = mm256_rol_epi32( y54 ^ y49 ^ y43 ^ y41, 1 );
        const __m256i y58 = mm256_rol_epi32( y55 ^ y50 ^ y44 ^ y42, 1 );
        const __m256i y59 = mm256_rol_epi32( y56 ^ y51 ^ y45 ^ y43, 1 );
        sha1_roundv(a,&b,c,d,&e, y40, sha1_k40, sha1_f40);
        sha1_roundv(e,&a,b,c,&d, y41, sha1_k40, sha1_f40);
        sha1_roundv(d,&e,a,b,&c, y42, sha1_k40, sha1_f40);
        sha1_roundv(c,&d,e,a,&b, y43, sha1_k40, sha1_f40);
        sha1_roundv(b,&c,d,e,&a, y44, sha1_k40, sha1_f40);
        sha1_roundv(a,&b,c,d,&e, y45, sha1_k40, sha1_f40);
        sha1_roundv(e,&a,b,c,&d, y46, sha1_k40, sha1_f40);
        sha1_roundv(d,&e,a,b,&c, y47, sha1_k40, sha1_f40);
        sha1_roundv(c,&d,e,a,&b, y48, sha1_k40, sha1_f40);
        sha1_roundv(b,&c,d,e,&a, y49, sha1_k40, sha1_f40);
        sha1_roundv(a,&b,c,d,&e, y50, sha1_k40, sha1_f40);
        sha1_roundv(e,&a,b,c,&d, y51, sha1_k40, sha1_f40);
        sha1_roundv(d,&e,a,b,&c, y52, sha1_k40, sha1_f40);
        sha1_roundv(c,&d,e,a,&b, y53, sha1_k40, sha1_f40);
        sha1_roundv(b,&c,d,e,&a, y54, sha1_k40, sha1_f40);
        sha1_roundv(a,&b,c,d,&e, y55, sha1_k40, sha1_f40);
        sha1_roundv(e,&a,b,c,&d, y56, sha1_k40, sha1_f40);
        sha1_roundv(d,&e,a,b,&c, y57, sha1_k40, sha1_f40);
        sha1_roundv(c,&d,e,a,&b, y58, sha1_k40, sha1_f40);
        sha1_roundv(b,&c,d,e,&a, y59, sha1_k40, sha1_f40);

        // round [60, 80)
        const __m256i y60 = mm256_rol_epi32( y57 ^ y52 ^ y46 ^ y44, 1 );
        const __m256i y61 = mm256_rol_epi32( y58 ^ y53 ^ y47 ^ y45, 1 );
        const __m256i y62 = mm256_rol_epi32( y59 ^ y54 ^ y48 ^ y46, 1 );
        const __m256i y63 = mm256_rol_epi32( y60 ^ y55 ^ y49 ^ y47, 1 );
        const __m256i y64 = mm256_rol_epi32( y61 ^ y56 ^ y50 ^ y48, 1 );
        const __m256i y65 = mm256_rol_epi32( y62 ^ y57 ^ y51 ^ y49, 1 );
        const __m256i y66 = mm256_rol_epi32( y63 ^ y58 ^ y52 ^ y50, 1 );
        const __m256i y67 = mm256_rol_epi32( y64 ^ y59 ^ y53 ^ y51, 1 );
        const __m256i y68 = mm256_rol_epi32( y65 ^ y60 ^ y54 ^ y52, 1 );
        const __m256i y69 = mm256_rol_epi32( y66 ^ y61 ^ y55 ^ y53, 1 );
        const __m256i y70 = mm256_rol_epi32( y67 ^ y62 ^ y56 ^ y54, 1 );
        const __m256i y71 = mm256_rol_epi32( y68 ^ y63 ^ y57 ^ y55, 1 );
        const __m256i y72 = mm256_rol_epi32( y69 ^ y64 ^ y58 ^ y56, 1 );
        const __m256i y73 = mm256_rol_epi32( y70 ^ y65 ^ y59 ^ y57, 1 );
        const __m256i y74 = mm256_rol_epi32( y71 ^ y66 ^ y60 ^ y58, 1 );
        const __m256i y75 = mm256_rol_epi32( y72 ^ y67 ^ y61 ^ y59, 1 );
        const __m256i y76 = mm256_rol_epi32( y73 ^ y68 ^ y62 ^ y60, 1 );
        const __m256i y77 = mm256_rol_epi32( y74 ^ y69 ^ y63 ^ y61, 1 );
        const __m256i y78 = mm256_rol_epi32( y75 ^ y70 ^ y64 ^ y62, 1 );
        const __m256i y79 = mm256_rol_epi32( y76 ^ y71 ^ y65 ^ y63, 1 );
        sha1_roundv(a,&b,c,d,&e, y60, sha1_k60, sha1_f60);
        sha1_roundv(e,&a,b,c,&d, y61, sha1_k60, sha1_f60);
        sha1_roundv(d,&e,a,b,&c, y62, sha1_k60, sha1_f60);
        sha1_roundv(c,&d,e,a,&b, y63, sha1_k60, sha1_f60);
        sha1_roundv(b,&c,d,e,&a, y64, sha1_k60, sha1_f60);
        sha1_roundv(a,&b,c,d,&e, y65, sha1_k60, sha1_f60);
        sha1_roundv(e,&a,b,c,&d, y66, sha1_k60, sha1_f60);
        sha1_roundv(d,&e,a,b,&c, y67, sha1_k60, sha1_f60);
        sha1_roundv(c,&d,e,a,&b, y68, sha1_k60, sha1_f60);
        sha1_roundv(b,&c,d,e,&a, y69, sha1_k60, sha1_f60);
        sha1_roundv(a,&b,c,d,&e, y70, sha1_k60, sha1_f60);
        sha1_roundv(e,&a,b,c,&d, y71, sha1_k60, sha1_f60);
        sha1_roundv(d,&e,a,b,&c, y72, sha1_k60, sha1_f60);
        sha1_roundv(c,&d,e,a,&b, y73, sha1_k60, sha1_f60);
        sha1_roundv(b,&c,d,e,&a, y74, sha1_k60, sha1_f60);
        sha1_roundv(a,&b,c,d,&e, y75, sha1_k60, sha1_f60);
        sha1_roundv(e,&a,b,c,&d, y76, sha1_k60, sha1_f60);
        sha1_roundv(d,&e,a,b,&c, y77, sha1_k60, sha1_f60);
        sha1_roundv(c,&d,e,a,&b, y78, sha1_k60, sha1_f60);
        sha1_roundv(b,&c,d,e,&a, y79, sha1_k60, sha1_f60);

        // compare result
        a = _mm256_add_epi32(a, _mm256_set1_epi32(sha1_a0));
        const __m256i cmp_a = _mm256_cmpeq_epi32(a & _mm256_set1_epi32(mask_a), _mm256_set1_epi32(target_a));
        if (unlikely(not _mm256_testz_si256(cmp_a, cmp_a))) {
            b = _mm256_add_epi32(b, _mm256_set1_epi32(sha1_b0));
            c = _mm256_add_epi32(c, _mm256_set1_epi32(sha1_c0));
            d = _mm256_add_epi32(d, _mm256_set1_epi32(sha1_d0));
            e = _mm256_add_epi32(e, _mm256_set1_epi32(sha1_e0));
            const __m256i cmp_b = _mm256_cmpeq_epi32(b & _mm256_set1_epi32(mask_b), _mm256_set1_epi32(target_b));
            const __m256i cmp_c = _mm256_cmpeq_epi32(c & _mm256_set1_epi32(mask_c), _mm256_set1_epi32(target_c));
            const __m256i cmp_d = _mm256_cmpeq_epi32(d & _mm256_set1_epi32(mask_d), _mm256_set1_epi32(target_d));
            const __m256i cmp_e = _mm256_cmpeq_epi32(e & _mm256_set1_epi32(mask_e), _mm256_set1_epi32(target_e));
            const __m256i cmp_ad = cmp_a & cmp_d;
            const __m256i cmp_bc = cmp_b & cmp_c;
            const __m256i cmp_ade = cmp_ad & cmp_e;
            if (unlikely(not _mm256_testz_si256(cmp_ade, cmp_bc))) {
                uint32_t cmp[vector_width]; _mm256_storeu_si256((__m256i *)cmp, cmp_ade & cmp_bc);
                repeat (i, vector_width) if (not found and cmp[i]) {
#pragma omp critical
                    {
                        if (not found) {
                            found = true;
                            memcpy(buffer, local, pow_sha1_block_length);
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
    pow_sha1_count = cnt;
    return found;
}
