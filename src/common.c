#include "proofofwork-private.h"
#include <string.h>
#include <stdlib.h>
#include <iso646.h>

uint8_t alphabet[256] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
};
size_t alphabet_size = 64;

int compare_char(const void *a, const void *b) { return *(char *)a - *(char *)b; }
bool pow_set_alphabet(uint8_t const *s, uint64_t size) {
    if (sizeof(alphabet) <= size) return false;
    memcpy(alphabet, s, size);
    qsort(alphabet, size, sizeof(char), &compare_char);
    int j = 0;
    for (int i = 0; i < size; ++ i) { // unique
        if (i == 0 or alphabet[i-1] != alphabet[i]) {
            alphabet[j ++] = alphabet[i];
        }
    }
    alphabet_size = j;
    return true;
}
bool pow_get_alphabet(uint8_t *s, uint64_t *size) {
    if (*size < alphabet_size) return false;
    memcpy(s, alphabet, alphabet_size);
    *size = alphabet_size;
    return true;
}

bool pow_set_num_threads(int n) {
#ifdef _OPENMP
    if (n == 0) n = omp_get_max_threads();
    omp_set_num_threads(n);
    return true;
#else
    return false;
#endif
}
int pow_get_num_threads() {
#ifdef _OPENMP
    return omp_get_num_threads();
#else
    return 0;
#endif
}
