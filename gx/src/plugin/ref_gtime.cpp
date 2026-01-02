//
// Created by Gxin on 2022/6/5.
//

#include <gx/gany.h>

#include "gx/gtime.h"


using namespace gany;

void refGTime()
{
    Class<GTime::Type>("Gx", "TimeType", "")
            .defEnum({
                         {"SystemClock", GTime::Type::SystemClock},
                         {"SteadyClock", GTime::Type::SteadyClock}
                     })
            .func(MetaFunction::ToString, [](GTime::Type &self) {
                switch (self) {
                    case GTime::SystemClock:
                        return "SystemClock";
                    case GTime::SteadyClock:
                        return "SteadyClock";
                }
                return "";
            })
            REF_ENUM_OPERATORS(GTime::Type);

    Class<GTime>("Gx", "GTime", "Gx time")
            .construct<>()
            .construct<GTime::Type>()
            .construct<GTime::Type, int64_t>()
            .construct<GTime &>()
            .inherit<GObject>()
            .defEnum({
                         {"SystemClock", GTime::Type::SystemClock},
                         {"SteadyClock", GTime::Type::SteadyClock}
                     })
            .func("swap", &GTime::swap, {"Swap two GTime objects.", {"other"}})
            .func("resetToSystemClock", &GTime::resetToSystemClock,
                  {"Reset the clock to the system clock and update it to the latest time."})
            .func("resetToSteadyClock", &GTime::resetToSteadyClock,
                  {"Reset the clock to a steady clock and update it to the latest time."})
            .func("update", &GTime::update, {"Update the clock to the latest time of the corresponding type."})
            .func("reset", &GTime::reset, {"Reset time to 0."})
            .func("millisecond", &GTime::millisecond, {"Get milliseconds of time."})
            .func("nanosecond", &GTime::nanosecond, {"Get nanoseconds of time."})
            .func("microsecond", &GTime::microsecond, {"Get microseconds of time."})
            .func("second", &GTime::second, {"Get seconds of time."})
            .func("secondD", &GTime::secondD, {"Gets the number of seconds of time, in double numbers."})
            .func("minute", &GTime::minute, {"Get minutes of time."})
            .func("hour", &GTime::hour, {"Get hour of time."})
            .func("day", &GTime::day, {"Get day of time."})
            .func("milliSecsTo", &GTime::milliSecsTo,
                  {"Calculate the millisecond difference between two times.", {"other"}})
            .func("microSecsTo", &GTime::microSecsTo,
                  {"Calculate the microsecond difference between two times.", {"other"}})
            .func("nanoSecsTo", &GTime::nanoSecsTo,
                  {"Calculate the nanosecond difference between two times.", {"other"}})
            .func("secsTo", &GTime::secsTo,
                  {"Calculate the second difference between two times.", {"other"}})
            .func("secsDTo", &GTime::secsDTo,
                  {"Calculate the second(double) difference between two times.", {"other"}})
            .func("addMilliSecs", &GTime::addMilliSecs,
                  {"Increase the specified number of milliseconds.", {"ms"}})
            .func("addSecs", &GTime::addSecs,
                  {"Increase the specified number of seconds.", {"s"}})
            .func("addMicroSecs", &GTime::addMicroSecs,
                  {"Increase the specified number of microseconds.", {"ms"}})
            .func("addNanoSecs", &GTime::addNanoSecs,
                  {"Increase the specified number of nanoseconds.", {"ns"}})
            .func("toString", [](const GTime &t, const std::string &format) {
                return t.toString(format);
            }, {"Format time is a string, only for SystemClock.", {"format"}})
            .func("toString", [](const GTime &t, const std::string &format, bool utc) {
                return t.toString(format, utc);
            }, {"Format time is a string, only for SystemClock.", {"format", "utc"}})
            .func(MetaFunction::Subtraction, [](const GTime &t1, const GTime &t2) {
                return t1 - t2;
            }, {"Calculate the difference between two times, the two times must be of the same type."})
            .func(MetaFunction::Addition, [](const GTime &t1, const GTime &t2) {
                      return t1 + t2;
                  },
                  {"To calculate the sum of two times, the two times must be of the same type.", {"t1", "t2"}})
            .func(MetaFunction::EqualTo, &GTime::operator==)
            .func(MetaFunction::LessThan, &GTime::operator<)
            .staticFunc("currentSystemTime", &GTime::currentSystemTime, {"Get current system time."})
            .staticFunc("currentSteadyTime", &GTime::currentSteadyTime, {"Get current steady time."});
}
