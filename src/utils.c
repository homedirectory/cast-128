#include "utils.h"

uint32_t mk32(uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4) {
    return (((uint32_t)x1) << 24) |
        (((uint32_t)x2) << 16) |
        (((uint32_t)x3) << 8) |
        x4;
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
