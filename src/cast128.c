#include <stdio.h>
#include <string.h>
#include "cast128.h"
#include "common.h"
#include "utils.h"
#include "sboxes.h"

#define TAKE_5BITS(x) (((byte)x) & right5_mask8)

// some constants
static byte right5_mask8 = ((byte) ~0) >> 3;
static uint64_t left_mask64 = ((uint64_t) ~0) << 32;
static uint64_t right_mask64 = ((uint64_t) ~0) >> 32;

static void quart(byte *dst, byte *src, u32 s1, u32 s2, u32 s3, u32 s4, u32 s5) {
    u32 src_q = u32_from_bytes(src);
    u32 dst_q = src_q ^ s1 ^ s2 ^ s3 ^ s4 ^ s5;
    u32_to_bytes_be(dst_q, dst);
}

Cast128* Cast128_init(Cast128 *cast, byte *key) {
    // 1. 16 pairs of subkeys
        // a pair consists of:
        // * 32-bit masking key
        // * 5-bit rotation key
    u32 *Km = cast->Km;
    byte *Kr = cast->Kr;

    // 2.4 Key Schedule
    // The subkeys are formed from the key x0x1x2x3x4x5x6x7x8x9xAxBxCxDxExF
    byte x[16];
    byte *x0 = &(x[0]), *x1 = &(x[1]), *x2 = &(x[2]), *x3 = &(x[3]),
         *x4 = &(x[4]), *x5 = &(x[5]), *x6 = &(x[6]), *x7 = &(x[7]),
         *x8 = &(x[8]), *x9 = &(x[9]), *xA = &(x[10]), *xB = &(x[11]),
         *xC = &(x[12]), *xD = &(x[13]), *xE = &(x[14]), *xF = &(x[15]);
    memcpy(x, key, 16);

    for(int i = 0; i < 16; i++)
        printf("x%X=%x\n", i, x[i]);

    // temporary bytes
    byte z[16];
    byte *z0 = &(z[0]), *z1 = &(z[1]), *z2 = &(z[2]), *z3 = &(z[3]),
         *z4 = &(z[4]), *z5 = &(z[5]), *z6 = &(z[6]), *z7 = &(z[7]),
         *z8 = &(z[8]), *z9 = &(z[9]), *zA = &(z[10]), *zB = &(z[11]),
         *zC = &(z[12]), *zD = &(z[13]), *zE = &(z[14]), *zF = &(z[15]);

    // ***** MASKING KEYS *****
    // z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    quart(z, x, S5[*xD], S6[*xF], S7[*xC], S8[*xE], S7[*x8]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    quart(z+8, x+0xC, S5[*z7], S6[*z6], S7[*z5], S8[*z4], S5[*x9]);
    // zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    quart(z+0xC, x+4, S5[*zA], S6[*z9], S7[*zB], S8[*z8], S6[*xB]);

    for (int i = 0; i < 16; i++)
        printf("z%X=%x\n", i, z[i]);

    Km[0] = S5[*z8] ^ S6[*z9] ^ S7[*z7] ^ S8[*z6] ^ S5[*z2];
    Km[1] = S5[*zA] ^ S6[*zB] ^ S7[*z5] ^ S8[*z4] ^ S6[*z6];
    Km[2] = S5[*zC] ^ S6[*zD] ^ S7[*z3] ^ S8[*z2] ^ S7[*z9];
    Km[3] = S5[*zE] ^ S6[*zF] ^ S7[*z1] ^ S8[*z0] ^ S8[*zC];

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);

    Km[4] = S5[*x3] ^ S6[*x2] ^ S7[*xC] ^ S8[*xD] ^ S5[*x8];
    Km[5] = S5[*x1] ^ S6[*x0] ^ S7[*xE] ^ S8[*xF] ^ S6[*xD];
    Km[6] = S5[*x7] ^ S6[*x6] ^ S7[*x8] ^ S8[*x9] ^ S7[*x3];
    Km[7] = S5[*x5] ^ S6[*x4] ^ S7[*xA] ^ S8[*xB] ^ S8[*x7];

    // z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    quart(z, x, S5[*xD], S6[*xF], S7[*xC], S8[*xE], S7[*x8]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    quart(z+8, x+0xC, S5[*z7], S6[*z6], S7[*z5], S8[*z4], S5[*x9]);
    // zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    quart(z+0xC, x+4, S5[*zA], S6[*z9], S7[*zB], S8[*z8], S6[*xB]);

    Km[8] = S5[*z3] ^ S6[*z2] ^ S7[*zC] ^ S8[*zD] ^ S5[*z9];
    Km[9] = S5[*z1] ^ S6[*z0] ^ S7[*zE] ^ S8[*zF] ^ S6[*zC];
    Km[10] = S5[*z7] ^ S6[*z6] ^ S7[*z8] ^ S8[*z9] ^ S7[*z2];
    Km[11] = S5[*z5] ^ S6[*z4] ^ S7[*zA] ^ S8[*zB] ^ S8[*z6];

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);

    Km[12] = S5[*x8] ^ S6[*x9] ^ S7[*x7] ^ S8[*x6] ^ S5[*x3];
    Km[13] = S5[*xA] ^ S6[*xB] ^ S7[*x5] ^ S8[*x4] ^ S6[*x7];
    Km[14] = S5[*xC] ^ S6[*xD] ^ S7[*x3] ^ S8[*x2] ^ S7[*x8];
    Km[15] = S5[*xE] ^ S6[*xF] ^ S7[*x1] ^ S8[*x0] ^ S8[*xD];

    // ***** ROTATING KEYS *****
    // use only the least significant 5 bits
    // z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    quart(z, x, S5[*xD], S6[*xF], S7[*xC], S8[*xE], S7[*x8]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    quart(z+8, x+0xC, S5[*z7], S6[*z6], S7[*z5], S8[*z4], S5[*x9]);
    // zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    quart(z+0xC, x+4, S5[*zA], S6[*z9], S7[*zB], S8[*z8], S6[*xB]);

    Kr[0] = TAKE_5BITS(S5[*z8] ^ S6[*z9] ^ S7[*z7] ^ S8[*z6] ^ S5[*z2]);
    Kr[1] = TAKE_5BITS(S5[*zA] ^ S6[*zB] ^ S7[*z5] ^ S8[*z4] ^ S6[*z6]);
    Kr[2] = TAKE_5BITS(S5[*zC] ^ S6[*zD] ^ S7[*z3] ^ S8[*z2] ^ S7[*z9]);
    Kr[3] = TAKE_5BITS(S5[*zE] ^ S6[*zF] ^ S7[*z1] ^ S8[*z0] ^ S8[*zC]);

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);
    
    Kr[4] = TAKE_5BITS(S5[*x3] ^ S6[*x2] ^ S7[*xC] ^ S8[*xD] ^ S5[*x8]);
    Kr[5] = TAKE_5BITS(S5[*x1] ^ S6[*x0] ^ S7[*xE] ^ S8[*xF] ^ S6[*xD]);
    Kr[6] = TAKE_5BITS(S5[*x7] ^ S6[*x6] ^ S7[*x8] ^ S8[*x9] ^ S7[*x3]);
    Kr[7] = TAKE_5BITS(S5[*x5] ^ S6[*x4] ^ S7[*xA] ^ S8[*xB] ^ S8[*x7]);

    // z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    quart(z, x, S5[*xD], S6[*xF], S7[*xC], S8[*xE], S7[*x8]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    quart(z+8, x+0xC, S5[*z7], S6[*z6], S7[*z5], S8[*z4], S5[*x9]);
    // zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    quart(z+0xC, x+4, S5[*zA], S6[*z9], S7[*zB], S8[*z8], S6[*xB]);

    Kr[8] = TAKE_5BITS(S5[*z3] ^ S6[*z2] ^ S7[*zC] ^ S8[*zD] ^ S5[*z9]);
    Kr[9] = TAKE_5BITS(S5[*z1] ^ S6[*z0] ^ S7[*zE] ^ S8[*zF] ^ S6[*zC]);
    Kr[10] = TAKE_5BITS(S5[*z7] ^ S6[*z6] ^ S7[*z8] ^ S8[*z9] ^ S7[*z2]);
    Kr[11] = TAKE_5BITS(S5[*z5] ^ S6[*z4] ^ S7[*zA] ^ S8[*zB] ^ S8[*z6]);

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);

    Kr[12] = TAKE_5BITS(S5[*x8] ^ S6[*x9] ^ S7[*x7] ^ S8[*x6] ^ S5[*x3]);
    Kr[13] = TAKE_5BITS(S5[*xA] ^ S6[*xB] ^ S7[*x5] ^ S8[*x4] ^ S6[*x7]);
    Kr[14] = TAKE_5BITS(S5[*xC] ^ S6[*xD] ^ S7[*x3] ^ S8[*x2] ^ S7[*x8]);
    Kr[15] = TAKE_5BITS(S5[*xE] ^ S6[*xF] ^ S7[*x1] ^ S8[*x0] ^ S8[*xD]);

    puts("SUBKEYS:");
    for (int i = 0; i < 16; i++)
        printf("  %2d: Km = %08x, Kr = %02x\n", i, Km[i], Kr[i]);

    return cast;
}

