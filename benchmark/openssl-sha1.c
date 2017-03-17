#include <stdio.h>
#include <stdint.h>
#include <openssl/sha.h>
int main(void) {
    uint8_t s[6+1] = {};
    uint8_t digest[SHA_DIGEST_LENGTH];
    int cnt = 0;
#define repeat_ascii(c) for (c = '!'; c <= '~'; ++ c)
    repeat_ascii (s[0]) {
    repeat_ascii (s[1]) {
    repeat_ascii (s[2]) {
    repeat_ascii (s[3]) {
    repeat_ascii (s[4]) {
    repeat_ascii (s[5]) {
#undef repeat_ascii
        SHA1(s, sizeof(s)-1, digest);
        ++ cnt;
        if (digest[0] == 0xab && digest[1] == 0xcd && digest[2] == 0xef) {
            goto done;
        }
    }}}}}}
done:
    printf("sha1\n");
    printf("abcdef\n");
    printf("%s\n", s);
    printf("%d\n", cnt);
    return 0;
}
