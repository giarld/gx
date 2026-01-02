//
// Created by Gxin on 2020/11/24.
//

#ifndef GX_GUUID_H
#define GX_GUUID_H

#include "gobject.h"

#include "common.h"


/**
 * @class GUuid
 * @brief Generate UUID and provide string output in multiple formats.
 */
class GX_API GUuid final : public GObject
{
public:
    enum class FormatType : uint8_t
    {
        // 32 digits:
        // 00000000000000000000000000000000
        N,

        // 32 digits separated by hyphens:
        // 00000000-0000-0000-0000-000000000000
        D,

        // 32 digits separated by hyphens, enclosed in braces:
        // {00000000-0000-0000-0000-000000000000}
        B,

        // 32 digits separated by hyphens, enclosed in parentheses:
        // (00000000-0000-0000-0000-000000000000)
        P
    };

    union
    {
        struct
        {
            uint32_t a;
            uint32_t b;
            uint32_t c;
            uint32_t d;
        };

        uint8_t raw[16];
        uint32_t values[4];
    };

public:
    explicit GUuid();

    explicit GUuid(const std::string &uuid);

    GUuid(GUuid &&b) noexcept;

    GUuid(const GUuid &b);

    ~GUuid() override;

public:
    std::string toString() const override;

    std::string toString(FormatType formatType) const;

    bool isValid() const;

    bool operator==(const GUuid &rh) const;

    bool operator!=(const GUuid &rh) const;

    explicit operator bool() const
    {
        return isValid();
    }
};

namespace std
{
template<>
struct hash<GUuid>
{
    size_t operator()(const GUuid &type) const noexcept
    {
        return gx::hashOf(type);
    }
};
}

#endif //GX_GUUID_H
