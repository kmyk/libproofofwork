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
#define static_assert(p, msg) enum { static_assert ## __LINE__ = 1/!!(p) }

static inline __m256i leftrotate(__m256i x, unsigned c) { return _mm256_slli_epi32(x, c) | _mm256_srli_epi32(x, 32 - c); }
static inline uint32_t leftrotate1(uint32_t x, unsigned c) { return (x << c) | (x >> (32 - c)); }
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
         _mm256_add_epi32( leftrotate(a, 5),
         _mm256_add_epi32( f(*b, c, d),
         _mm256_add_epi32( w,
                           _mm256_set1_epi32(k) ))));
    *b = leftrotate(*b, 30);
}
static inline void sha1_round(__m256i a, __m256i *b, __m256i c,  __m256i d,  __m256i *e, uint32_t w, uint32_t k, __m256i (*f)(__m256i, __m256i, __m256i)) {
    *e = _mm256_add_epi32( *e,
         _mm256_add_epi32( leftrotate(a, 5),
         _mm256_add_epi32( f(*b, c, d),
                           _mm256_set1_epi32(w + k) )));
    *b = leftrotate(*b, 30);
}

uint64_t pow_sha1_count = 0; // for benchmark
bool pow_sha1_mine(uint8_t *mask, uint8_t *target, uint8_t *buffer, uint64_t *size) {
    static_assert (__BYTE_ORDER == __LITTLE_ENDIAN, "");
#ifdef _OPENMP
    int saved_omp_num_threads = -1;
    if (getenv("OMP_NUM_THREADS") == NULL) {
        saved_omp_num_threads = omp_get_max_threads();
        omp_set_num_threads(omp_get_max_threads());
    }
#endif
    const uint32_t mask_a = htobe32(((uint32_t *)mask)[0]);
    const uint32_t mask_b = htobe32(((uint32_t *)mask)[1]);
    const uint32_t mask_c = htobe32(((uint32_t *)mask)[2]);
    const uint32_t mask_d = htobe32(((uint32_t *)mask)[3]);
    const uint32_t mask_e = htobe32(((uint32_t *)mask)[4]);
    const uint32_t target_a = htobe32(((uint32_t *)target)[0]) & mask_a;
    const uint32_t target_b = htobe32(((uint32_t *)target)[1]) & mask_b;
    const uint32_t target_c = htobe32(((uint32_t *)target)[2]) & mask_c;
    const uint32_t target_d = htobe32(((uint32_t *)target)[3]) & mask_d;
    const uint32_t target_e = htobe32(((uint32_t *)target)[4]) & mask_d;
    enum { message_bytes = 55 };
    if (*size > 44) return false;
    for (int i = *size; i < 44; ++ i) buffer[i] = 0x41;
    const uint32_t x0  = htobe32(((uint32_t *)buffer)[0]);
    const uint32_t x1  = htobe32(((uint32_t *)buffer)[1]);
    const uint32_t x2  = htobe32(((uint32_t *)buffer)[2]);
    const uint32_t x3  = htobe32(((uint32_t *)buffer)[3]);
    const uint32_t x4  = htobe32(((uint32_t *)buffer)[4]);
    const uint32_t x5  = htobe32(((uint32_t *)buffer)[5]);
    const uint32_t x6  = htobe32(((uint32_t *)buffer)[6]);
    const uint32_t x7  = htobe32(((uint32_t *)buffer)[7]);
    const uint32_t x8  = htobe32(((uint32_t *)buffer)[8]);
    const uint32_t x9  = htobe32(((uint32_t *)buffer)[9]);
    const uint32_t x10 = htobe32(((uint32_t *)buffer)[10]);
    static const uint32_t x14 = 0x00000000;
    static const uint32_t x15 = message_bytes * 8;
    bool found = false;
    uint64_t cnt = 0;

#define repeat_ascii(c) for (uint8_t c = '!'; c <= '~'; ++ c)
#pragma omp parallel for shared(found) reduction(+:cnt)
    repeat_ascii (i11) { if (found) continue;
    repeat_ascii (i10) { if (found) break;
    repeat_ascii (i9 ) { if (found) break;
    repeat_ascii (i8 ) { if (found) break;
        const uint32_t x11 = i8 | ((uint32_t)i9 << 8) | ((uint32_t)i10 << 16) | ((uint32_t)i11 << 24);
    repeat_ascii (i3 ) { if (found) break;
    repeat_ascii (i2 ) { if (found) break;
    repeat_ascii (i1 ) { if (found) break;
        const uint32_t x13 = 0x80 | ((uint32_t)i1 << 8) | ((uint32_t)i2 << 16) | ((uint32_t)i3 << 24);
    repeat_ascii (i7 ) {
        cnt += ('z'-'#'+1)*(uint64_t)('~'-'!'+1)*('~'-'!'+1);
    repeat_ascii (i6 ) {
    repeat_ascii (i5 ) {
#undef repeat_ascii
        __m256i y12 = _mm256_set1_epi32((uint32_t)i7 | ((uint32_t)i6 << 8) | ((uint32_t)i5 << 16) | ('#' << 24)) + _mm256_set_epi32(7<<24,6<<24,5<<24,4<<24,3<<24,2<<24,1<<24,0<<24);
    for (uint8_t i4 = '#'; i4+7 < 'z'+1; i4 += 8, y12 += _mm256_set1_epi32(8<<24)) {
        __m256i a = _mm256_set1_epi32(sha1_a0);
        __m256i b = _mm256_set1_epi32(sha1_b0);
        __m256i c = _mm256_set1_epi32(sha1_c0);
        __m256i d = _mm256_set1_epi32(sha1_d0);
        __m256i e = _mm256_set1_epi32(sha1_e0);

        // Round 1 head
        sha1_round (a,&b,c,d,&e, x0 , sha1_k00, sha1_f00);
        sha1_round (e,&a,b,c,&d, x1 , sha1_k00, sha1_f00);
        sha1_round (d,&e,a,b,&c, x2 , sha1_k00, sha1_f00);
        sha1_round (c,&d,e,a,&b, x3 , sha1_k00, sha1_f00);
        sha1_round (b,&c,d,e,&a, x4 , sha1_k00, sha1_f00);
        sha1_round (a,&b,c,d,&e, x5 , sha1_k00, sha1_f00);
        sha1_round (e,&a,b,c,&d, x6 , sha1_k00, sha1_f00);
        sha1_round (d,&e,a,b,&c, x7 , sha1_k00, sha1_f00);
        sha1_round (c,&d,e,a,&b, x8 , sha1_k00, sha1_f00);
        sha1_round (b,&c,d,e,&a, x9 , sha1_k00, sha1_f00);
        sha1_round (a,&b,c,d,&e, x10, sha1_k00, sha1_f00);
        sha1_round (e,&a,b,c,&d, x11, sha1_k00, sha1_f00);
        sha1_roundv(d,&e,a,b,&c, y12, sha1_k00, sha1_f00);
        sha1_round (c,&d,e,a,&b, x13, sha1_k00, sha1_f00);
        sha1_round (b,&c,d,e,&a, x14, sha1_k00, sha1_f00);
        sha1_round (a,&b,c,d,&e, x15, sha1_k00, sha1_f00);
        // Round 1 tail
        const uint32_t x16 = leftrotate1( x13 ^ x8  ^ x2 ^ x0, 1 );
        const uint32_t x17 = leftrotate1( x14 ^ x9  ^ x3 ^ x1, 1 );
        const uint32_t x18 = leftrotate1( x15 ^ x10 ^ x4 ^ x2, 1 );
        const uint32_t x19 = leftrotate1( x16 ^ x11 ^ x5 ^ x3, 1 );
        sha1_round (e,&a,b,c,&d, x16, sha1_k00, sha1_f00);
        sha1_round (d,&e,a,b,&c, x17, sha1_k00, sha1_f00);
        sha1_round (c,&d,e,a,&b, x18, sha1_k00, sha1_f00);
        sha1_round (b,&c,d,e,&a, x19, sha1_k00, sha1_f00);

        // Round 2
        const __m256i  y20 = leftrotate ( y12 ^ _mm256_set1_epi32(x17 ^ x6 ^ x4), 1 );
        const uint32_t x21 = leftrotate1( x18 ^ x13 ^ x7 ^ x5, 1 );
        const uint32_t x22 = leftrotate1( x19 ^ x14 ^ x8 ^ x6, 1 );
        const __m256i  y23 = leftrotate ( y20 ^ _mm256_set1_epi32(x15 ^ x9 ^ x7), 1 );
        const uint32_t x24 = leftrotate1( x21 ^ x16 ^ x10 ^ x8, 1 );
        const uint32_t x25 = leftrotate1( x22 ^ x17 ^ x11 ^ x9, 1 );
        const __m256i  y26 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y23, y12), _mm256_set1_epi32(x18 ^ x10)), 1 );
        const uint32_t x27 = leftrotate1( x24 ^ x19 ^ x13 ^ x11, 1 );
        const __m256i  y28 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y20, y12), _mm256_set1_epi32(x25 ^ x14)), 1 );
        const __m256i  y29 = leftrotate ( _mm256_xor_si256(y26, _mm256_set1_epi32(x21 ^ x15 ^ x13)), 1 );
        const uint32_t x30 = leftrotate1( x27 ^ x22 ^ x16 ^ x14, 1 );
        const __m256i  y31 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y28, y23), _mm256_set1_epi32(x17 ^ x15)), 1 );
        const __m256i  y32 = leftrotate ( _mm256_xor_si256(y29, _mm256_set1_epi32(x24 ^ x18 ^ x16)), 1 );
        const uint32_t x33 = leftrotate1( x30 ^ x25 ^ x19 ^ x17, 1 );
        const __m256i  y34 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y31, y26), _mm256_xor_si256(y20, _mm256_set1_epi32(x18))), 1 );
        const __m256i  y35 = leftrotate ( _mm256_xor_si256(y32, _mm256_set1_epi32(x27 ^ x21 ^ x19)), 1 );
        const __m256i  y36 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y28, y20), _mm256_set1_epi32(x33 ^ x22)), 1 );
        const __m256i  y37 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y34, y29), _mm256_xor_si256(y23, _mm256_set1_epi32(x21))), 1 );
        const __m256i  y38 = leftrotate ( _mm256_xor_si256(y35, _mm256_set1_epi32(x30 ^ x24 ^ x22)), 1 );
        const __m256i  y39 = leftrotate ( _mm256_xor_si256(_mm256_xor_si256(y36, y31), _mm256_xor_si256(y23, _mm256_set1_epi32(x25))), 1 );
        sha1_roundv(a,&b,c,d,&e, y20, sha1_k20, sha1_f20);
        sha1_round (e,&a,b,c,&d, x21, sha1_k20, sha1_f20);
        sha1_round (d,&e,a,b,&c, x22, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y23, sha1_k20, sha1_f20);
        sha1_round (b,&c,d,e,&a, x24, sha1_k20, sha1_f20);
        sha1_round (a,&b,c,d,&e, x25, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y26, sha1_k20, sha1_f20);
        sha1_round (d,&e,a,b,&c, x27, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y28, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y29, sha1_k20, sha1_f20);
        sha1_round (a,&b,c,d,&e, x30, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y31, sha1_k20, sha1_f20);
        sha1_roundv(d,&e,a,b,&c, y32, sha1_k20, sha1_f20);
        sha1_round (c,&d,e,a,&b, x33, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y34, sha1_k20, sha1_f20);
        sha1_roundv(a,&b,c,d,&e, y35, sha1_k20, sha1_f20);
        sha1_roundv(e,&a,b,c,&d, y36, sha1_k20, sha1_f20);
        sha1_roundv(d,&e,a,b,&c, y37, sha1_k20, sha1_f20);
        sha1_roundv(c,&d,e,a,&b, y38, sha1_k20, sha1_f20);
        sha1_roundv(b,&c,d,e,&a, y39, sha1_k20, sha1_f20);

        // Round 3
        const __m256i y40 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y37, y32), _mm256_xor_si256(y26, _mm256_set1_epi32(x24))), 1 );
        const __m256i y41 = leftrotate( _mm256_xor_si256(y38, _mm256_set1_epi32(x33 ^ x27 ^ x25)), 1 );
        const __m256i y42 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y39, y34), _mm256_xor_si256(y28, y26)), 1 );
        const __m256i y43 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y40, y35), _mm256_xor_si256(y29, _mm256_set1_epi32(x27))), 1 );
        const __m256i y44 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y41, y36), _mm256_xor_si256(y28, _mm256_set1_epi32(x30))), 1 );
        const __m256i y45 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y42, y37), _mm256_xor_si256(y31, y29)), 1 );
        const __m256i y46 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y43, y38), _mm256_xor_si256(y32, _mm256_set1_epi32(x30))), 1 );
        const __m256i y47 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y44, y39), _mm256_xor_si256(y31, _mm256_set1_epi32(x33))), 1 );
        const __m256i y48 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y45, y40), _mm256_xor_si256(y34, y32)), 1 );
        const __m256i y49 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y46, y41), _mm256_xor_si256(y35, _mm256_set1_epi32(x33))), 1 );
        const __m256i y50 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y47, y42), _mm256_xor_si256(y36, y34)), 1 );
        const __m256i y51 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y48, y43), _mm256_xor_si256(y37, y35)), 1 );
        const __m256i y52 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y49, y44), _mm256_xor_si256(y38, y36)), 1 );
        const __m256i y53 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y50, y45), _mm256_xor_si256(y39, y37)), 1 );
        const __m256i y54 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y51, y46), _mm256_xor_si256(y40, y38)), 1 );
        const __m256i y55 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y52, y47), _mm256_xor_si256(y41, y39)), 1 );
        const __m256i y56 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y53, y48), _mm256_xor_si256(y42, y40)), 1 );
        const __m256i y57 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y54, y49), _mm256_xor_si256(y43, y41)), 1 );
        const __m256i y58 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y55, y50), _mm256_xor_si256(y44, y42)), 1 );
        const __m256i y59 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y56, y51), _mm256_xor_si256(y45, y43)), 1 );
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

        // Round 4
        const __m256i y60 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y57, y52), _mm256_xor_si256(y46, y44)), 1 );
        const __m256i y61 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y58, y53), _mm256_xor_si256(y47, y45)), 1 );
        const __m256i y62 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y59, y54), _mm256_xor_si256(y48, y46)), 1 );
        const __m256i y63 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y60, y55), _mm256_xor_si256(y49, y47)), 1 );
        const __m256i y64 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y61, y56), _mm256_xor_si256(y50, y48)), 1 );
        const __m256i y65 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y62, y57), _mm256_xor_si256(y51, y49)), 1 );
        const __m256i y66 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y63, y58), _mm256_xor_si256(y52, y50)), 1 );
        const __m256i y67 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y64, y59), _mm256_xor_si256(y53, y51)), 1 );
        const __m256i y68 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y65, y60), _mm256_xor_si256(y54, y52)), 1 );
        const __m256i y69 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y66, y61), _mm256_xor_si256(y55, y53)), 1 );
        const __m256i y70 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y67, y62), _mm256_xor_si256(y56, y54)), 1 );
        const __m256i y71 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y68, y63), _mm256_xor_si256(y57, y55)), 1 );
        const __m256i y72 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y69, y64), _mm256_xor_si256(y58, y56)), 1 );
        const __m256i y73 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y70, y65), _mm256_xor_si256(y59, y57)), 1 );
        const __m256i y74 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y71, y66), _mm256_xor_si256(y60, y58)), 1 );
        const __m256i y75 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y72, y67), _mm256_xor_si256(y61, y59)), 1 );
        const __m256i y76 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y73, y68), _mm256_xor_si256(y62, y60)), 1 );
        const __m256i y77 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y74, y69), _mm256_xor_si256(y63, y61)), 1 );
        const __m256i y78 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y75, y70), _mm256_xor_si256(y64, y62)), 1 );
        const __m256i y79 = leftrotate( _mm256_xor_si256(_mm256_xor_si256(y76, y71), _mm256_xor_si256(y65, y63)), 1 );
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

        a = _mm256_add_epi32(a, _mm256_set1_epi32(sha1_a0));
        const __m256i cmp_a = _mm256_cmpeq_epi32(_mm256_and_si256(a, _mm256_set1_epi32(mask_a)), _mm256_set1_epi32(target_a));
        if (unlikely(not _mm256_testz_si256(cmp_a, cmp_a))) {
            b = _mm256_add_epi32(b, _mm256_set1_epi32(sha1_b0));
            c = _mm256_add_epi32(c, _mm256_set1_epi32(sha1_c0));
            d = _mm256_add_epi32(d, _mm256_set1_epi32(sha1_d0));
            e = _mm256_add_epi32(e, _mm256_set1_epi32(sha1_e0));
            const __m256i cmp_b = _mm256_cmpeq_epi32(_mm256_and_si256(b, _mm256_set1_epi32(mask_b)), _mm256_set1_epi32(target_b));
            const __m256i cmp_c = _mm256_cmpeq_epi32(_mm256_and_si256(c, _mm256_set1_epi32(mask_c)), _mm256_set1_epi32(target_c));
            const __m256i cmp_d = _mm256_cmpeq_epi32(_mm256_and_si256(d, _mm256_set1_epi32(mask_d)), _mm256_set1_epi32(target_d));
            const __m256i cmp_e = _mm256_cmpeq_epi32(_mm256_and_si256(e, _mm256_set1_epi32(mask_e)), _mm256_set1_epi32(target_e));
            const __m256i cmp_ad = _mm256_and_si256(cmp_a, cmp_d);
            const __m256i cmp_bc = _mm256_and_si256(cmp_b, cmp_c);
            const __m256i cmp_ade = _mm256_and_si256(cmp_ad, cmp_e);
            if (unlikely(not _mm256_testz_si256(cmp_ade, cmp_bc))) {
                __attribute__((__aligned__(32))) uint32_t cmp[8]; _mm256_store_si256((__m256i *)cmp, _mm256_and_si256(cmp_ade, cmp_bc));
                __attribute__((__aligned__(32))) uint32_t z12[8]; _mm256_store_si256((__m256i *)z12, y12);
                repeat (i, 8) if (not found and cmp[i]) {
#pragma omp critical
                    {
                        if (not found) {
                            found = true;
                            ((uint32_t *)buffer)[11] = be32toh(x11);
                            ((uint32_t *)buffer)[12] = be32toh(z12[i]);
                            ((uint32_t *)buffer)[13] = be32toh(x13);
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
    pow_sha1_count = cnt;
    return found;
}
