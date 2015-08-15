//
//  BRHash.c
//
//  Created by Aaron Voisine on 8/8/15.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include "BRHash.h"
#include <string.h>

// bitwise left rotation
#define rol32(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

// basic sha1 functions
#define f1(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define f2(x, y, z) ((x) ^ (y) ^ (z))
#define f3(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))

// basic sha1 operation
#define sha1(x, y, z) (t = rol32(a, 5) + (x) + e + (y) + (z), e = d, d = c, c = rol32(b, 30), b = a, a = t)

static void BRSHA1Compress(unsigned *r, unsigned *x)
{
    unsigned a = r[0], b = r[1], c = r[2], d = r[3], e = r[4], i = 0, t;
    
    for (; i < 16; i++) sha1(f1(b, c, d), 0x5a827999, (x[i] = BRBE32(x[i])));
    for (; i < 20; i++) sha1(f1(b, c, d), 0x5a827999, (x[i] = rol32(x[i - 3] ^ x[i - 8] ^ x[i - 14] ^ x[i - 16], 1)));
    for (; i < 40; i++) sha1(f2(b, c, d), 0x6ed9eba1, (x[i] = rol32(x[i - 3] ^ x[i - 8] ^ x[i - 14] ^ x[i - 16], 1)));
    for (; i < 60; i++) sha1(f3(b, c, d), 0x8f1bbcdc, (x[i] = rol32(x[i - 3] ^ x[i - 8] ^ x[i - 14] ^ x[i - 16], 1)));
    for (; i < 80; i++) sha1(f2(b, c, d), 0xca62c1d6, (x[i] = rol32(x[i - 3] ^ x[i - 8] ^ x[i - 14] ^ x[i - 16], 1)));
    
    r[0] += a, r[1] += b, r[2] += c, r[3] += d, r[4] += e;
}

BRUInt160 BRSHA1(const void *data, size_t len)
{
    BRUInt160 md;
    unsigned i, x[80], buf[] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 }; // initial buffer values
    
    for (i = 0; i < len; i += 64) { // process data in 64 byte blocks
        memcpy(x, (const char *)data + i, (i + 64 < len) ? 64 : len - i);
        if (i + 64 > len) break;
        BRSHA1Compress(buf, x);
    }
    
    memset((unsigned char *)x + (len - i), 0, 64 - (len - i)); // clear remainder of x
    ((unsigned char *)x)[len - i] = 0x80; // append padding
    if (len - i >= 56) BRSHA1Compress(buf, x), memset(x, 0, sizeof(*x)*16); // length goes to next block
    *(unsigned long long *)&x[14] = BRBE64((unsigned long long)len*8); // append length in bits
    BRSHA1Compress(buf, x); // finalize
    for (i = 0; i < 5; i++) md.u32[i] = BRBE32(buf[i]); // write to md
    return md;
}

// bitwise right rotation
#define ror32(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

// basic sha2 functions
#define ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

// basic sha256 functions
#define s0(x) (ror32((x), 2) ^ ror32((x), 13) ^ ror32((x), 22))
#define s1(x) (ror32((x), 6) ^ ror32((x), 11) ^ ror32((x), 25))
#define s2(x) (ror32((x), 7) ^ ror32((x), 18) ^ ((x) >> 3))
#define s3(x) (ror32((x), 17) ^ ror32((x), 19) ^ ((x) >> 10))

static void BRSHA256Compress(unsigned *r, unsigned *x)
{
    static const unsigned k[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    
    unsigned a = r[0], b = r[1], c = r[2], d = r[3], e = r[4], f = r[5], g = r[6], h = r[7], i, t1, t2, w[64];
    
    for (i = 0; i < 16; i++) w[i] = BRBE32(x[i]);
    for (; i < 64; i++) w[i] = s3(w[i - 2]) + w[i - 7] + s2(w[i - 15]) + w[i - 16];
    
    for (i = 0; i < 64; i++) {
        t1 = h + s1(e) + ch(e, f, g) + k[i] + w[i];
        t2 = s0(a) + maj(a, b, c);
        h = g, g = f, f = e, e = d + t1, d = c, c = b, b = a, a = t1 + t2;
    }
    
    r[0] += a, r[1] += b, r[2] += c, r[3] += d, r[4] += e, r[5] += f, r[6] += g, r[7] += h;
}

BRUInt256 BRSHA256(const void *data, size_t len)
{
    BRUInt256 md;
    size_t i;
    unsigned x[16], buf[] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                              0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }; // initial buffer values
    
    for (i = 0; i < len; i += 64) { // process data in 64 byte blocks
        memcpy(x, (const char *)data + i, (i + 64 < len) ? 64 : len - i);
        if (i + 64 > len) break;
        BRSHA256Compress(buf, x);
    }
    
    memset((unsigned char *)x + (len - i), 0, 64 - (len - i)); // clear remainder of x
    ((unsigned char *)x)[len - i] = 0x80; // append padding
    if (len - i >= 56) BRSHA256Compress(buf, x), memset(x, 0, 64); // length goes to next block
    *(unsigned long long *)&x[14] = BRBE64((unsigned long long)len*8); // append length in bits
    BRSHA256Compress(buf, x); // finalize
    for (i = 0; i < 8; i++) md.u32[i] = BRBE32(buf[i]); // write to md
    return md;
}

