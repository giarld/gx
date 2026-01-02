//
// Created by Gxin on 24-4-17.
//

#include "gx/zipfile.h"

#include <zip.h>
#include <unzip.h>

#include <gx/gstring.h>

#include <algorithm>

#if GX_PLATFORM_WINDOWS

#include <windows.h>

#elif GX_PLATFORM_LINUX || GX_PLATFORM_BSD || GX_PLATFORM_ANDROID || GX_PLATFORM_OSX || GX_PLATFORM_IOS
#include <iconv.h>
#endif


#define UNZ_BUFFER_SIZE 8192
#define ZIP_BUFFER_SIZE 16384

static GFile::OpenModeFlags translateOpenMode(int mode)
{
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ) {
        return GFile::ReadOnly;
    }
    if (!!(mode & ZLIB_FILEFUNC_MODE_EXISTING) || !!(mode & ZLIB_FILEFUNC_MODE_CREATE)) {
        return GFile::ReadWrite;
    }
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_WRITE) {
        return GFile::WriteOnly;
    }

    return 0;
}

static voidpf uzOpen64File(voidpf opaque, const void *filename, int mode)
{
    GFile *file;
    if (opaque) {
        file = static_cast<GFile *>(opaque);
    } else {
        file = GX_NEW(GFile, static_cast<const char *>(filename));
    }

    if (!file) {
        return nullptr;
    }

    if (!!(mode & ZLIB_FILEFUNC_MODE_EXISTING) && !file->exists()) {
        return nullptr;
    }

    const GFile::OpenModeFlags openMode = translateOpenMode(mode) | GFile::Binary;
    if (file->open(openMode)) {
        return file;
    }

    GX_DELETE(file);
    return nullptr;
}

static uLong uzReadFile(voidpf, voidpf stream, void *buf, uLong size)
{
    auto *file = static_cast<GFile *>(stream);
    if (!file) {
        return 0;
    }

    return file->read(static_cast<char *>(buf), size);
}

static uLong uzWriteFile(voidpf, voidpf stream, const void *buf, uLong size)
{
    auto *file = static_cast<GFile *>(stream);
    if (!file) {
        return 0;
    }

    return file->write(static_cast<const char *>(buf), size);
}

static ZPOS64_T uzTell64File(voidpf, voidpf stream)
{
    const auto *file = static_cast<GFile *>(stream);
    if (!file) {
        return 0;
    }

    return file->tell();
}

static long uzSeek64File(voidpf, voidpf stream, ZPOS64_T offset, int origin)
{
    auto *file = static_cast<GFile *>(stream);
    if (!file) {
        return -1;
    }

    int _origin;
    switch (origin) {
        case ZLIB_FILEFUNC_SEEK_CUR:
            _origin = SEEK_CUR;
            break;
        case ZLIB_FILEFUNC_SEEK_END:
            _origin = SEEK_END;
            break;
        case ZLIB_FILEFUNC_SEEK_SET:
            _origin = SEEK_SET;
            break;
        default:
            return -1;
    }

    return file->seek(static_cast<int64_t>(offset), _origin) ? 0 : -1;
}

static int uzCloseFile(voidpf, voidpf stream)
{
    auto *file = static_cast<GFile *>(stream);
    if (!file) {
        return -1;
    }

    file->close();
    GX_DELETE(file);

    return 0;
}

static int uzTestErrorFile(voidpf, voidpf stream)
{
    const auto *file = static_cast<GFile *>(stream);
    if (!file) {
        return -1;
    }
    const int ret = ferror(static_cast<FILE *>(file->getFilePtr()));
    return ret;
}

static unsigned long getFileCrc(const GByteArray &data)
{
    unsigned long calculateCrc = 0;
    int64_t sizeRead;
    void *buf = ::malloc(8192);

    data.seekReadPos(SEEK_SET, 0);
    do {
        sizeRead = data.read(buf, 8192);

        if (sizeRead > 0) {
            calculateCrc = crc32_z(calculateCrc, static_cast<const uint8_t *>(buf), sizeRead);
        }
    } while (sizeRead > 0);

    data.seekReadPos(SEEK_SET, 0);

    ::free(buf);

    return calculateCrc;
}

/// ================ ZipReader ================

GxZipReader::GxZipReader() = default;

GxZipReader::~GxZipReader()
{
    close();
}

GxZipReader::GxZipReader(GxZipReader &&rhs) noexcept
{
    std::swap(this->mZipPtr, rhs.mZipPtr);
    std::swap(this->mPassword, rhs.mPassword);
    std::swap(this->mFileInfos, rhs.mFileInfos);
}

