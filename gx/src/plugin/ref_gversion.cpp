//
// Created by Gxin on 2022/7/18.
//

#include <gx/gversion.h>

#include "gx/gany.h"


using namespace gany;

void refGVersion()
{
    Class<GVersion>("Gx", "GVersion", "Gx version number conversion and comparison tool")
            .construct<uint16_t, uint16_t, uint16_t, uint16_t>()
            .construct<const std::string &>()
            .inherit<GObject>()
            .func("toUInt64", &GVersion::toUInt64)
            .func("getMajor", &GVersion::getMajor)
            .func("getMinor", &GVersion::getMinor)
            .func("getPatch", &GVersion::getPatch)
            .func("getBuild", &GVersion::getBuild)
            .func(MetaFunction::EqualTo, &GVersion::operator==)
            .func(MetaFunction::LessThan, &GVersion::operator<);
}
