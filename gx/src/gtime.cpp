//
// Created by Gxin on 2020/6/19.
//

#include "gx/gtime.h"

#include <chrono>
#include <ratio>
#include <utility>


using TimeSec = std::chrono::duration<GTime::TimeType>;
using TimeSecD = std::chrono::duration<double>;
using TimeMilliSec = std::chrono::duration<GTime::TimeType, std::milli>;
using TimeMicroSec = std::chrono::duration<GTime::TimeType, std::micro>;
using TimeNanoSec = std::chrono::duration<GTime::TimeType, std::nano>;
using TimeMinute = std::chrono::duration<GTime::TimeType, std::ratio<60, 1> >;
using TimeHour = std::chrono::duration<GTime::TimeType, std::ratio<3600, 1> >;
using TimeDay = std::chrono::duration<GTime::TimeType, std::ratio<86400, 1> >;

GTime::GTime(Type type, TimeType nanosecond)
    : mType(type),
      mTime(nanosecond)
{
    if (mTime < 0) {
        update();
    }
}

GTime::GTime(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second)
    : mType(SystemClock),
      mTime(0)
{
    tm time{};
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    time.tm_hour = hour;
    time.tm_min = minute;
    time.tm_sec = second;

    addSecs(mktime(&time));
}

GTime::~GTime() = default;

GTime::GTime(const GTime &b) = default;

GTime::GTime(GTime &&b) noexcept
    : mType(b.mType),
      mTime(b.mTime)
{
    b.mTime = 0;
}

GTime &GTime::operator=(const GTime &b) = default;

GTime &GTime::operator=(GTime &&b) noexcept
{
    if (this != &b) {
        swap(b);
        b.mTime = 0;
    }
    return *this;
}

void GTime::swap(GTime &b) noexcept
{
    std::swap(b.mType, this->mType);
    std::swap(b.mTime, this->mTime);
}

void GTime::resetToSystemClock()
{
    mType = SystemClock;
    update();
}

void GTime::resetToSteadyClock()
{
    mType = SteadyClock;
    update();
}