BRUInt256 BRSHA256_2(const void *data, size_t len)
{
    BRUInt256 md = BRSHA256(data, len);
    
    return BRSHA256(&md, sizeof(md));
}

// bitwise right rotation
#define ror64(a, b) (((a) >> (b)) | ((a) << (64 - (b))))

// basic sha512 opeartions
#define S0(x) (ror64((x), 28) ^ ror64((x), 34) ^ ror64((x), 39))
#define S1(x) (ror64((x), 14) ^ ror64((x), 18) ^ ror64((x), 41))
#define S2(x) (ror64((x), 1) ^ ror64((x), 8) ^ ((x) >> 7))
#define S3(x) (ror64((x), 19) ^ ror64((x), 61) ^ ((x) >> 6))

static void BRSHA512Compress(unsigned long long *r, unsigned long long *x)
{
    static const unsigned long long k[] = {
        0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538,
        0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe,
        0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235,
        0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
        0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab,
        0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725,
        0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed,
        0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
        0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218,
        0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53,
        0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373,
        0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
        0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c,
        0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6,
        0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc,
        0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
    };
    
    unsigned long long a = r[0], b = r[1], c = r[2], d = r[3], e = r[4], f = r[5], g = r[6], h = r[7], t1, t2, w[80];
    unsigned i;
    
    for (i = 0; i < 16; i++) w[i] = BRBE64(x[i]);
    for (; i < 80; i++) w[i] = S3(w[i - 2]) + w[i - 7] + S2(w[i - 15]) + w[i - 16];
    
    for (i = 0; i < 80; i++) {
        t1 = h + S1(e) + ch(e, f, g) + k[i] + w[i];
        t2 = S0(a) + maj(a, b, c);
        h = g, g = f, f = e, e = d + t1, d = c, c = b, b = a, a = t1 + t2;
    }
    
    r[0] += a, r[1] += b, r[2] += c, r[3] += d, r[4] += e, r[5] += f, r[6] += g, r[7] += h;
}

BRUInt512 BRSHA512(const void *data, size_t len)
{
    BRUInt512 md;
    size_t i;
    unsigned long long x[16], buf[] = { 0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
                                        0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};
    
    for (i = 0; i < len; i += 128) { // process data in 128 byte blocks
        memcpy(x, (const char *)data + i, (i + 128 < len) ? 128 : len - i);
        if (i + 128 > len) break;
        BRSHA512Compress(buf, x);
    }
    
    memset((unsigned char *)x + (len - i), 0, 128 - (len - i)); // clear remainder of x
    ((unsigned char *)x)[len - i] = 0x80; // append padding
    if (len - i >= 112) BRSHA512Compress(buf, x), memset(x, 0, 128); // length goes to next block
    x[14] = 0, x[15] = BRBE64((unsigned long long)len*8); // append length in bits
    BRSHA512Compress(buf, x); // finalize
    for (i = 0; i < 8; i++) md.u64[i] = BRBE64(buf[i]); // write to md
    return md;
}

// basic ripemd functions
#define f(x, y, z) ((x) ^ (y) ^ (z))
#define g(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define h(x, y, z) (((x) | ~(y)) ^ (z))
#define i(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define j(x, y, z) ((x) ^ ((y) | ~(z)))

// basic ripemd operation
#define rmd(a, b, c, d, e, f, g, h, i, j) ((a) = rol32((f) + (b) + BRLE32(c) + (d), (e)) + (g), (f) = (g), (g) = (h),\
                                           (h) = rol32((i), 10), (i) = (j), (j) = (a))

// ripemd left line
static const unsigned rl1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, // round 1, id
                      rl2[] = { 7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8 }, // round 2, rho
                      rl3[] = { 3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12 }, // round 3, rho^2
                      rl4[] = { 1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2 }, // round 4, rho^3
                      rl5[] = { 4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13 }; // round 5, rho^4

