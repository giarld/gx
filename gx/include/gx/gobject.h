//
// Created by Gxin on 2019-07-16.
//

#ifndef GX_GOBJECT_H
#define GX_GOBJECT_H

#include "gx/gglobal.h"

#include <string>
#include <typeinfo>
#include <sstream>


class GObject
{
public:
    explicit GObject() = default;

    virtual ~GObject() = default;

    virtual std::string toString() const
    {
        std::stringstream ss;
        ss << "<" << typeid(*this).name() << " at " << this << ">";
        return ss.str();
    }
};

#endif //GX_GOBJECT_H
