#include <stdio.h>
#include "../src/cast128.h"

// cipher - expected cipher text
int test_enc_block(uint64_t* key, uint64_t plain, uint64_t exp_cipher) {
    uint64_t ciph = enc_block(plain, key);
    if (ciph != exp_cipher) {
        puts("Failure!");
        printf("Expected: %16lx\n", exp_cipher);
        printf("Got     : %16lx\n", ciph);
        return -1;
    }
    else {
        puts("Success.");
        return 0;
    }

}

int main(int argc, char **argv) {
    uint64_t key[2] = { 0x0123456712345678, 0x234567893456789a };
    test_enc_block(key, 0x0123456789abcdef, 0x238b4fe5847e44b2);
}