GxZipReader &GxZipReader::operator=(GxZipReader &&rhs) noexcept
{
    if (this != &rhs) {
        std::swap(this->mZipPtr, rhs.mZipPtr);
        std::swap(this->mPassword, rhs.mPassword);
        std::swap(this->mFileInfos, rhs.mFileInfos);
    }
    return *this;
}

bool GxZipReader::open(const GFile &file, const std::string &password)
{
    close();

    if (!file.isFile()) {
        return false;
    }

    zlib_filefunc64_def fileFunc{};
    fileFunc.zopen64_file = uzOpen64File;
    fileFunc.zread_file = uzReadFile;
    fileFunc.zwrite_file = uzWriteFile;
    fileFunc.ztell64_file = uzTell64File;
    fileFunc.zseek64_file = uzSeek64File;
    fileFunc.zclose_file = uzCloseFile;
    fileFunc.zerror_file = uzTestErrorFile;

    mZipPtr = unzOpen2_64(file.filePath().c_str(), &fileFunc);
    if (mZipPtr == nullptr) {
        return false;
    }
    mPassword = password;

    return true;
}

void GxZipReader::close()
{
    if (mZipPtr) {
        mFileInfos.clear();
        mFileInfoIndex.clear();

        unzClose(mZipPtr);
        mZipPtr = nullptr;
    }
}

bool GxZipReader::isOpen() const
{
    return mZipPtr != nullptr;
}

int64_t GxZipReader::getFileCount() const
{
    if (!mZipPtr) {
        return 0;
    }

    unz_global_info64 globalInfo64{};
    unzGetGlobalInfo64(mZipPtr, &globalInfo64);

    return static_cast<int64_t>(globalInfo64.number_entry);
}

std::vector<GxZipReader::FileInfo> GxZipReader::getFileInfos()
{
    unz_global_info64 globalInfo64{};
    unzGetGlobalInfo64(mZipPtr, &globalInfo64);

    if (mFileInfos.size() == globalInfo64.number_entry) {
        return mFileInfos;
    }

    unz_file_info64 fileInfo64{};
    char szFileName[256];

    mFileInfos.clear();
    mFileInfoIndex.clear();
    mFileInfos.reserve(globalInfo64.number_entry);

    unzGoToFirstFile(mZipPtr);

    for (int32_t i = 0; i < globalInfo64.number_entry; i++) {
        const int ret = unzGetCurrentFileInfo64(mZipPtr, &fileInfo64, szFileName, 256, nullptr, 0, nullptr, 0);
        if (ret != UNZ_OK) {
            break;
        }

        GString fileName(szFileName);
        std::string stdFileName = fileName.toStdString();
        mFileInfos.push_back({
            .fileName = stdFileName,
            .isDirectory = fileName.endWith(FILE_SEPARATOR),
            .compressedSize = fileInfo64.compressed_size,
            .uncompressedSize = fileInfo64.uncompressed_size,
            .crc = fileInfo64.crc,
            .timeStamp = GTime(fileInfo64.tmu_date.tm_year, fileInfo64.tmu_date.tm_mon + 1,
                               fileInfo64.tmu_date.tm_mday,
                               fileInfo64.tmu_date.tm_hour, fileInfo64.tmu_date.tm_min,
                               fileInfo64.tmu_date.tm_sec),
            .fileOffset = unzGetOffset64(mZipPtr)
        });
        mFileInfoIndex[stdFileName] = mFileInfos.size() - 1;

        unzGoToNextFile(mZipPtr);
    }
    unzGoToFirstFile(mZipPtr);

    return mFileInfos;
}

GxZipReader::FileInfo GxZipReader::getFileInfo(const std::string &fileName)
{
    if (mFileInfos.empty()) {
        getFileInfos();
    }

    const auto findIt = mFileInfoIndex.find(fileName);
    if (findIt != mFileInfoIndex.end()) {
        const size_t index = findIt->second;
        if (index < mFileInfos.size()) {
            return mFileInfos[index];
        }
    }

    return {};
}

GByteArray GxZipReader::readFile(const std::string &fileName)
{
    const auto fileInfo = getFileInfo(fileName);

    GByteArray data;

    if (fileInfo.isDirectory) {
        return data;
    }

    // Seek to file
    if (unzSetOffset64(mZipPtr, fileInfo.fileOffset) != UNZ_OK) {
        return data;
    }

    void *buf = ::malloc(UNZ_BUFFER_SIZE);

    int rSize;
    if (fileInfo.fileName == fileName) {
        const char *passwd = mPassword.empty() ? nullptr : mPassword.c_str();
        if (unzOpenCurrentFilePassword(mZipPtr, passwd) != UNZ_OK) {
            goto end;
        }

        do {
            rSize = unzReadCurrentFile(mZipPtr, buf, UNZ_BUFFER_SIZE);
            if (rSize < 0) {
                data.clear();
                break;
            }
            if (rSize > 0) {
                data.write(buf, rSize);
            }
        } while (rSize > 0);

        unzCloseCurrentFile(mZipPtr);
    }

end:
    ::free(buf);

    return data;
}

