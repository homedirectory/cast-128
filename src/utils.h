#pragma once

#include <stddef.h>
#include "common.h"

uint32_t u32_from_bytes(byte *p);

void u32_to_bytes_be(u32 zq, byte *z);

uint32_t leftrot(uint32_t x, uint32_t n);

uint32_t mod2_32(uint32_t x);

void printbytes(byte *p, u64 n);

char* strhexdump(char *dst, byte *src, size_t n);

byte* hexstr_to_bytes(byte *dst, const char *src);
