#include <stdio.h>
#include <string.h>
#include "utils.h"

static u32 uint32_max = ~0;

// make uint32 (BE) from first 4 bytes pointed to p
uint32_t u32_from_bytes(byte *p) {
    return (((u32)p[0]) << 24) |
        (((u32)p[1]) << 16) |
        (((u32)p[2]) << 8) |
        ((u32)p[3]);
}

// store bytes of uint32 into z (BE)
// last byte of zq is stored at z[0], etc.
void u32_to_bytes_be(u32 zq, byte *z) {
    z[0] = ((byte*)&zq)[3];
    z[1] = ((byte*)&zq)[2];
    z[2] = ((byte*)&zq)[1];
    z[3] = ((byte*)&zq)[0];
}

// left rotate (circular shift) x by n bits
uint32_t leftrot(uint32_t x, uint32_t n) {
    // only makes sense to rotate by 32 max
    // rotate by 32 == unchanged
    n %= 32;
    if (n == 0)
        return x;
    return (x << n) | (x >> (32-n));
}

// computes x mod 2**32
// instead of storing 2**32 in uint64_t
// we can use uint32_t which can have max value of 2**32 - 1
// and hack around with it
uint32_t mod2_32(uint32_t x) {
    // x == 2**32 - 1
    if (x + 1 == 0)
        return x;
    return x % uint32_max;
}

void printbytes(byte *p, u64 n) {
    for (int i = 0; i < n; i++)
        printf("%02x ", p[i]);
    puts("");
}

// byte to a single hex char
static char btoh(byte b) {
    // b must be in range [0,15]
    if (b > 15) return 0;
    else if (b >= 10) return b + 97 - 10;
    else return b + 48;
}

static char* byte_to_hex(char *dst, byte b) {
    byte bl = b >> 4;
    byte br = b & 0x0f;

    dst[0] = btoh(bl);
    dst[1] = btoh(br);
}

// dst must be large enough to hold 2n chars
char* strhexdump(char *dst, byte *src, size_t n) {
    for (int i = 0; i < n; i++) {
        byte_to_hex(dst+(i*2), src[i]);
    }
    dst[n*2] = '\0';
    return dst;
}

// single hex char to byte
static byte htob(char h) {
    if (h >= 48 && h <= 57) // [0,9]
        return h - 48;
    else if (h >= 97 && h <= 102) // [a,f]
        return h - 97 + 10;
    else if (h >= 65 && h <= 70) // [A,F]
        return h - 65 + 10;
    return 0;
}

// convert a hexstring (big-endian) to bytes
// dst must be able to hold strlen(src)/2 bytes
byte* hexstr_to_bytes(byte *dst, const char *src) {
    size_t n = strlen(src);
    for (size_t i = 0; i < n; i+=2) {
        dst[i/2] = (htob(src[i]) * 16) +  htob(src[i+1]);
    }
    return dst;
}