/// ================ ZipWriter ================

GxZipWriter::GxZipWriter() = default;

GxZipWriter::~GxZipWriter()
{
    close();
}

GxZipWriter::GxZipWriter(GxZipWriter &&rhs) noexcept
{
    std::swap(this->mZipPtr, rhs.mZipPtr);
    std::swap(this->mPassword, rhs.mPassword);
}

GxZipWriter &GxZipWriter::operator=(GxZipWriter &&rhs) noexcept
{
    if (this != &rhs) {
        std::swap(this->mZipPtr, rhs.mZipPtr);
        std::swap(this->mPassword, rhs.mPassword);
    }
    return *this;
}

bool GxZipWriter::open(const GFile &file, const std::string &password)
{
    close();

    if (file.exists() && !file.isFile()) {
        return false;
    }

    const bool createNew = !file.exists();
    const int append = createNew ? 0 : 1;

    zlib_filefunc64_32_def fileFuncDef{};
    fileFuncDef.zfile_func64.zopen64_file = uzOpen64File;
    fileFuncDef.zfile_func64.zread_file = uzReadFile;
    fileFuncDef.zfile_func64.zwrite_file = uzWriteFile;
    fileFuncDef.zfile_func64.ztell64_file = uzTell64File;
    fileFuncDef.zfile_func64.zseek64_file = uzSeek64File;
    fileFuncDef.zfile_func64.zclose_file = uzCloseFile;
    fileFuncDef.zfile_func64.zerror_file = uzTestErrorFile;

    mZipPtr = zipOpen3(file.filePath().c_str(), append, nullptr, &fileFuncDef);
    if (mZipPtr == nullptr) {
        return false;
    }
    mPassword = password;

    return true;
}

void GxZipWriter::close()
{
    if (mZipPtr) {
        zipClose(mZipPtr, nullptr);
        mZipPtr = nullptr;
    }
}

bool GxZipWriter::isOpen() const
{
    return mZipPtr != nullptr;
}

bool GxZipWriter::writeFile(const std::string &fileName, const GByteArray &data, int32_t compressLevel)
{
    if (!mZipPtr) {
        return false;
    }

    const char *tPasswd = mPassword.empty() ? nullptr : mPassword.c_str();

    const int zip64 = data.size() >= 0xffffffff ? 1 : 0;

    const unsigned long crcFile = getFileCrc(data);

    zip_fileinfo zipFileinfo{};

    const GTime currentTime = GTime::currentSystemTime();
    const GString timeStr = currentTime.toString("yyyy-MM-dd-HH-mm-ss");
    const auto timeSplit = timeStr.split("-");

    zipFileinfo.tmz_date.tm_year = std::stoi(timeSplit[0].toString());
    zipFileinfo.tmz_date.tm_mon = std::stoi(timeSplit[1].toString()) - 1;
    zipFileinfo.tmz_date.tm_mday = std::stoi(timeSplit[2].toString());
    zipFileinfo.tmz_date.tm_hour = std::stoi(timeSplit[3].toString());
    zipFileinfo.tmz_date.tm_min = std::stoi(timeSplit[4].toString());
    zipFileinfo.tmz_date.tm_sec = std::stoi(timeSplit[5].toString());

    if (compressLevel < 0) {
        compressLevel = 6;
    }
    compressLevel = std::clamp(compressLevel, 0, 9);

    int err = zipOpenNewFileInZip3_64(mZipPtr, fileName.c_str(), &zipFileinfo,
                                      nullptr, 0, nullptr, 0, nullptr /* comment*/,
                                      (compressLevel != 0) ? Z_DEFLATED : 0,
                                      compressLevel, 0,
                                      -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                      tPasswd, crcFile, zip64);

    if (err != ZIP_OK) {
        return false;
    }

    int64_t sizeRead;
    void *buf = ::malloc(ZIP_BUFFER_SIZE);

    data.seekReadPos(SEEK_SET, 0);
    do {
        sizeRead = data.read(buf, ZIP_BUFFER_SIZE);

        if (sizeRead > 0) {
            err = zipWriteInFileInZip(mZipPtr, buf, static_cast<unsigned>(sizeRead));
        }
    } while (err == ZIP_OK && sizeRead > 0);

    data.seekReadPos(SEEK_SET, 0);

    ::free(buf);

    if (err != ZIP_OK) {
        return false;
    }

    if (zipCloseFileInZip(mZipPtr) != ZIP_OK) {
        return false;
    }

    return true;
}
