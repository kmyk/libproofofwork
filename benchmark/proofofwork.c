#include <stdio.h>
#include <string.h>
#include "proofofwork.h"

extern uint64_t pow_md5_count;
int main(int argc, char **argv) {
    uint8_t mask[pow_md5_digest_length] = { 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t target[pow_md5_digest_length] = { 0xca, 0xfe, 0xba, 0xbe, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t buffer[pow_md5_block_length];
    uint64_t size = 15;
    memcpy(buffer, "libproofofwork:", size);
    pow_md5_mine(mask, target, buffer, &size);
    printf("cafebabe\n");
    printf("%s\n", buffer);
    printf("%lu\n", pow_md5_count);
    return 0;
}