void GTime::update()
{
    mTime = mType == SystemClock
                ? std::chrono::duration_cast<TimeNanoSec>((std::chrono::system_clock::now()).time_since_epoch()).count()
                : std::chrono::duration_cast<TimeNanoSec>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void GTime::reset()
{
    mTime = 0;
}

GTime::TimeType GTime::millisecond() const
{
    return std::chrono::duration_cast<TimeMilliSec>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::nanosecond() const
{
    return mTime;
}

GTime::TimeType GTime::microsecond() const
{
    return std::chrono::duration_cast<TimeMicroSec>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::second() const
{
    return std::chrono::duration_cast<TimeSec>(TimeNanoSec(mTime)).count();
}

double GTime::secondD() const
{
    return std::chrono::duration_cast<TimeSecD>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::minute() const
{
    return std::chrono::duration_cast<TimeMinute>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::hour() const
{
    return std::chrono::duration_cast<TimeHour>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::day() const
{
    return std::chrono::duration_cast<TimeDay>(TimeNanoSec(mTime)).count();
}

GTime::TimeType GTime::milliSecsTo(const GTime &other) const
{
    return this->millisecond() - other.millisecond();
}

GTime::TimeType GTime::microSecsTo(const GTime &other) const
{
    return this->microsecond() - other.microsecond();
}

GTime::TimeType GTime::nanoSecsTo(const GTime &other) const
{
    return this->nanosecond() - other.nanosecond();
}

GTime::TimeType GTime::secsTo(const GTime &other) const
{
    return this->second() - other.second();
}

double GTime::secsDTo(const GTime &other) const
{
    return this->secondD() - other.secondD();
}

void GTime::addMilliSecs(TimeType ms)
{
    this->mTime += (ms * 1000000L);
}

void GTime::addSecs(TimeType s)
{
    this->mTime += (s * 1000000000L);
}

void GTime::addMicroSecs(TimeType ms)
{
    this->mTime += (ms * 1000L);
}

void GTime::addNanoSecs(TimeType ns)
{
    this->mTime += ns;
}

std::string GTime::toString(const std::string &format, bool utc) const
{
    const TimeType msecs = this->millisecond();
    time_t tTime = msecs / 1000;
    struct tm tmv{};
    if (utc) {
#if GX_PLATFORM_WINDOWS
        ::gmtime_s(&tmv, &tTime);
#else
        ::gmtime_r(&tTime, &tmv);
#endif
    } else {
#if GX_PLATFORM_WINDOWS
        ::localtime_s(&tmv, &tTime);
#else
        ::localtime_r(&tTime, &tmv);
#endif
    }
    const int64_t onlyMs = msecs % 1000;
    GString formatMs = GString::toString(onlyMs);
    if (onlyMs / 10 == 0) {
        formatMs.insert(0, "0");
    }
    if (onlyMs / 100 == 0) {
        formatMs.insert(0, "0");
    }
    GString _format = format;
    _format = _format.replace("yyyy", GString::toString(tmv.tm_year + 1900))
                     .replace(
                         "yy", ((tmv.tm_year + 1900) % 100) / 10 ? GString::toString((tmv.tm_year + 1900) % 100) : GString("0") + GString::toString((tmv.tm_year + 1900) % 100))
                     .replace("MM", tmv.tm_mon >= 9
                                        ? GString::toString(tmv.tm_mon + 1)
                                        : GString("0") +
                                          GString::toString(tmv.tm_mon + 1))
                     .replace("M", GString::toString(tmv.tm_mon + 1))
                     .replace("dd", tmv.tm_mday / 10 ? GString::toString(tmv.tm_mday) : GString("0") + GString::toString(tmv.tm_mday))
                     .replace("d", GString::toString(tmv.tm_mday))
                     .replace("HH", tmv.tm_hour / 10 ? GString::toString(tmv.tm_hour) : GString("0") + GString::toString(tmv.tm_hour))
                     .replace("H", GString::toString(tmv.tm_hour))
                     .replace("hh", (tmv.tm_hour % 12) / 10
                                        ? GString::toString(tmv.tm_hour % 12)
                                        : GString("0")
                                          + GString::toString(tmv.tm_hour % 12))
                     .replace("h", GString::toString(tmv.tm_hour % 12))
                     .replace("mm", tmv.tm_min / 10 ? GString::toString(tmv.tm_min) : GString("0") + GString::toString(tmv.tm_min))
                     .replace("m", GString::toString(tmv.tm_min))
                     .replace("ss", tmv.tm_sec / 10 ? GString::toString(tmv.tm_sec) : GString("0") + GString::toString(tmv.tm_sec))
                     .replace("s", GString::toString(tmv.tm_sec))
                     .replace("zzz", formatMs)
                     .replace("z", GString::toString(onlyMs))
                     .replace("ap", tmv.tm_hour < 12 ? "am" : "pm")
                     .replace("AP", tmv.tm_hour < 12 ? "AM" : "PM");
    return _format.toStdString();
}

bool GTime::operator==(const GTime &rh) const
{
    return mTime == rh.mTime;
}

bool GTime::operator!=(const GTime &rh) const
{
    return !(*this == rh);
}

bool GTime::operator<(const GTime &rh) const
{
    return mTime < rh.mTime;
}

bool GTime::operator>(const GTime &rh) const
{
    return mTime > rh.mTime;
}

bool GTime::operator<=(const GTime &rh) const
{
    return mTime <= rh.mTime;
}

bool GTime::operator>=(const GTime &rh) const
{
    return mTime >= rh.mTime;
}

GTime GTime::currentSystemTime()
{
    GTime t = GTime(SystemClock);
    return t;
}

GTime GTime::currentSteadyTime()
{
    GTime t = GTime(SteadyClock);
    return t;
}
