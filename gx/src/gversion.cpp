//
// Created by Gxin on 2022/7/18.
//

#include "gx/gversion.h"

#include "gx/gstring.h"

#include <sstream>


GVersion::GVersion(uint64_t versionCode)
{
    mMajor = versionCode >> 48 & 0xFFFF;
    mMinor = versionCode >> 32 & 0xFFFF;
    mPatch = versionCode >> 16 & 0xFFFF;
    mBuild = versionCode & 0xFFFF;
}

GVersion::GVersion(uint16_t major, uint16_t minor, uint16_t patch, uint16_t build)
    : mMajor(major), mMinor(minor), mPatch(patch), mBuild(build)
{
}

GVersion::GVersion(const std::string &version)
{
    const GString verStr(version);
    const auto verPs = verStr.split(".");

    if (!verPs.empty()) {
        mMajor = std::stoi(verPs[0].toStdString());
    }
    if (verPs.size() >= 2) {
        mMinor = std::stoi(verPs[1].toStdString());
    }
    if (verPs.size() >= 3) {
        mPatch = std::stoi(verPs[2].toStdString());
    }
    if (verPs.size() >= 4) {
        mBuild = std::stoi(verPs[3].toStdString());
    }
}

GVersion::~GVersion() = default;

GVersion::GVersion(const GVersion &b)
    : mMajor(b.mMajor),
      mMinor(b.mMinor),
      mPatch(b.mPatch),
      mBuild(b.mBuild)
{
}

GVersion::GVersion(GVersion &&b) noexcept
    : mMajor(b.mMajor),
      mMinor(b.mMinor),
      mPatch(b.mPatch),
      mBuild(b.mBuild)
{
}

GVersion &GVersion::operator=(const GVersion &b)
{
    if (this != &b) {
        mMajor = b.mMajor;
        mMinor = b.mMinor;
        mPatch = b.mPatch;
        mBuild = b.mBuild;
    }
    return *this;
}

GVersion &GVersion::operator=(GVersion &&b) noexcept
{
    if (this != &b) {
        std::swap(mMajor, b.mMajor);
        std::swap(mMinor, b.mMinor);
        std::swap(mPatch, b.mPatch);
        std::swap(mBuild, b.mBuild);
    }
    return *this;
}

uint64_t GVersion::toUInt64() const
{
    return mBuild | (mPatch << 16) | (static_cast<uint64_t>(mMinor) << 32) | (static_cast<uint64_t>(mMajor) << 48);
}

std::string GVersion::toString() const
{
    std::stringstream ss;
    ss << mMajor << "." << mMinor << "." << mPatch << "." << mBuild;
    return ss.str();
}

uint16_t GVersion::getMajor() const
{
    return mMajor;
}

uint16_t GVersion::getMinor() const
{
    return mMinor;
}

uint16_t GVersion::getPatch() const
{
    return mPatch;
}

uint16_t GVersion::getBuild() const
{
    return mBuild;
}

bool GVersion::operator==(const GVersion &b) const
{
    return this->toUInt64() == b.toUInt64();
}

bool GVersion::operator!=(const GVersion &b) const
{
    return this->toUInt64() != b.toUInt64();
}

bool GVersion::operator<(const GVersion &b) const
{
    return this->toUInt64() < b.toUInt64();
}

bool GVersion::operator<=(const GVersion &b) const
{
    return this->toUInt64() <= b.toUInt64();
}

bool GVersion::operator>(const GVersion &b) const
{
    return this->toUInt64() > b.toUInt64();
}

bool GVersion::operator>=(const GVersion &b) const
{
    return this->toUInt64() >= b.toUInt64();
}
