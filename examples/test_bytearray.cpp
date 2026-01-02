//
// Created by Gxin on 2022/4/22.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/reg_gx.h>

#include <gx/gbytearray.h>
#include <gx/gfile.h>
#include <gx/gany.h>
#include <gx/debug.h>

#include <cstring>


struct Info
{
    int a;
    float b;
    char c;
};

inline bool operator==(const Info &lhs, const Info &rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c;
}

GByteArray &operator<<(GByteArray &ba, const Info &out)
{
    ba.write(out);
    return ba;
}

GByteArray &operator>>(GByteArray &ba, Info &in)
{
    ba.read(in);
    return ba;
}

bool testRW()
{
    GByteArray ba;
    ba.write((uint32_t) 1);
    ba.write(2.0f);
    ba.write((double) 3.0);
    ba.write(std::string("hello"));
    ba.write(Info{1, 2.0f, 'c'});

    uint32_t v1;
    float v2;
    double v3;
    std::string v4;
    Info v5;

    ba.read(v1);
    ba.read(v2);
    ba.read(v3);
    ba.read(v4);
    ba.read(v5);

    return v1 == 1 && v2 == 2.0f && v3 == 3.0 && v4 == "hello" && v5 == Info{1, 2.0f, 'c'};
}

bool testRW2()
{
    GByteArray ba;
    ba << (uint32_t) 1 << 2.0f << (double) 3.0 << std::string("hello") << Info{1, 2.0f, 'c'};

    uint32_t v1;
    float v2;
    double v3;
    std::string v4;
    Info v5;

    ba >> v1 >> v2 >> v3 >> v4 >> v5;

    return v1 == 1 && v2 == 2.0f && v3 == 3.0 && v4 == "hello" && v5 == Info{1, 2.0f, 'c'};
}

bool testRWGAny()
{
    GAny objA = 123;
    GAny objB = 3.1415926;
    GAny objC = "Hello";
    GAny objD = true;
    GAny objE = {1, 2, 3, 4.44, "5"};
    GAny objF = {
            {"a", 1},
            {"b", 2.1234},
            {"c", false},
            {"d", "str"},
            {"e", objE}
    };

    GByteArray ba;
    ba << objA << objB << objC << objD << objE << objF;

    GAny retA, retB, retC, retD, retE, retF;
    ba >> retA >> retB >> retC >> retD >> retE >> retF;

    return retA == objA
           && retB == objB
           && retC == objC
           && retD == objD
           && retE == objE
           && retF == objF;
}

bool testHex()
{
    GByteArray ba1;
    ba1.write((uint32_t) 1);
    ba1.write(2.0f);
    ba1.write((double) 3.0);
    ba1.write(std::string("hello"));
    ba1.write(Info{1, 2.0f, 'c'});

    std::string hex = ba1.toHexString();

    GByteArray ba2 = GByteArray::fromHexString(hex);

    return ba1 == ba2;
}

bool testCompare()
{
    GByteArray ba1;
    ba1.write((uint32_t) 1);
    ba1.write(2.0f);
    ba1.write((double) 3.0);
    ba1.write(std::string("hello"));
    ba1.write(Info{1, 2.0f, 'c'});

    GByteArray ba2;
    ba2.write((uint32_t) 1);
    ba2.write(2.0f);
    ba2.write((double) 3.0);
    ba2.write(std::string("hello"));
    ba2.write(Info{1, 2.0f, 'c'});

    return ba1 == ba2;
}

bool testCompress()
{
    GByteArray ba;
    ba.write((uint32_t) 1);
    ba.write(2.0f);
    ba.write((double) 3.0);
    ba.write(std::string("hello"));
    ba.write(Info{1, 2.0f, 'c'});

    GByteArray baCompressed = GByteArray::compress(ba);
    GByteArray ba2 = GByteArray::uncompress(baCompressed);

    return ba == ba2;
}

bool testBase64()
{
    GByteArray ba;
    ba.write((uint32_t) 1);
    ba.write(2.0f);
    ba.write((double) 3.0);
    ba.write(std::string("hello"));
    ba.write(Info{1, 2.0f, 'c'});

    std::string baBase64 = GByteArray::base64Encode(ba);
    GByteArray ba2 = GByteArray::base64Decode(baBase64);

    return ba == ba2;
}

bool testMd5Sum()
{
    const char *str = "Hello World!";
    GByteArray ba;
    ba.write((uint8_t *) str, strlen(str));

    GByteArray md5sum = GByteArray::md5Sum(ba);

    return md5sum.toHexString() == "ed076287532e86365e841e92bfc50d8c";
}

bool testSha1Sum()
{
    const char *str = "Hello World!";
    GByteArray ba;
    ba.write((uint8_t *) str, strlen(str));

    GByteArray sha1sum = GByteArray::sha1Sum(ba);

    return sha1sum.toHexString() == "2ef7bde608ce5404e97d5f042f95f89f1c232871";
}

bool testSha256Sum()
{
    const char *str = "Hello World!";
    GByteArray ba;
    ba.write((uint8_t *) str, strlen(str));

    GByteArray sha256sum = GByteArray::sha256Sum(ba);

    return sha256sum.toHexString() == "7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069";
}

bool testByteOrder()
{
    uint64_t v1 = 0x12344321ffeeaabb;

    GByteArray ba;
    ba.setByteOrder(GByteArray::ByteOrder::BigEndian);
    ba.write(v1);

    uint64_t r1;
    ba.read(r1);

#if GX_CPU_ENDIAN_LITTLE
    uint64_t bigV = 0xbbaaeeff21433412;
    return r1 == v1 && memcmp(ba.data(), &bigV, sizeof(uint64_t)) == 0;
#else
    uint64_t bigV = 0x12344321ffeeaabb;
    return r1 == v1 && memcmp(ba.data(), &bigV, sizeof(uint64_t)) == 0;
#endif
}

bool testCopyOnWrite()
{
    {
        GByteArray ba(8 * 1024);

        GByteArray bb = ba;
        if (ba.data() != bb.data()) {
            return false;
        }

        bb.write(123);  // 写时拷贝

        if (ba.data() == bb.data()) {
            return false;
        }
    }

    GByteArray ba;
    ba.write(123);

    GByteArray bb = ba;
    GByteArray bc = ba;

    if (!(ba == bb && ba == bc)) {
        return false;
    }

    ba.seekWritePos(SEEK_SET, 0);
    ba.write(456);

    return ba != bb && bb == bc;
}

int main(int argc, char *argv[])
{
    initGAnyCore(); // testRWGAny need this

    GANY_LOAD_MODULE(Gx);

    Log("Test read and write: {}", testRW() ? "OK" : "FAIL");
    Log("Test read and write 2: {}", testRW2() ? "OK" : "FAIL");
    Log("Test read and write GAny: {}", testRWGAny() ? "OK" : "FAIL");
    Log("Test hex: {}", testHex() ? "OK" : "FAIL");
    Log("Test compare: {}", testCompare() ? "OK" : "FAIL");
    Log("Test compress: {}", testCompress() ? "OK" : "FAIL");
    Log("Test base64: {}", testBase64() ? "OK" : "FAIL");
    Log("Test md5Sum: {}", testMd5Sum() ? "OK" : "FAIL");
    Log("Test sha1Sum: {}", testSha1Sum() ? "OK" : "FAIL");
    Log("Test sha256Sum: {}", testSha256Sum() ? "OK" : "FAIL");
    Log("Test byte order: {}", testByteOrder() ? "OK" : "FAIL");
    Log("Test copy on write: {}", testCopyOnWrite() ? "OK" : "FAIL");

    return EXIT_SUCCESS;
}