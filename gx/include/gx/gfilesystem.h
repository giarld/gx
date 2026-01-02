//
// Created by Gxin on 25-1-24.
//

#ifndef GX_IO_DEVICE_H
#define GX_IO_DEVICE_H

#include "gobject.h"

#include "gtime.h"


#define FILE_SEPARATOR  "/"

class GFileSystem_I : public GObject
{
public:
    ~GFileSystem_I() override = default;

    virtual GString formatPath(GString path) const = 0;

    virtual GString absoluteFilePath(GString path) const = 0;

    virtual bool exists(const GString &filePath) const = 0;

    virtual bool isFile(const GString &filePath) const = 0;

    virtual bool isDirectory(const GString &filePath) const = 0;

    virtual bool isRelative(const GString &filePath) const = 0;

    virtual bool isRoot(const GString &filePath) const = 0;

    virtual bool isReadOnly(const GString &filePath) const = 0;

    virtual bool setReadOnly(const GString &filePath, bool isReadOnly) = 0;

    virtual GTime createTime(const GString &filePath) const = 0;

    virtual GTime lastModifiedTime(const GString &filePath) const = 0;

    virtual GTime lastAccessTime(const GString &filePath) const = 0;

    virtual bool remove(const GString &filePath) const = 0;

    virtual bool rename(const GString &filePath, const GString &newFileName) = 0;

    virtual bool mkdir(const GString &filePath) const = 0;

    virtual bool mkdirs(const GString &filePath) const = 0;

    virtual int64_t fileSize(const GString &filePath) const = 0;

    virtual std::vector<GString> listFiles(const GString &filePath) const = 0;

    virtual void *open(const GString &filePath, const GString &mode) = 0;

    virtual bool flush(void *filePtr) = 0;

    virtual void close(void *filePtr) = 0;

    virtual int64_t read(void *filePtr, void *buffer, int64_t elementSize, int64_t count) = 0;

    virtual GString readLine(void *filePtr) = 0;

    virtual bool atEnd(void *filePtr) const = 0;

    virtual int64_t write(void *filePtr, const void *buffer, int64_t elementSize, int64_t count) = 0;

    virtual bool seek(void *filePtr, int64_t offset, int origin) = 0;

    virtual int64_t tell(void *filePtr) const = 0;
};

#endif //GX_IO_DEVICE_H
