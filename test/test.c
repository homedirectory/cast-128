#include <stdio.h>
#include "../src/cast128.h"
#include "../src/common.h"
#include "../src/utils.h"

// cipher - expected cipher text
int test_enc_block(byte* key, byte *plain, byte *exp_cipher) {
    printf("Key      :");
    printbytes(key, 8);
    //for (int i = 0; i < 16; i++)
    //    printf(" %0x", key[i]);
    //printf('\n');

    printf("Plaintext:");
    printbytes(plain, 8);
    byte ciph[8];
    enc_block(plain, key, ciph);
    if (ciph != exp_cipher) {
        puts("Failure! Incorrect ciphertext.");
        printf("Expected :");
        printbytes(exp_cipher, 8);
        printf("Got      :");
        printbytes(ciph, 8);
        return -1;
    }
    else {
        puts("Success.");
        return 0;
    }

}

int main(int argc, char **argv) {
    byte plaintext[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    byte key[16] = { 0x01, 0x23, 0x45, 0x67, 0x12, 0x34, 0x56, 0x78, 
                     0x23, 0x45, 0x67, 0x89, 0x34, 0x56, 0x78, 0x9A };
    byte ciphertext[8] = { 0x23, 0x8B, 0x4F, 0xE5, 0x84, 0x7E, 0x44, 0xB2 };

    test_enc_block(key, plaintext, ciphertext);
}
