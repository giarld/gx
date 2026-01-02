//
// Created by Gxin on 2022/10/28.
//

#include "gx/ghashsum.h"

#include "algorithm/md5.h"
#include "algorithm/sha1.h"
#include "algorithm/sha256.h"


class Md5HashJob : public GHashJob
{
public:
    explicit Md5HashJob()
    {
        fMD5Init(&mContext);
    }

    ~Md5HashJob() override
    = default;

    void update(const GByteArray &data) override
    {
        fMD5Update(&mContext, data.data(), data.size());
    }

    void update(const uint8_t *data, uint32_t size) override
    {
        fMD5Update(&mContext, data, size);
    }

    GByteArray final() override
    {
        uint8_t digest[MD5_BLOCK_SIZE];
        fMD5Final(&mContext, digest);
        return GByteArray(digest, MD5_BLOCK_SIZE);
    }

private:
    Md5Context mContext{};
};

class Sha1Job : public GHashJob
{
public:
    explicit Sha1Job()
    {
        fSHA1Init(&mContext);
    }

    ~Sha1Job() override
    = default;

    void update(const GByteArray &data) override
    {
        fSHA1Update(&mContext, data.data(), data.size());
    }

    void update(const uint8_t *data, uint32_t size) override
    {
        fSHA1Update(&mContext, data, size);
    }

    GByteArray final() override
    {
        uint8_t digest[SHA1_BLOCK_SIZE];
        fSHA1Final(&mContext, digest);
        return GByteArray(digest, SHA1_BLOCK_SIZE);
    }

private:
    SHA1Context mContext{};
};

class Sha256Job : public GHashJob
{
public:
    explicit Sha256Job()
    {
        fSHA256Init(&mContext);
    }

    ~Sha256Job() override
    = default;

    void update(const GByteArray &data) override
    {
        fSHA256Update(&mContext, data.data(), data.size());
    }

    void update(const uint8_t *data, uint32_t size) override
    {
        fSHA256Update(&mContext, data, size);
    }

    GByteArray final() override
    {
        uint8_t digest[SHA256_BLOCK_SIZE];
        fSHA256Final(&mContext, digest);
        return GByteArray(digest, SHA256_BLOCK_SIZE);
    }

private:
    SHA256Context mContext{};
};


std::unique_ptr<GHashJob> GHashSum::hashSum(HashType hashType)
{
    switch (hashType) {
        case Md5:
            return std::make_unique<Md5HashJob>();
        case Sha1:
            return std::make_unique<Sha1Job>();
        case Sha256:
            return std::make_unique<Sha256Job>();
    }
    return nullptr;
}
