#include <stdio.h>
#include <string.h>
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

// 2.4 Key Schedule
static void mk_subkeys(uint32_t *mask_keys, byte *rot_keys, byte *key) {
    // The subkeys are formed from the key x0x1x2x3x4x5x6x7x8x9xAxBxCxDxExF
    byte x[16];
    byte *x0 = &(x[0]), *x1 = &(x[1]), *x2 = &(x[2]), *x3 = &(x[3]),
         *x4 = &(x[4]), *x5 = &(x[5]), *x6 = &(x[6]), *x7 = &(x[7]),
         *x8 = &(x[8]), *x9 = &(x[9]), *xA = &(x[10]), *xB = &(x[11]),
         *xC = &(x[12]), *xD = &(x[13]), *xE = &(x[14]), *xF = &(x[15]);
    memcpy (x, key, 16);

    for (int i = 0; i < 16; i++)
        printf ("x%X=%x\n", i, x[i]);

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

    mask_keys[0] = S5[*z8] ^ S6[*z9] ^ S7[*z7] ^ S8[*z6] ^ S5[*z2];
    mask_keys[1] = S5[*zA] ^ S6[*zB] ^ S7[*z5] ^ S8[*z4] ^ S6[*z6];
    mask_keys[2] = S5[*zC] ^ S6[*zD] ^ S7[*z3] ^ S8[*z2] ^ S7[*z9];
    mask_keys[3] = S5[*zE] ^ S6[*zF] ^ S7[*z1] ^ S8[*z0] ^ S8[*zC];

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);

    mask_keys[4] = S5[*x3] ^ S6[*x2] ^ S7[*xC] ^ S8[*xD] ^ S5[*x8];
    mask_keys[5] = S5[*x1] ^ S6[*x0] ^ S7[*xE] ^ S8[*xF] ^ S6[*xD];
    mask_keys[6] = S5[*x7] ^ S6[*x6] ^ S7[*x8] ^ S8[*x9] ^ S7[*x3];
    mask_keys[7] = S5[*x5] ^ S6[*x4] ^ S7[*xA] ^ S8[*xB] ^ S8[*x7];

    // z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    quart(z, x, S5[*xD], S6[*xF], S7[*xC], S8[*xE], S7[*x8]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    quart(z+8, x+0xC, S5[*z7], S6[*z6], S7[*z5], S8[*z4], S5[*x9]);
    // zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    quart(z+0xC, x+4, S5[*zA], S6[*z9], S7[*zB], S8[*z8], S6[*xB]);

    mask_keys[8] = S5[*z3] ^ S6[*z2] ^ S7[*zC] ^ S8[*zD] ^ S5[*z9];
    mask_keys[9] = S5[*z1] ^ S6[*z0] ^ S7[*zE] ^ S8[*zF] ^ S6[*zC];
    mask_keys[10] = S5[*z7] ^ S6[*z6] ^ S7[*z8] ^ S8[*z9] ^ S7[*z2];
    mask_keys[11] = S5[*z5] ^ S6[*z4] ^ S7[*zA] ^ S8[*zB] ^ S8[*z6];

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);

    mask_keys[12] = S5[*x8] ^ S6[*x9] ^ S7[*x7] ^ S8[*x6] ^ S5[*x3];
    mask_keys[13] = S5[*xA] ^ S6[*xB] ^ S7[*x5] ^ S8[*x4] ^ S6[*x7];
    mask_keys[14] = S5[*xC] ^ S6[*xD] ^ S7[*x3] ^ S8[*x2] ^ S7[*x8];
    mask_keys[15] = S5[*xE] ^ S6[*xF] ^ S7[*x1] ^ S8[*x0] ^ S8[*xD];

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

    rot_keys[0] = TAKE_5BITS(S5[*z8] ^ S6[*z9] ^ S7[*z7] ^ S8[*z6] ^ S5[*z2]);
    rot_keys[1] = TAKE_5BITS(S5[*zA] ^ S6[*zB] ^ S7[*z5] ^ S8[*z4] ^ S6[*z6]);
    rot_keys[2] = TAKE_5BITS(S5[*zC] ^ S6[*zD] ^ S7[*z3] ^ S8[*z2] ^ S7[*z9]);
    rot_keys[3] = TAKE_5BITS(S5[*zE] ^ S6[*zF] ^ S7[*z1] ^ S8[*z0] ^ S8[*zC]);

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);
    
    rot_keys[4] = TAKE_5BITS(S5[*x3] ^ S6[*x2] ^ S7[*xC] ^ S8[*xD] ^ S5[*x8]);
    rot_keys[5] = TAKE_5BITS(S5[*x1] ^ S6[*x0] ^ S7[*xE] ^ S8[*xF] ^ S6[*xD]);
    rot_keys[6] = TAKE_5BITS(S5[*x7] ^ S6[*x6] ^ S7[*x8] ^ S8[*x9] ^ S7[*x3]);
    rot_keys[7] = TAKE_5BITS(S5[*x5] ^ S6[*x4] ^ S7[*xA] ^ S8[*xB] ^ S8[*x7]);

    // z0z1z2z3 = x0x1x2x3 ^ S5[xD] ^ S6[xF] ^ S7[xC] ^ S8[xE] ^ S7[x8];
    quart(z, x, S5[*xD], S6[*xF], S7[*xC], S8[*xE], S7[*x8]);
    // z4z5z6z7 = x8x9xAxB ^ S5[z0] ^ S6[z2] ^ S7[z1] ^ S8[z3] ^ S8[xA];
    quart(z+4, x+8, S5[*z0], S6[*z2], S7[*z1], S8[*z3], S8[*xA]);
    // z8z9zAzB = xCxDxExF ^ S5[z7] ^ S6[z6] ^ S7[z5] ^ S8[z4] ^ S5[x9];
    quart(z+8, x+0xC, S5[*z7], S6[*z6], S7[*z5], S8[*z4], S5[*x9]);
    // zCzDzEzF = x4x5x6x7 ^ S5[zA] ^ S6[z9] ^ S7[zB] ^ S8[z8] ^ S6[xB];
    quart(z+0xC, x+4, S5[*zA], S6[*z9], S7[*zB], S8[*z8], S6[*xB]);

    rot_keys[8] = TAKE_5BITS(S5[*z3] ^ S6[*z2] ^ S7[*zC] ^ S8[*zD] ^ S5[*z9]);
    rot_keys[9] = TAKE_5BITS(S5[*z1] ^ S6[*z0] ^ S7[*zE] ^ S8[*zF] ^ S6[*zC]);
    rot_keys[10] = TAKE_5BITS(S5[*z7] ^ S6[*z6] ^ S7[*z8] ^ S8[*z9] ^ S7[*z2]);
    rot_keys[11] = TAKE_5BITS(S5[*z5] ^ S6[*z4] ^ S7[*zA] ^ S8[*zB] ^ S8[*z6]);

    // x0x1x2x3 = z8z9zAzB ^ S5[z5] ^ S6[z7] ^ S7[z4] ^ S8[z6] ^ S7[z0];
    quart(x, z+8, S5[*z5], S6[*z7], S7[*z4], S8[*z6], S7[*z0]);
    // x4x5x6x7 = z0z1z2z3 ^ S5[x0] ^ S6[x2] ^ S7[x1] ^ S8[x3] ^ S8[z2];
    quart(x+4, z, S5[*x0], S6[*x2], S7[*x1], S8[*x3], S8[*z2]);
    // x8x9xAxB = z4z5z6z7 ^ S5[x7] ^ S6[x6] ^ S7[x5] ^ S8[x4] ^ S5[z1];
    quart(x+8, z+4, S5[*x7], S6[*x6], S7[*x5], S8[*x4], S5[*z1]);
    // xCxDxExF = zCzDzEzF ^ S5[xA] ^ S6[x9] ^ S7[xB] ^ S8[x8] ^ S6[z3];
    quart(x+0xC, z+0xC, S5[*xA], S6[*x9], S7[*xB], S8[*x8], S6[*z3]);

    rot_keys[12] = TAKE_5BITS(S5[*x8] ^ S6[*x9] ^ S7[*x7] ^ S8[*x6] ^ S5[*x3]);
    rot_keys[13] = TAKE_5BITS(S5[*xA] ^ S6[*xB] ^ S7[*x5] ^ S8[*x4] ^ S6[*x7]);
    rot_keys[14] = TAKE_5BITS(S5[*xC] ^ S6[*xD] ^ S7[*x3] ^ S8[*x2] ^ S7[*x8]);
    rot_keys[15] = TAKE_5BITS(S5[*xE] ^ S6[*xF] ^ S7[*x1] ^ S8[*x0] ^ S8[*xD]);
}

