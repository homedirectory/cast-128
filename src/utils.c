#include <stdio.h>
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
        printf("%0x ", p[i]);
    puts("");
}
