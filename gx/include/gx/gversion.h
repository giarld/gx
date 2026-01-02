//
// Created by Gxin on 2022/7/18.
//

#ifndef GX_G_VERSION_H
#define GX_G_VERSION_H

#include "gobject.h"


/**
 * @class GVersion
 * @brief Version number conversion and comparison tool
 */
class GX_API GVersion : public GObject
{
public:
    explicit GVersion(uint64_t versionCode = 0);

    explicit GVersion(uint16_t major, uint16_t minor, uint16_t patch, uint16_t build);

    explicit GVersion(const std::string &version);

    ~GVersion() override;

    GVersion(const GVersion &b);

    GVersion(GVersion &&b) noexcept;

    GVersion &operator=(const GVersion &b);

    GVersion &operator=(GVersion &&b) noexcept;

public:
    uint64_t toUInt64() const;

    std::string toString() const override;

    uint16_t getMajor() const;

    uint16_t getMinor() const;

    uint16_t getPatch() const;

    uint16_t getBuild() const;

public:
    bool operator==(const GVersion &b) const;

    bool operator!=(const GVersion &b) const;

    bool operator<(const GVersion &b) const;

    bool operator<=(const GVersion &b) const;

    bool operator>(const GVersion &b) const;

    bool operator>=(const GVersion &b) const;

private:
    uint16_t mMajor = 0;
    uint16_t mMinor = 0;
    uint16_t mPatch = 0;
    uint16_t mBuild = 0;
};

#endif //GX_G_VERSION_H