// function Type 1
static uint32_t f1(uint32_t D, uint32_t km, byte kr) {
    // I = ((Kmi + D) <<< Kri)
    uint32_t I = leftrot(km + D, kr);
    byte Ia = ((byte*) &I)[0];
    byte Ib = ((byte*) &I)[1];
    byte Ic = ((byte*) &I)[2];
    byte Id = ((byte*) &I)[3];
    // f = ((S1[Ia] ^ S2[Ib]) - S3[Ic]) + S4[Id]
    return (S1[Ia] ^ S2[Ib]) - S3[Ic] + S4[Id];
}

// function Type 2
static uint32_t f2(uint32_t D, uint32_t km, byte kr) {
    // I = ((Kmi ^ D) <<< Kri)
    uint32_t I = leftrot(km ^ D, kr);
    byte Ia = ((byte*) &I)[0];
    byte Ib = ((byte*) &I)[1];
    byte Ic = ((byte*) &I)[2];
    byte Id = ((byte*) &I)[3];
    // f = ((S1[Ia] - S2[Ib]) + S3[Ic]) ^ S4[Id]
    return (S1[Ia] - S2[Ib] + S3[Ic]) ^ S4[Id];
}

// function Type 3
static uint32_t f3(uint32_t D, uint32_t km, byte kr) {
    // I = ((Kmi - D) <<< Kri)
    uint32_t I = leftrot(km - D, kr);
    byte Ia = ((byte*) &I)[0];
    byte Ib = ((byte*) &I)[1];
    byte Ic = ((byte*) &I)[2];
    byte Id = ((byte*) &I)[3];
    // f = ((S1[Ia] + S2[Ib]) ^ S3[Ic]) - S4[Id]
    return ((S1[Ia] + S2[Ib]) ^ S3[Ic]) - S4[Id];
}