// ripemd right line
static const unsigned rr1[] = { 5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12 }, // round 1, pi
                      rr2[] = { 6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2 }, // round 2, rho pi
                      rr3[] = { 15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13 }, // round 3, rho^2 pi
                      rr4[] = { 8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14 }, // round 4, rho^3 pi
                      rr5[] = { 12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11 }; // round 5, rho^4 pi

// ripemd left line shifts
static const unsigned sl1[] = { 11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8 }, // round 1
                      sl2[] = { 7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12 }, // round 2
                      sl3[] = { 11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5 }, // round 3
                      sl4[] = { 11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12 }, // round 4
                      sl5[] = { 9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6 }; // round 5

// ripemd right line shifts
static const unsigned sr1[] = { 8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6 }, // round 1
                      sr2[] = { 9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11 }, // round 2
                      sr3[] = { 9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5 }, // round 3
                      sr4[] = { 15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8 }, // round 4
                      sr5[] = { 8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11 }; // round 5

static void BRRMDcompress(unsigned *r, unsigned *x)
{
    unsigned al = r[0], bl = r[1], cl = r[2], dl = r[3], el = r[4], ar = al, br = bl, cr = cl, dr = dl, er = el, i, t;
    
    for (i = 0; i < 16; i++) rmd(t, f(bl, cl, dl), x[rl1[i]], 0x00000000, sl1[i], al, el, dl, cl, bl); // round 1 left
    for (i = 0; i < 16; i++) rmd(t, j(br, cr, dr), x[rr1[i]], 0x50a28be6, sr1[i], ar, er, dr, cr, br); // round 1 right
    for (i = 0; i < 16; i++) rmd(t, g(bl, cl, dl), x[rl2[i]], 0x5a827999, sl2[i], al, el, dl, cl, bl); // round 2 left
    for (i = 0; i < 16; i++) rmd(t, i(br, cr, dr), x[rr2[i]], 0x5c4dd124, sr2[i], ar, er, dr, cr, br); // round 2 right
    for (i = 0; i < 16; i++) rmd(t, h(bl, cl, dl), x[rl3[i]], 0x6ed9eba1, sl3[i], al, el, dl, cl, bl); // round 3 left
    for (i = 0; i < 16; i++) rmd(t, h(br, cr, dr), x[rr3[i]], 0x6d703ef3, sr3[i], ar, er, dr, cr, br); // round 3 right
    for (i = 0; i < 16; i++) rmd(t, i(bl, cl, dl), x[rl4[i]], 0x8f1bbcdc, sl4[i], al, el, dl, cl, bl); // round 4 left
    for (i = 0; i < 16; i++) rmd(t, g(br, cr, dr), x[rr4[i]], 0x7a6d76e9, sr4[i], ar, er, dr, cr, br); // round 4 right
    for (i = 0; i < 16; i++) rmd(t, j(bl, cl, dl), x[rl5[i]], 0xa953fd4e, sl5[i], al, el, dl, cl, bl); // round 5 left
    for (i = 0; i < 16; i++) rmd(t, f(br, cr, dr), x[rr5[i]], 0x00000000, sr5[i], ar, er, dr, cr, br); // round 5 right
    
    t = r[1] + cl + dr; // final result for r[0]
    r[1] = r[2] + dl + er, r[2] = r[3] + el + ar, r[3] = r[4] + al + br, r[4] = r[0] + bl + cr, r[0] = t; // combine
}

// ripemd-160 hash function: http://homes.esat.kuleuven.be/~bosselae/ripemd160.html
BRUInt160 BRRMD160(const void *data, size_t len)
{
    BRUInt160 md;
    size_t i;
    unsigned x[16], buf[] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 }; // initial buffer values
    
    for (i = 0; i <= len; i += 64) { // process data in 64 byte blocks
        memcpy(x, (const char *)data + i, (i + 64 < len) ? 64 : len - i);
        if (i + 64 > len) break;
        BRRMDcompress(buf, x);
    }
    
    memset((unsigned char *)x + (len - i), 0, 64 - (len - i)); // clear remainder of x
    ((unsigned char *)x)[len - i] = 0x80; // append padding
    if (len - i >= 56) BRRMDcompress(buf, x), memset(x, 0, 64); // length goes to next block
    *(unsigned long long *)&x[14] = BRLE64((unsigned long long)len*8); // append length in bits
    BRRMDcompress(buf, x); // finalize
    for (i = 0; i < 5; i++) md.u32[i] = BRLE32(buf[i]); // write to md
    return md;
}

BRUInt160 BRHash160(const void *data, size_t len)
{
    BRUInt256 md = BRSHA256(data, len);
    
    return BRRMD160(&md, sizeof(md));
}
