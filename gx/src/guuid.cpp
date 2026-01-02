//
// Created by Gxin on 2020/11/24.
//

#include "gx/guuid.h"

#include <gx/gtime.h>

#include <sstream>
#include <string>


#if GX_PLATFORM_WINDOWS

#include <objbase.h>

#elif GX_PLATFORM_LINUX || GX_PLATFORM_OSX || GX_PLATFORM_IOS

#include <uuid/uuid.h>

#else

#include <gx/gcrypto.h>

#endif

#define UUID_BUF_LEN 64

#if GX_PLATFORM_WINDOWS

static void genUUID(GUuid &result)
{
    static bool sInit = false;
    if (!sInit) {
        CoInitialize(nullptr);
        sInit = true;
    }

    CoCreateGuid(reinterpret_cast<GUID *>(&result.values));
}

#elif GX_PLATFORM_LINUX || GX_PLATFORM_OSX || GX_PLATFORM_IOS

static void genUUID(GUuid &result)
{
    uuid_t uuid;
    uuid_generate(uuid);
    auto ptr = (uint32_t *) &uuid;
    result.a = ptr[0];
    result.b = ptr[1];
    result.c = ptr[2];
    result.d = ptr[3];
}

#else

static void genUUID(GUuid &result)
{
    GByteArray randData = GCrypto::randomBytes(4*4);
    randData.read(result.a);
    randData.read(result.b);
    randData.read(result.c);
    randData.read(result.d);
}

#endif

GUuid::GUuid()
    : a(0), b(0), c(0), d(0)
{
    genUUID(*this);
}

GUuid::GUuid(const std::string &uuid)
{
    GString text = uuid;
    switch (uuid.size()) {
        case 32: {
            // FormatType::N
            std::stringstream sA;
            std::stringstream sB;
            std::stringstream sC;
            std::stringstream sD;
            sA << std::hex << text.substring(0, 8);
            sB << std::hex << text.substring(8, 8);
            sC << std::hex << text.substring(16, 8);
            sD << std::hex << text.substring(24, 8);
            sA >> a;
            sB >> b;
            sC >> c;
            sD >> d;
        }
        break;
        case 36: {
            // FormatType::D
            std::stringstream sA;
            std::stringstream sB;
            std::stringstream sC;
            std::stringstream sD;
            sA << std::hex << text.substring(0, 8);
            sB << std::hex << (text.substring(9, 4) + text.substring(14, 4));
            sC << std::hex << (text.substring(19, 4) + text.substring(24, 4));
            sD << std::hex << text.substring(28, 8);
            sA >> a;
            sB >> b;
            sC >> c;
            sD >> d;
        }
        break;
        case 38: {
            // FormatType::B or FormatType::P
            if (text.at(0) != text.at(text.length() - 1)) {
                std::stringstream sA;
                std::stringstream sB;
                std::stringstream sC;
                std::stringstream sD;
                sA << std::hex << text.substring(1, 8);
                sB << std::hex << (text.substring(10, 4) + text.substring(15, 4));
                sC << std::hex << (text.substring(20, 4) + text.substring(25, 4));
                sD << std::hex << text.substring(29, 8);
                sA >> a;
                sB >> b;
                sC >> c;
                sD >> d;
            }
        }
        break;
        default: {
            a = b = c = d = 0;
        }
    }
}

GUuid::GUuid(GUuid &&b) noexcept
    : a(b.a), b(b.b), c(b.c), d(b.d)
{
}

GUuid::GUuid(const GUuid &b)
    : a(b.a), b(b.b), c(b.c), d(b.d)
{
}

GUuid::~GUuid() = default;

std::string GUuid::toString() const
{
    return toString(FormatType::D);
}

std::string GUuid::toString(FormatType formatType) const
{
    char buf[UUID_BUF_LEN] = {};

    switch (formatType) {
        case FormatType::N: {
            snprintf(buf, UUID_BUF_LEN, "%08x%08x%08x%08x",
                     a, b, c, d);
        }
        break;
        case FormatType::B: {
            snprintf(buf, UUID_BUF_LEN,
                     "{%08x-%04x-%04x-%04x-%04x%08x}",
                     a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
        break;
        case FormatType::P: {
            snprintf(buf, UUID_BUF_LEN,
                     "(%08x-%04x-%04x-%04x-%04x%08x)",
                     a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
        break;
        default: {
            snprintf(buf, UUID_BUF_LEN,
                     "%08x-%04x-%04x-%04x-%04x%08x",
                     a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
    }
    return buf;
}

bool GUuid::isValid() const
{
    return (a | b | c | d) != 0;
}

bool GUuid::operator==(const GUuid &rh) const
{
    return gx::bitwiseEqual(values, rh.values);
}

bool GUuid::operator!=(const GUuid &rh) const
{
    return !(*this == rh);
}
