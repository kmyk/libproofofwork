#ifndef PROOFOFWORK_H
#define PROOFOFWORK_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @param [in,out] size       sholud be a multiple of the vector width (8 for AVX2)
 * @return                    true if succeeded
 */
bool pow_set_alphabet(uint8_t const *alphabet, uint64_t size);
bool pow_get_alphabet(uint8_t *alphabet, uint64_t *size);

/**
 * Set the number of threads, if possible.
 * If you have no OMP_NUM_THREADS variable, you should use this before calling functions.
 * @param [in]                use all threads if 0
 * @return                    true if succeeded
 */
bool pow_set_num_threads(int n);
/**
 * @return                    0 if failed
 */
int pow_get_num_threads();

enum { pow_indices_length = 8 };
enum { pow_md5_block_length = 64 };
enum { pow_md5_digest_length = 16 };
/**
 * @param [in]     mask       the length must be pow_md5_digest_length.
 * @param [in]     target     the length must be pow_md5_digest_length.
 * @param [in,out] buffer     the length must be at most pow_md5_block_length.
 * @param [in]     size
 * @param [in]     indices    indices of buffer to modify. the length must be pow_indices.
 * @return                    whether a text is found or not.
 */
bool pow_md5_mine(uint8_t const *mask, uint8_t const *target, uint8_t *buffer, uint64_t size, int32_t const *indices);

enum { pow_sha1_block_length = 64 };
enum { pow_sha1_digest_length = 20 };
bool pow_sha1_mine(uint8_t const *mask, uint8_t const *target, uint8_t *buffer, uint64_t size, int32_t const *indices);

enum { pow_sha256_block_length = 64 };
enum { pow_sha256_digest_length = 32 };
bool pow_sha256_mine(uint8_t const *mask, uint8_t const *target, uint8_t *buffer, uint64_t size, int32_t const *indices);

#endif
