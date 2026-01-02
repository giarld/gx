//
// Created by Gxin on 2022/6/2.
//

#ifndef GX_OS_H
#define GX_OS_H

#include "gx/gglobal.h"

#include <string>
#include <vector>


namespace gx::os
{
GX_API void *dlOpen(const std::string &path);

GX_API void dlClose(void *handle);

GX_API void *dlSym(void *handle, const std::string &symbol);

GX_API std::string getEnv(const std::string &name);

GX_API std::vector<std::string> getEnvPATH();

/**
 * @brief Get operating system name
 * @return Windows|Linux|macOS|iOS|UWP|Android|BSD|Web
 */
GX_API std::string name();

/**
 * @brief Get CPU type name
 * @return ARM|MIPS|PowerPC|RISC-V|x86
 */
GX_API std::string cpuName();

/**
 * @brief Get CPU architecture bit width name
 * @return 32-bit|64-bit
 */
GX_API std::string archName();

GX_API int execute(const std::string &cmd);
}

#endif //GX_OS_H