static void do_round(uint32_t (*f)(uint32_t, uint32_t, byte),
        uint32_t* l0, uint32_t* r0, uint32_t* l, uint32_t* r, 
        uint16_t km, byte kr) 
{
    printf("-> Li = %8x, Ri = %8x\n", *l, *r);
    uint32_t _l = *l; // save current value before assigning
    *l = *r0;
    uint32_t _r = *r; // save current value before assigning
    *r = *l0 ^ f(*r0, km, kr);
    // store previous values
    *r0 = _r;
    *l0 = _l;
    printf("<- Li = %8x, Ri = %8x\n", *l, *r);
}

// processes a single block of plaintext with given key
//
// *plain is a pointer to 8 bytes of plaintext
// *key is a pointer to 16 bytes that compose a 128-bit key
// 8 bytes of ciphertext are stored at *cipher
void enc_block(byte *plain, byte *key, byte *cipher) {
    // 1. 16 pairs of subkeys
        // a pair consists of:
        // * 32-bit masking key
        // * 5-bit rotation key
    uint32_t mask_keys[16]; // Km
    byte rot_keys[16];   // Kr
    // TODO make only once
    mk_subkeys(mask_keys, rot_keys, key);

    puts("SUBKEYS:");
    for (int i = 0; i < 16; i++)
        printf("  %2d: Km = %8x, Kr = %2x\n", i, mask_keys[i], rot_keys[i]);

    //uint32_t mask_keys[16] = {4095708460, 3155639846, 2023884783, 3974146037, 2091973483, 2782058038, 3616248839, 1455450579, 2196251660, 861157193, 2283001287, 3120355122, 1494248246, 1231745449, 418962499, 2375937807};
    //byte rot_keys[16] = {2213389188, 21, 27, 1, 5, 3, 31, 31, 28, 16, 31, 18, 1, 29, 25, 1};
    // 2. split plaintext into 2 halves
    //uint32_t left0 = plain & left_mask64;
    uint32_t left0 = u32_from_bytes(plain);
    uint32_t right0 = u32_from_bytes(plain+4);
    printf("L0 = %16x, R0 = %16x\n", left0, right0);

    // 3. 16 rounds
    uint32_t left, right;
    // Li = Ri-1;
    // Ri = Li-1 ^ f(Ri-1, Kmi, Kri)
    // Rounds 1, 4, 7, 10, 13, and 16 use f function Type 1.
    // Rounds 2, 5, 8, 11, and 14 use f function Type 2.
    // Rounds 3, 6, 9, 12, and 15 use f function Type 3.
    do_round(f1, &left0, &right0, &left, &right, mask_keys[0],  rot_keys[0]);  // 1
    do_round(f2, &left0, &right0, &left, &right, mask_keys[1],  rot_keys[1]);  // 2
    do_round(f3, &left0, &right0, &left, &right, mask_keys[2],  rot_keys[2]);  // 3
    do_round(f1, &left0, &right0, &left, &right, mask_keys[3],  rot_keys[3]);  // 4
    do_round(f2, &left0, &right0, &left, &right, mask_keys[4],  rot_keys[4]);  // 5
    do_round(f3, &left0, &right0, &left, &right, mask_keys[5],  rot_keys[5]);  // 6
    do_round(f1, &left0, &right0, &left, &right, mask_keys[6],  rot_keys[6]);  // 7
    do_round(f2, &left0, &right0, &left, &right, mask_keys[7],  rot_keys[7]);  // 8
    do_round(f3, &left0, &right0, &left, &right, mask_keys[8],  rot_keys[8]);  // 9
    do_round(f1, &left0, &right0, &left, &right, mask_keys[9],  rot_keys[9]);  // 10
    do_round(f2, &left0, &right0, &left, &right, mask_keys[10], rot_keys[10]); // 11
    do_round(f3, &left0, &right0, &left, &right, mask_keys[11], rot_keys[11]); // 12
    do_round(f1, &left0, &right0, &left, &right, mask_keys[12], rot_keys[12]); // 13
    do_round(f2, &left0, &right0, &left, &right, mask_keys[13], rot_keys[13]); // 14
    do_round(f3, &left0, &right0, &left, &right, mask_keys[14], rot_keys[14]); // 15
    do_round(f1, &left0, &right0, &left, &right, mask_keys[15], rot_keys[15]); // 16

    // 4. ciphertext = c1...c64 = R16,L16
    // c1...c32 = r1...r32
    // c33...c64 = l1...l32
    memcpy(cipher, &right, 4);
    memcpy(cipher+4, &left, 4);
}


// TODO
uint64_t dec_block(uint64_t plain, uint64_t *key) {
    return 0;
}
