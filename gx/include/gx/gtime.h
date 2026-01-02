//
// Created by Gxin on 2020/6/19.
//

#ifndef GX_GTIME_H
#define GX_GTIME_H

#include "gobject.h"
#include "gstring.h"
#include "debug.h"

#include <ctime>


/**
 * @class GTime
 * @brief Time class, providing time acquisition and operation for system clock and steady clock,
 * providing mathematical operations for various time units,
 * and supporting output in text format in the current time zone or UTC time.
 */
class GX_API GTime final : public GObject
{
public:
    using TimeType = int64_t;

public:
    /**
     * 时钟类型
     */
    enum Type
    {
        SystemClock, ///< System clock
        SteadyClock  ///< Steady clock
    };

public:
    explicit GTime(Type type = SystemClock, TimeType nanosecond = -1);

    explicit GTime(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second);

    ~GTime() override;

    GTime(const GTime &b);

    GTime(GTime &&b) noexcept;

    GTime &operator=(const GTime &b);

    GTime &operator=(GTime &&b) noexcept;

public:
    void swap(GTime &b) noexcept;

    /**
     * @brief Reset the time unit to the system clock, and the time count will be reset to zero
     */
    void resetToSystemClock();

    /**
     * @brief Resetting the time unit to a stable clock will reset the time count to zero
     */
    void resetToSteadyClock();

    /**
     * @brief Update the time to the current time of the corresponding clock
     */
    void update();

    /**
     * @brief Reset time to 0
     */
    void reset();

    /**
     * @brief Returns time in milliseconds
     * @return
     */
    TimeType millisecond() const;

    /**
     * @brief Returns time in nanoseconds
     * @return
     */
    TimeType nanosecond() const;

    /**
     * @brief Returns time in microseconds
     * @return
     */
    TimeType microsecond() const;

    /**
     * @brief Returns time in seconds
     * @return
     */
    TimeType second() const;

    /**
     * @brief Returns time in seconds (floating-point type)
     * @return
     */
    double secondD() const;

    /**
     * @brief Returns time in minutes
     * @return
     */
    TimeType minute() const;

    /**
     * @brief Return time in hours
     * @return
     */
    TimeType hour() const;

    /**
     * @brief Return time in days
     * @return
     */
    TimeType day() const;

    /**
     * @brief Returns the millisecond difference between the current time and the incoming time
     * @param other
     * @return
     */
    TimeType milliSecsTo(const GTime &other) const;

    /**
     * @brief Returns the microsecond difference between the current time and the incoming time
     * @param other
     * @return
     */
    TimeType microSecsTo(const GTime &other) const;

    /**
     * @brief Returns the nanosecond difference between the current time and the incoming time
     * @param other
     * @return
     */
    TimeType nanoSecsTo(const GTime &other) const;

    /**
     * @brief Returns the second difference between the current time and the incoming time
     * @param other
     * @return
     */
    TimeType secsTo(const GTime &other) const;

    /**
     * @brief Returns the second difference between the current time and the incoming time (floating-point type)
     * @param other
     * @return
     */
    double secsDTo(const GTime &other) const;

    /**
     * @brief Increase or decrease the clock by ms milliseconds
     * @param ms
     */
    void addMilliSecs(TimeType ms);

    /**
     * @brief Increase or decrease the clock by s seconds
     * @param s
     */
    void addSecs(TimeType s);

    /**
     * @brief Increase or decrease the clock by ms microseconds
     * @param ms
     */
    void addMicroSecs(TimeType ms);

    /**
     * @brief Increase or decrease the clock by ns nanoseconds
     * @param ns
     */
    void addNanoSecs(TimeType ns);

    /**
     * Format as String <br>
     * Supported expressions: <br>
     * yyyy   0-9999, representing the year of the current time <br>
     * yy     00-99, representing the last two digits of the current time year <br>
     * MM     00-12, representing the month of the current time <br>
     * M      0-12, representing the month of the current time <br>
     * dd     00-31, representing the current time in days <br>
     * d      0-31, representing the current time in days <br>
     * HH     00-23, representing the hour of the current time (in 24-hour format) <br>
     * H      0-23, representing the hour of the current time (in 24-hour format) <br>
     * hh     00-12, representing the hour of the current time (in 12 hour format) <br>
     * h      0-12, representing the hour of the current time (in 12 hour format) <br>
     * mm     00-59, representing the minutes of the current time <br>
     * m      0-59, representing the minutes of the current time <br>
     * ss     00-59, representing the current time in seconds <br>
     * s      0-59, representing the current time in seconds <br>
     * zzz    000-999, representing the current time in milliseconds <br>
     * z      0-999, representing the current time in milliseconds <br>
     * ap     am/pm, indicating morning or afternoon <br>
     * AM     AM/PM, indicating morning or afternoon <br>
     * <br>
     * ex: <br>
     * "yyyy-MM-dd HH:mm:ss.zzz AP" <br>
     *
     * @param format
     * @param utc   Is it UTC time
     * @return
     */
    std::string toString(const std::string &format, bool utc = false) const;

    std::string toString() const override
    {
        if (mType == SystemClock) {
            return toString("yyyy-MM-dd HH:mm:ss.zzz");
        }
        return std::to_string(nanosecond());
    }

    friend GTime operator-(const GTime &a, const GTime &b) noexcept
    {
        GX_ASSERT(a.mType == b.mType);
        GTime c(a.mType);
        c.mTime = std::abs(a.mTime - b.mTime);
        return c;
    }

    friend GTime operator+(const GTime &a, const GTime &b) noexcept
    {
        GX_ASSERT(a.mType == b.mType);
        GTime c(a.mType);
        c.mTime = std::abs(a.mTime + b.mTime);
        return c;
    }

    bool operator==(const GTime &rh) const;

    bool operator!=(const GTime &rh) const;

    bool operator<(const GTime &rh) const;

    bool operator>(const GTime &rh) const;

    bool operator<=(const GTime &rh) const;

    bool operator>=(const GTime &rh) const;

public:
    /**
     * @brief Returns the current system time
     * @return
     */
    static GTime currentSystemTime();

    /**
     * @brief Return to current steady time
     * @return
     */
    static GTime currentSteadyTime();

private:
    Type mType;
    TimeType mTime; // Nanosecond
};

#endif //GX_GTIME_H