// function Type 1
static u32 f1(u32 D, u32 km, byte kr) {
    // I = ((Kmi + D) <<< Kri)
    u32 I = leftrot(km + D, kr);
    // Ia...Id - from most to least significant byte
    // little-endian
    byte Ia = ((byte*) &I)[3];
    byte Ib = ((byte*) &I)[2];
    byte Ic = ((byte*) &I)[1];
    byte Id = ((byte*) &I)[0];
    // f = ((S1[Ia] ^ S2[Ib]) - S3[Ic]) + S4[Id]
    return (S1[Ia] ^ S2[Ib]) - S3[Ic] + S4[Id];
}

// function Type 2
static u32 f2(u32 D, u32 km, byte kr) {
    // I = ((Kmi ^ D) <<< Kri)
    u32 I = leftrot(km ^ D, kr);
    // Ia...Id - from most to least significant byte
    // little-endian
    byte Ia = ((byte*) &I)[3];
    byte Ib = ((byte*) &I)[2];
    byte Ic = ((byte*) &I)[1];
    byte Id = ((byte*) &I)[0];
    // f = ((S1[Ia] - S2[Ib]) + S3[Ic]) ^ S4[Id]
    return (S1[Ia] - S2[Ib] + S3[Ic]) ^ S4[Id];
}

