//
// Created by Gxin on 2022/4/22.
//

#ifndef GX_ALGORITHM_SHA_1_H
#define GX_ALGORITHM_SHA_1_H

#include <gx/gglobal.h>


#define SHA1_BLOCK_SIZE 20

struct SHA1Context
{
    uint8_t data[64];
    uint32_t dataLen;
    uint64_t bitLen;
    uint32_t state[5];
    uint32_t k[4];
};

void fSHA1Init(SHA1Context *ctx);

void fSHA1Update(SHA1Context *ctx, const uint8_t *data, uint32_t len);

void fSHA1Final(SHA1Context *ctx, uint8_t *hash);

#endif //GX_ALGORITHM_SHA_1_H
