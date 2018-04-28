#define _BSD_SOURCE
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h>
#include "proofofwork-private.h"

uint64_t pow_sha256_count = 0; // for benchmark
bool pow_sha256_mine(uint8_t const *mask, uint8_t const *target, uint8_t *buffer, uint64_t size, int32_t const *indices) {
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
    if (size > pow_sha256_block_length - sizeof(uint64_t) / CHAR_BIT - 1) return false;

    // search
    bool found = false;
    uint64_t cnt = 0;
    static_assert (pow_indices_length == 8, "");
    repeat (i7, alphabet_size) { if (indices[7] != -1) buffer[indices[7]] = alphabet[i7];
    repeat (i6, alphabet_size) { if (indices[6] != -1) buffer[indices[6]] = alphabet[i6];
    repeat (i5, alphabet_size) { if (indices[5] != -1) buffer[indices[5]] = alphabet[i5];
    repeat (i4, alphabet_size) { if (indices[4] != -1) buffer[indices[4]] = alphabet[i4];
    repeat (i3, alphabet_size) { if (indices[3] != -1) buffer[indices[3]] = alphabet[i3];
    repeat (i2, alphabet_size) { if (indices[2] != -1) buffer[indices[2]] = alphabet[i2];
    repeat (i1, alphabet_size) { if (indices[1] != -1) buffer[indices[1]] = alphabet[i1];
    repeat (i0, alphabet_size) { if (indices[0] != -1) buffer[indices[0]] = alphabet[i0];
        uint8_t digest[pow_sha256_digest_length];
        SHA256(buffer, size, digest);
        ++ cnt;
        found = true;
        repeat (j, pow_sha256_digest_length) {
            if ((digest[j] & mask[j]) != (target[j] & mask[j])) {
                found = false;
                break;
            }
        }
        if (found) goto done;
    }}}}}}}}
done: ;

    // leave
    pow_sha256_count = cnt;
    return found;
}
