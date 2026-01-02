//
// Created by Gxin on 2023/7/28.
//

#include <gx/gany.h>

#include <gx/os.h>


using namespace gany;

using namespace gx;

void refOs()
{
    auto clazz = GAnyClass::Class("Gx", "Os", "");
    clazz->staticFunc("getEnv", &os::getEnv, {.args = {"name"}})
         .staticFunc("getEnvPATH", &os::getEnvPATH)
         .staticFunc("name", &os::name,
                     {.doc = "Get operating system name. return: Windows|Linux|macOS|iOS|UWP|Android|BSD|Web."})
         .staticFunc("cpuName", &os::cpuName,
                     {.doc = "Get CPU type name. return: ARM|MIPS|PowerPC|RISC-V|x86."})
         .staticFunc("archName", &os::archName,
                     {.doc = "Get CPU architecture bit width name. return: 32-bit|64-bit."})
         .staticFunc("execute", &os::execute, {.args = {"cmd"}});
    GAny::Export(clazz);
}
