#include <stdio.h>
#include <string.h>
#include "proofofwork.h"

extern uint64_t pow_sha1_count;
int main(int argc, char **argv) {
    uint8_t mask[pow_sha1_digest_length] = { 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t target[pow_sha1_digest_length] = { 0xde, 0xad, 0xbe, 0xaf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t buffer[pow_sha1_block_length];
    uint64_t size = 15;
    memcpy(buffer, "libproofofwork:", size);
    pow_sha1_mine(mask, target, buffer, &size);
    printf("sha1\n");
    printf("deadbeaf\n");
    printf("%s\n", buffer);
    printf("%lu\n", pow_sha1_count);
    return 0;
}
