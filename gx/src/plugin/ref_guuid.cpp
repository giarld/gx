//
// Created by Gxin on 2022/6/5.
//

#include <gx/gany.h>

#include "gx/guuid.h"

#include "gx/enum.h"


using namespace gany;

DEF_ENUM_OPERATORS(GUuid::FormatType, uint8_t);

void refGUuid()
{
    Class<GUuid::FormatType>("Gx", "GUuidFormatType", "Enum GUuid::FormatType.")
            .defEnum({
                         {"N", GUuid::FormatType::N},
                         {"D", GUuid::FormatType::D},
                         {"B", GUuid::FormatType::B},
                         {"P", GUuid::FormatType::P},
                     })
            .func(MetaFunction::ToString, [](GUuid::FormatType &self) {
                switch (self) {
                    case GUuid::FormatType::N:
                        return "N";
                    case GUuid::FormatType::D:
                        return "D";
                    case GUuid::FormatType::B:
                        return "B";
                    case GUuid::FormatType::P:
                        return "P";
                }
                return "";
            })
            REF_ENUM_OPERATORS(GUuid::FormatType);

    Class<GUuid>("Gx", "GUuid", "Gx uuid.")
            .construct<>()
            .construct<const std::string &>()
            .construct<GUuid &>()
            .inherit<GObject>()
            .func(MetaFunction::ToString, [](GUuid &self) {
                return self.toString();
            })
            .func(MetaFunction::EqualTo, &GUuid::operator==)
            .func("toString", [](GUuid &self, GUuid::FormatType &formatType) {
                return self.toString(formatType);
            })
            .func("isValid", &GUuid::isValid)
            .defEnum({
                         {"N", GUuid::FormatType::N},
                         {"D", GUuid::FormatType::D},
                         {"B", GUuid::FormatType::B},
                         {"P", GUuid::FormatType::P},
                     });
}
