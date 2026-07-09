//
// Created by Gxin on 2022/6/2.
//

#include "gx/os.h"

#include <gx/platform.h>

#include <gx/gstring.h>
#include <gx/debug.h>


#if GX_PLATFORM_WINDOWS

#include <windows.h>

#elif !GX_PLATFORM_IOS

#include <dlfcn.h>

#endif

namespace gx::os
{
void *dlOpen(const std::string &path)
{
#if GX_PLATFORM_WINDOWS
    GWString wStr = GString(path).toUtf16();
    return (void *) ::LoadLibraryW(wStr.data());
#elif GX_PLATFORM_IOS
    return nullptr;
#else
    return ::dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif
}

void dlClose(void *handle)
{
#if GX_PLATFORM_WINDOWS
    ::FreeLibrary((HMODULE) handle);
#elif GX_PLATFORM_IOS
    return;
#else
    ::dlclose(handle);
#endif
}

void *dlSym(void *handle, const std::string &symbol)
{
#if GX_PLATFORM_WINDOWS
    return (void *) ::GetProcAddress((HMODULE) handle, symbol.c_str());
#elif GX_PLATFORM_IOS
    return nullptr;
#else
    return ::dlsym(handle, symbol.c_str());
#endif
}

std::string getEnv(const std::string &name)
{
#if GX_PLATFORM_WINDOWS
    const int32_t bufferSize = 512;
    std::vector<wchar_t> buffer(bufferSize);

    GWString wName(name);
    DWORD result = GetEnvironmentVariableW(wName.data(), buffer.data(), bufferSize);
    if (result == 0) {
        LogW("");
        return "";
    }
    if (bufferSize < result) {
        buffer.clear();
        buffer.resize(result);
        result = GetEnvironmentVariableW(wName.data(), buffer.data(), result);
        if (!result) {
            return "";
        }
    }
    return GString(buffer.data()).toStdString();
#else
    char *env = getenv(name.c_str());
    if (env) {
        return env;
    }
    return "";
#endif
}

static bool fillPath(GString &path, int depth = 0)
{
#if GX_PLATFORM_WINDOWS
    int32_t pIndex = path.indexOf("%");
    if (pIndex < 0) {
        return true;
    }
    if (depth >= 3) {
        return false;
    }

    int32_t pEnd = path.indexOf("%", pIndex + 1);
    if (pEnd < 0) {
        return true;
    }

    GString flag = path.substring(pIndex, pEnd - pIndex + 1);
    GString envKey = path.substring(pIndex + 1, pEnd - pIndex - 1);
    path = path.replace(flag, getEnv(envKey.toStdString()));
    return fillPath(path, depth + 1);
#else
    return true;
#endif
}

std::vector<std::string> getEnvPATH()
{
    GString PATH = getEnv("PATH");

#if GX_PLATFORM_WINDOWS
    std::string sep = ";";
#else
    std::string sep = ":";
#endif
    auto paths = PATH.split(sep);
    std::vector<std::string> ret;
    ret.reserve(paths.size());
    for (auto &p: paths) {
        if (!p.isEmpty() && fillPath(p)) {
            ret.push_back(p.toStdString());
        }
    }
    return ret;
}

std::string name()
{
#if GX_PLATFORM_WINDOWS
    return "Windows";
#elif GX_PLATFORM_LINUX
    return "Linux";
#elif GX_PLATFORM_OSX
    return "macOS";
#elif GX_PLATFORM_IOS
    return "iOS";
#elif GX_PLATFORM_UWP
    return "UWP";
#elif GX_PLATFORM_ANDROID
    return "Android";
#elif GX_PLATFORM_BSD
    return "BSD";
#elif GX_PLATFORM_EMSCRIPTEN
    return "Web";
#else
    return "Unknown";
#endif
}

std::string cpuName()
{
#ifdef GX_CPU_NAME
    return GX_CPU_NAME;
#else
    return "Unknown";
#endif
}

std::string archName()
{
#ifdef GX_ARCH_NAME
    return GX_ARCH_NAME;
#else
    return "Unknown";
#endif
}

int execute(const std::string &cmd)
{
#if GX_PLATFORM_WINDOWS
    GWString wCmd = GString(cmd).toUtf16();

    std::vector<wchar_t> comSpec(MAX_PATH);
    DWORD size = ::GetEnvironmentVariableW(L"COMSPEC", comSpec.data(), static_cast<DWORD>(comSpec.size()));
    if (size == 0 || size >= comSpec.size()) {
        const GWString defaultCmd(L"C:\\Windows\\System32\\cmd.exe");
        comSpec.assign(defaultCmd.data(), defaultCmd.data() + defaultCmd.length());
        comSpec.push_back(L'\0');
    }
    std::wstring cmdPath(comSpec.data());
    std::wstring commandLine = L"\"" + cmdPath + L"\" /d /s /c \"" + std::wstring(wCmd.data(), wCmd.length()) + L"\"";

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};
    if (!::CreateProcessW(cmdPath.c_str(), commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo)) {
        return -1;
    }

    const DWORD waitResult = ::WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = static_cast<DWORD>(-1);
    if (waitResult != WAIT_FAILED) {
        ::GetExitCodeProcess(processInfo.hProcess, &exitCode);
    }
    ::CloseHandle(processInfo.hThread);
    ::CloseHandle(processInfo.hProcess);
    return static_cast<int>(exitCode);
#elif GX_PLATFORM_OS_DESKTOP
    return ::system(cmd.c_str());
#else
    return -1;
#endif
}
}
