//
// Created by Gxin on 24-4-17.
//

#ifndef GX_ZIP_FILE_H
#define GX_ZIP_FILE_H

#include "gobject.h"

#include "gbytearray.h"
#include "gfile.h"
#include "gtime.h"

#include <map>
#include <string>
#include <vector>


class GX_API GxZipReader final : public GObject
{
public:
    struct FileInfo
    {
        std::string fileName;
        bool isDirectory{};
        uint64_t compressedSize{};
        uint64_t uncompressedSize{};
        uint64_t crc{};
        GTime timeStamp{};
        uint64_t fileOffset{};
    };

public:
    explicit GxZipReader();

    ~GxZipReader() override;

    GxZipReader(const GxZipReader &) = delete;

    GxZipReader(GxZipReader &&rhs) noexcept;

    GxZipReader &operator=(const GxZipReader &) = delete;

    GxZipReader &operator=(GxZipReader &&rhs) noexcept;

public:
    bool open(const GFile &file, const std::string &password = "");

    void close();

    bool isOpen() const;

    int64_t getFileCount() const;

    std::vector<FileInfo> getFileInfos();

    FileInfo getFileInfo(const std::string &fileName);

    GByteArray readFile(const std::string &fileName);

public:
    void *mZipPtr = nullptr;
    std::string mPassword;
    std::vector<FileInfo> mFileInfos;
    std::map<std::string, size_t> mFileInfoIndex;
};

class GX_API GxZipWriter final : public GObject
{
public:
    explicit GxZipWriter();

    ~GxZipWriter() override;

    GxZipWriter(const GxZipWriter &) = delete;

    GxZipWriter(GxZipWriter &&rhs) noexcept;

    GxZipWriter &operator=(const GxZipWriter &) = delete;

    GxZipWriter &operator=(GxZipWriter &&rhs) noexcept;

public:
    bool open(const GFile &file, const std::string &password = "");

    void close();

    bool isOpen() const;

    bool writeFile(const std::string &fileName, const GByteArray &data, int32_t compressLevel = -1);

private:
    void *mZipPtr = nullptr;
    std::string mPassword;
};

#endif //GX_ZIP_FILE_H
