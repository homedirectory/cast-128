#include "utils.h"

uint32_t mk32(uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4) {
    return (((uint32_t)x1) << 24) |
        (((uint32_t)x2) << 16) |
        (((uint32_t)x3) << 8) |
        x4;
}
