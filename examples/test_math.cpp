//
// Created by Gxin on 2020/5/7.
//

#include <gx/debug.h>

#include <gx/gtime.h>

#include <gx/math/math_la.h>


int main(int argc, char *argv[])
{
#if GX_PLATFORM_WINDOWS
    system("chcp 65001");
#endif
    math::float2 a{2.123, 3.159};
    math::int2 b{2};

    math::float2 c = a + b;

    Log("c: {}, {}", c.x, c.y);

    return EXIT_SUCCESS;
}