#include <stdio.h>
#include <string.h>
#include "../src/cast128.h"
#include "../src/common.h"
#include "../src/utils.h"

int test_Cast128_encrypt(Cast128 *cast, byte *msg, size_t n, byte *exp_ciph) {
    puts("=== Testing encryption ===");

    printf("Message: ");
    printbytes(msg, n);

    byte ciph[n];
    Cast128_encrypt(cast, msg, n, ciph);

    if (memcmp(ciph, exp_ciph, n)) {
        puts("FAILURE! Incorrect ciphertext.");
        printf("Expected: ");
        printbytes(exp_ciph, n);
        printf("Got     : ");
        printbytes(ciph, n);
        return -1;
    }
    else {
        puts("SUCCESS");
        return 0;
    }
}

int test_Cast128_decrypt(Cast128 *cast, byte *ciph, size_t n, byte *exp_msg) {
    puts("=== Testing decryption ===");

    printf("Ciphertext: ");
    printbytes(ciph, n);

    byte msg[n];
    Cast128_decrypt(cast, ciph, n, msg);

    if (memcmp(msg, exp_msg, n)) {
        puts("FAILURE! Incorrect messsage.");
        printf("Expected: ");
        printbytes(exp_msg, n);
        printf("Got     : ");
        printbytes(msg, n);
        return -1;
    }
    else {
        puts("SUCCESS");
        return 0;
    }
}

int main(int argc, char **argv) {
    byte key[16] = { 0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78, 
                     0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A };
    byte plaintext[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    byte ciphertext[8] = { 0x23, 0x8B, 0x4F, 0xE5, 0x84, 0x7E, 0x44, 0xB2 };

    Cast128 cast;
    Cast128_init(&cast, key); 

    printf("Key: ");
    printbytes(key, 8);

    test_Cast128_encrypt(&cast, plaintext, sizeof(plaintext), ciphertext);
    test_Cast128_decrypt(&cast, ciphertext, sizeof(ciphertext), plaintext);
}
