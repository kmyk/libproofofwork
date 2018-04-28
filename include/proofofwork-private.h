#ifndef PROOFOFWORK_PRIVATE_H
#define PROOFOFWORK_PRIVATE_H
#include "proofofwork.h"

#if defined(__linux) || defined(linux)
#include <endian.h>
#elif defined(__APPLE__) || defined(__DARWIN__)
#include <libkern/OSByteOrder.h>
#if defined(__LITTLE_ENDIAN__)
#define __LITTLE_ENDIAN 1
#define __BYTE_ORDER __LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN__)
#define __BYTE_ORDER __BIG_ENDIAN__
#else
#error "unknown byte order."
#endif
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#else
#error "unsupported OS"
#endif

#include <limits.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <immintrin.h>
enum { vector_width = 8 };
enum { vector_align = 32 };
static inline __m256i mm256_rol_epi32(__m256i x, unsigned c) { return _mm256_slli_epi32(x, c) | _mm256_srli_epi32(x, 32 - c); }

#define unlikely(x) __builtin_expect(!!(x), 0)
#define repeat(i,n) for (int i = 0; (i) < (int)n; ++(i))
#define concat_(a, b) a ## b
#define concat(a, b) concat_(a, b)
#define static_assert(p, msg) enum { concat(static_assert, __COUNTER__) = 1/!!(p) }

extern uint8_t alphabet[256];
extern size_t alphabet_size;

// for benchmark
extern uint64_t pow_md5_count;
extern uint64_t pow_sha1_count;
extern uint64_t pow_sha256_count;

#endif
