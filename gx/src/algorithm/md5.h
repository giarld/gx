//
// Created by Gxin on 2022/4/22.
//

#ifndef GX_ALGORITHM_MD5_H
#define GX_ALGORITHM_MD5_H

#include <gx/gglobal.h>


#define MD5_BLOCK_SIZE 16

struct Md5Context
{
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
};

void fMD5Init(Md5Context *context);

void fMD5Update(Md5Context *context, const uint8_t *input, uint32_t inputLen);

void fMD5Final(Md5Context *context, uint8_t digest[MD5_BLOCK_SIZE]);

#endif //GX_ALGORITHM_MD5_H
