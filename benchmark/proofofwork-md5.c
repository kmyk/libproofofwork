#include <stdio.h>
#include <string.h>
#include "proofofwork.h"

extern uint64_t pow_md5_count;
int main(int argc, char **argv) {
    uint8_t mask[pow_md5_digest_length] = { 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t target[pow_md5_digest_length] = { 0xca, 0xfe, 0xba, 0xbe, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t buffer[pow_md5_block_length] = { 'f', 'l', 'a', 'g', '{', 0, 0, 0, 0, 0, 0, 0, 0, '}' };
    int32_t indices[pow_indices_length] = { 5,6,7,8,9,10,11,12 };
    uint64_t size = 14;
    pow_set_num_threads(0);
    pow_md5_mine(mask, target, buffer, size, indices);
    printf("md5\n");
    printf("cafebabe\n");
    printf("%s\n", buffer);
    printf("%lu\n", pow_md5_count);
    return 0;
}
