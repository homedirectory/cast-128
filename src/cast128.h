#ifndef CAST128_H
#define CAST128_H

#include <stdint.h>

uint64_t enc_block(uint64_t plain, uint64_t *key);

#endif
