#ifndef CAST128_H
#define CAST128_H

#include "common.h"
#include "stddef.h"

#define BLOCKSIZE 8

typedef struct _Cast128 {
    u32 Km[16]; // masking keys
    byte Kr[16]; // rotating keys
} Cast128;

Cast128* Cast128_init(Cast128 *cast, byte *key);
int Cast128_encrypt(Cast128 *cast, byte *msg, size_t n, byte *ciph);
int Cast128_decrypt(Cast128 *cast, byte *ciph, size_t n, byte *msg);

#endif
