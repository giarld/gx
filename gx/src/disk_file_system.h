//
// Created by Gxin on 25-1-24.
//

#ifndef GX_DISK_FILE_SYSTEM_H
#define GX_DISK_FILE_SYSTEM_H

#include "gx/gfilesystem.h"

#include <sys/stat.h>
#include <fstream>

#if GX_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <dirent.h>

#endif

#if !(GX_COMPILER_MSVC)

#include <cerrno>

#endif

#if GX_PLATFORM_LINUX || GX_PLATFORM_BSD || GX_PLATFORM_ANDROID

#include <unistd.h>

#elif GX_PLATFORM_OSX || GX_PLATFORM_IOS

#include <mach-o/dyld.h>

#endif

#define MAX_ABS_PATH 2048


class GDiskFileSystem : public GFileSystem_I
{
public:
    explicit GDiskFileSystem();

    ~GDiskFileSystem() override;

public:
    GString formatPath(GString path) const override;

    GString absoluteFilePath(GString path) const override;

    bool exists(const GString &filePath) const override;

    bool isFile(const GString &filePath) const override;

    bool isDirectory(const GString &filePath) const override;

    bool isRelative(const GString &filePath) const override;

    bool isRoot(const GString &filePath) const override;

    bool isReadOnly(const GString &filePath) const override;

    bool setReadOnly(const GString &filePath, bool isReadOnly) override;

    GTime createTime(const GString &filePath) const override;

    GTime lastModifiedTime(const GString &filePath) const override;

    GTime lastAccessTime(const GString &filePath) const override;

    bool remove(const GString &filePath) const override;

    bool rename(const GString &filePath, const GString &newFileName) override;

    bool mkdir(const GString &filePath) const override;

    bool mkdirs(const GString &filePath) const override;

    int64_t fileSize(const GString &filePath) const override;

    std::vector<GString> listFiles(const GString &filePath) const override;

    void *open(const GString &filePath, const GString &mode) override;

    bool flush(void *filePtr) override;

    void close(void *filePtr) override;

    int64_t read(void *filePtr, void *buffer, int64_t elementSize, int64_t count) override;

    GString readLine(void *filePtr) override;

    bool atEnd(void *filePtr) const override;

    int64_t write(void *filePtr, const void *buffer, int64_t elementSize, int64_t count) override;

    bool seek(void *filePtr, int64_t offset, int origin) override;

    int64_t tell(void *filePtr) const override;
};

extern GDiskFileSystem *getDiskFileSystem();

#endif //GX_DISK_FILE_SYSTEM_H
