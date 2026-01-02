//
// Created by Gxin on 2020/5/7.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/debug.h>
#include <gx/reg_gx.h>
#include <gx/reg_gx_math.h>
#include <gx/gtime.h>
#include <gx/gfile.h>


class A
{
public:
    explicit A(const std::string &name)
        : name(name)
    {
    }

public:
    std::string name;
};

class B : public A
{
public:
    explicit B(const std::string &name)
        : A(name)
    {
    }
};


void func(const std::shared_ptr<A> &obj)
{
    Log(">>> {}", obj->name);
}

int main(int argc, char *argv[])
{
#if GX_PLATFORM_WINDOWS
    system("chcp 65001>nul");
#endif

    initGAnyCore();

    GANY_LOAD_MODULE(Gx);

    GAny func = [](const GString &s) {
        Log(">> {}", s);
    };

    GAny ret = func("Hello");
    if (ret.isException()) {
        LogE("{}", ret.toString());
    }

    return EXIT_SUCCESS;
}
