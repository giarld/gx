//
// Created by Gxin on 2022/6/4.
//

#include <gx/gany.h>

#include "gx/gobject.h"


using namespace gany;

void refGObject()
{
    Class<GObject>("Gx", "GObject", "Gx base class")
            .construct<>()
            .func(MetaFunction::ToString, &GObject::toString);
}
