//
// Created by Gxin on 2022/10/28.
//

#ifndef GX_HASH_SUM_H
#define GX_HASH_SUM_H

#include "gbytearray.h"


class GHashJob
{
public:
    virtual ~GHashJob()
    {
    }

    virtual void update(const GByteArray &data) = 0;

    virtual void update(const uint8_t *data, uint32_t size) = 0;

    virtual GByteArray final() = 0;
};

class GX_API GHashSum
{
public:
    enum HashType
    {
        Md5 = 1,
        Sha1 = 2,
        Sha256 = 3,
    };

public:
    static std::unique_ptr<GHashJob> hashSum(HashType hashType);
};

#endif //GX_HASH_SUM_H