// function Type 3
static u32 f3(u32 D, u32 km, byte kr) {
    // I = ((Kmi - D) <<< Kri)
    u32 I = leftrot(km - D, kr);
    // Ia...Id - from most to least significant byte
    // little-endian
    byte Ia = ((byte*) &I)[3];
    byte Ib = ((byte*) &I)[2];
    byte Ic = ((byte*) &I)[1];
    byte Id = ((byte*) &I)[0];
    // f = ((S1[Ia] + S2[Ib]) ^ S3[Ic]) - S4[Id]
    return ((S1[Ia] + S2[Ib]) ^ S3[Ic]) - S4[Id];
}

static void enc_round(u32 (*f)(u32, u32, byte),
        u32 *l, u32 *r, u32 km, byte kr) 
{
    u32 _l = *l; // save current value before assigning
    *l = *r;                 // Li = Ri-1
    *r = _l ^ f(*r, km, kr); // Ri = Li-1 ^ f(Ri-1, Kmi, Kri)
    printf("Li = %08x, Ri = %08x\n", *l, *r);
}

static void dec_round(u32 (*f)(u32, u32, byte),
        u32 *l, u32 *r, u32 km, byte kr)
{
    u32 _r = *l;
    *l = *r ^ f(*l, km, kr);
    *r = _r;
    printf("Li = %08x, Ri = %08x\n", *l, *r);
}

// processes a single block of plaintext with given key
//
// *plain is a pointer to 8 bytes of plaintext
// 8 bytes of ciphertext are stored at *ciph
// *Km - masking keys
// *Kr - rotating keys
static void enc_block(byte *plain, byte *ciph, u32 *Km, byte *Kr) {
    // === split plaintext into 2 halves ===
    u32 left = u32_from_bytes(plain);
    u32 right = u32_from_bytes(plain+4);
    printf("L0 = %08x, R0 = %08x\n", left, right);

    // 3. 16 rounds
    // Rounds 1, 4, 7, 10, 13, and 16 use f function Type 1.
    // Rounds 2, 5, 8, 11, and 14 use f function Type 2.
    // Rounds 3, 6, 9, 12, and 15 use f function Type 3.
    enc_round(f1, &left, &right, Km[0],  Kr[0]);  // 1
    enc_round(f2, &left, &right, Km[1],  Kr[1]);  // 2
    enc_round(f3, &left, &right, Km[2],  Kr[2]);  // 3
    enc_round(f1, &left, &right, Km[3],  Kr[3]);  // 4
    enc_round(f2, &left, &right, Km[4],  Kr[4]);  // 5
    enc_round(f3, &left, &right, Km[5],  Kr[5]);  // 6
    enc_round(f1, &left, &right, Km[6],  Kr[6]);  // 7
    enc_round(f2, &left, &right, Km[7],  Kr[7]);  // 8
    enc_round(f3, &left, &right, Km[8],  Kr[8]);  // 9
    enc_round(f1, &left, &right, Km[9],  Kr[9]);  // 10
    enc_round(f2, &left, &right, Km[10], Kr[10]); // 11
    enc_round(f3, &left, &right, Km[11], Kr[11]); // 12
    enc_round(f1, &left, &right, Km[12], Kr[12]); // 13
    enc_round(f2, &left, &right, Km[13], Kr[13]); // 14
    enc_round(f3, &left, &right, Km[14], Kr[14]); // 15
    enc_round(f1, &left, &right, Km[15], Kr[15]); // 16

    // 4. ciphertext = c1...c64 = R16,L16
    // c1...c32 = r1...r32
    // c33...c64 = l1...l32
    ciph[0] = ((byte*)&right)[3];
    ciph[1] = ((byte*)&right)[2];
    ciph[2] = ((byte*)&right)[1];
    ciph[3] = ((byte*)&right)[0];

    ciph[4] = ((byte*)&left)[3];
    ciph[5] = ((byte*)&left)[2];
    ciph[6] = ((byte*)&left)[1];
    ciph[7] = ((byte*)&left)[0];
}

