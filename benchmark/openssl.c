#include <stdio.h>
#include <stdint.h>
#include <openssl/md5.h>
int main(void) {
    uint8_t s[6+1] = {};
    uint8_t digest[MD5_DIGEST_LENGTH];
    int cnt = 0;
#define repeat_ascii(c) for (c = '!'; c <= '~'; ++ c)
    repeat_ascii (s[0]) {
    repeat_ascii (s[1]) {
    repeat_ascii (s[2]) {
    repeat_ascii (s[3]) {
    repeat_ascii (s[4]) {
    repeat_ascii (s[5]) {
#undef repeat_ascii
        MD5(s, sizeof(s)-1, digest);
        ++ cnt;
        if (digest[0] == 0xc0 && digest[1] == 0xff && digest[2] == 0xee) {
            goto done;
        }
    }}}}}}
done:
    printf("c0ffee\n");
    printf("%s\n", s);
    printf("%d\n", cnt);
    return 0;
}
