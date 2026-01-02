//
// Created by Gxin on 2022/4/22.
//

#ifndef GX_ALGORITHM_SHA256_H
#define GX_ALGORITHM_SHA256_H

#include <gx/gglobal.h>


#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

struct SHA256Context
{
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
};

void fSHA256Init(SHA256Context *ctx);

void fSHA256Update(SHA256Context *ctx, const uint8_t data[], size_t len);

void fSHA256Final(SHA256Context *ctx, uint8_t hash[]);

#endif //GX_ALGORITHM_SHA256_H