static void dec_block(byte *ciph, byte *plain, u32 *Km, byte *Kr) {
    /* Decryption is identical to the encryption algorithm,
       except that the rounds (and therefore the subkey pairs) are used in
       reverse order to compute (L0,R0) from (R16,L16).*/

    // ciphertext = c1...c64 = R16,L16
    // c1...c32 = r1...r32
    // c33...c64 = l1...l32
    u32 right = u32_from_bytes(ciph);
    u32 left = u32_from_bytes(ciph+4);
    printf("L16 = %08x, R16 = %08x\n", left, right);

    // 16 rounds
    // Rounds 1, 4, 7, 10, 13, and 16 use f function Type 1.
    // Rounds 2, 5, 8, 11, and 14 use f function Type 2.
    // Rounds 3, 6, 9, 12, and 15 use f function Type 3.
    dec_round(f1, &left, &right, Km[15], Kr[15]); // 16
    dec_round(f3, &left, &right, Km[14], Kr[14]); // 15
    dec_round(f2, &left, &right, Km[13], Kr[13]); // 14
    dec_round(f1, &left, &right, Km[12], Kr[12]); // 13
    dec_round(f3, &left, &right, Km[11], Kr[11]); // 12
    dec_round(f2, &left, &right, Km[10], Kr[10]); // 11
    dec_round(f1, &left, &right, Km[9],  Kr[9]);  // 10
    dec_round(f3, &left, &right, Km[8],  Kr[8]);  // 9
    dec_round(f2, &left, &right, Km[7],  Kr[7]);  // 8
    dec_round(f1, &left, &right, Km[6],  Kr[6]);  // 7
    dec_round(f3, &left, &right, Km[5],  Kr[5]);  // 6
    dec_round(f2, &left, &right, Km[4],  Kr[4]);  // 5
    dec_round(f1, &left, &right, Km[3],  Kr[3]);  // 4
    dec_round(f3, &left, &right, Km[2],  Kr[2]);  // 3
    dec_round(f2, &left, &right, Km[1],  Kr[1]);  // 2
    dec_round(f1, &left, &right, Km[0],  Kr[0]);  // 1

    // 4. plaintext = L0,R0
    plain[0] = ((byte*)&left)[3];
    plain[1] = ((byte*)&left)[2];
    plain[2] = ((byte*)&left)[1];
    plain[3] = ((byte*)&left)[0];

    plain[4] = ((byte*)&right)[3];
    plain[5] = ((byte*)&right)[2];
    plain[6] = ((byte*)&right)[1];
    plain[7] = ((byte*)&right)[0];
}

int Cast128_encrypt(Cast128 *cast, byte *msg, size_t n, byte *ciph) {
    //TODO
    if (n % BLOCKSIZE) {
        printf("Sorry, only messages with length that is a multiple of %d are supported\n",
                BLOCKSIZE);
        return -1;
    }

    size_t bn = n / BLOCKSIZE; // num of blocks
    for (int i = 0; i < bn; i++) {
        enc_block(msg, ciph, cast->Km, cast->Kr);
        msg += BLOCKSIZE;
        ciph += BLOCKSIZE;
    }

    return 0;
}

int Cast128_decrypt(Cast128 *cast, byte *ciph, size_t n, byte *msg) {
    //TODO
    if (n % BLOCKSIZE) {
        printf("Sorry, only ciphertext with length that is a multiple of %d is supported\n",
                BLOCKSIZE);
        return -1;
    }

    size_t bn = n / BLOCKSIZE; // num of blocks
    for (int i = 0; i < bn; i++) {
        dec_block(ciph, msg, cast->Km, cast->Kr);
        ciph += BLOCKSIZE;
        msg += BLOCKSIZE;
    }

    return 0;
}
