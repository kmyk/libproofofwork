#include <stdio.h>
#include <string.h>
#include "proofofwork.h"

extern uint64_t pow_sha1_count;
int main(int argc, char **argv) {
    uint8_t mask[pow_sha1_digest_length] = { 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t target[pow_sha1_digest_length] = { 0xde, 0xad, 0xbe, 0xaf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t buffer[pow_sha1_block_length] = { 'f', 'l', 'a', 'g', '{', 0, 0, 0, 0, 0, 0, 0, 0, '}' };
    int32_t indices[pow_indices_length] = { 5,6,7,8,9,10,11,12 };
    uint64_t size = 14;
    pow_set_num_threads(0);
    pow_sha1_mine(mask, target, buffer, size, indices);
    printf("sha1\n");
    printf("deadbeaf\n");
    printf("%s\n", buffer);
    printf("%lu\n", pow_sha1_count);
    return 0;
}
