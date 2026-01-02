//
// Created by Gxin on 2020/3/9.
//

#include "gx/gfile.h"
#include "disk_file_system.h"

#include <gx/gany.h>


GFile::GFile(const std::string &path, GFileSystem_I *fs)
{
    mFileSystem = fs == nullptr ? getDiskFileSystem() : fs;

    mPath = mFileSystem->formatPath(path);

#if GX_PLATFORM_ANDROID || GX_PLATFORM_OSX || GX_PLATFORM_IOS
    if (dynamic_cast<GDiskFileSystem *>(mFileSystem) != nullptr) {
        if (mPath.isEmpty() || mPath == ".") {
            mPath = mainDirectory().mPath;
        } else if (mPath.startWith("./")) {
            mPath = mainDirectory().mPath + "/" + mPath.right(mPath.length() - 2);
            mPath = mFileSystem->formatPath(mPath);
        }
    }
#endif
}

GFile::GFile(const GFile &parent, const std::string &child)
{
    mFileSystem = parent.mFileSystem;

    mPath = parent.mPath;
    mPath.append(FILE_SEPARATOR).append(child);
    mPath = mFileSystem->formatPath(mPath);
}

GFile::GFile(const GFile &other)
{
    mFileSystem = other.mFileSystem;
    mPath = mFileSystem->formatPath(other.mPath);
}

GFile::GFile(GFile &&other) noexcept
{
    swap(other);
}

GFile::~GFile()
{
    close();
}

GFile &GFile::operator=(GFile &&other) noexcept
{
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void GFile::setFile(const std::string &path)
{
    GFile f(path);
    this->swap(f);
}

void GFile::setFile(const GFile &parent, const std::string &child)
{
    GFile f(parent, child);
    this->swap(f);
}

void GFile::swap(GFile &file) noexcept
{
    std::swap(mFileSystem, file.mFileSystem);

    std::swap(this->mFilePtr, file.mFilePtr);
    mPath.swap(file.mPath);
}

std::string GFile::toString() const
{
    return absoluteFilePath();
}

bool GFile::exists() const
{
    return mFileSystem->exists(mPath);
}

bool GFile::isFile() const
{
    return mFileSystem->isFile(mPath);
}

bool GFile::isDirectory() const
{
    return mFileSystem->isDirectory(mPath);
}

bool GFile::isRelative() const
{
    return mFileSystem->isRelative(mPath);
}

bool GFile::isRoot() const
{
    return mFileSystem->isRoot(mPath);
}

bool GFile::isReadOnly() const
{
    return mFileSystem->isReadOnly(mPath);
}

bool GFile::setReadOnly(bool isReadOnly) const
{
    return mFileSystem->setReadOnly(mPath, isReadOnly);
}

GTime GFile::createTime() const
{
    return mFileSystem->createTime(mPath);
}

GTime GFile::lastModifiedTime() const
{
    return mFileSystem->lastModifiedTime(mPath);
}

GTime GFile::lastAccessTime() const
{
    return mFileSystem->lastAccessTime(mPath);
}

std::string GFile::filePath() const
{
    return this->mPath.toStdString();
}

std::string GFile::fileName() const
{
    if (isRoot()) {
        return this->mPath.toStdString();
    }
    const int lastSepIndex = this->mPath.lastIndexOf(FILE_SEPARATOR);
    if (lastSepIndex < 0) {
        return this->mPath.toStdString();
    }
    return this->mPath.substring(lastSepIndex + 1).toStdString();
}

std::string GFile::fileNameWithoutExtension() const
{
    const GString fileName = this->fileName();
    const int32_t findDot = fileName.lastIndexOf(".");
    if (findDot > 0) {
        return fileName.substring(0, findDot).toStdString();
    }
    return fileName.toStdString();
}

std::string GFile::fileSuffix() const
{
    if (isDirectory()) {
        return "";
    }
    const GString fileName = this->fileName();

    const int32_t findDot = fileName.lastIndexOf(".");
    const int32_t findSep = fileName.lastIndexOf(FILE_SEPARATOR);
    if (findDot > findSep) {
        const GString suffix = fileName.substring(findDot + 1, -1);
        return suffix.toStdString();
    }
    return "";
}

std::string GFile::absoluteFilePath() const
{
    return mFileSystem->absoluteFilePath(mPath).toStdString();
}

std::string GFile::absolutePath() const
{
    const GString absFilePath = this->absoluteFilePath();
    if (isDirectory()) {
        return absFilePath.toStdString();
    }
    const int lastSepIndex = absFilePath.lastIndexOf(FILE_SEPARATOR);
    return absFilePath.left(lastSepIndex).toStdString();
}

GFile GFile::parent() const
{
    if (mPath.isEmpty()) {
        return GFile();
    }
    const GString absPath = this->absolutePath();
    if (!isDirectory() || isRoot()) {
        return GFile(absPath.toStdString());
    }
    const int lastSepIndex = absPath.lastIndexOf(FILE_SEPARATOR);
    return GFile(absPath.left(lastSepIndex).toStdString());
}

bool GFile::remove() const
{
    return mFileSystem->remove(mPath);
}

bool GFile::rename(GFile &newFile)
{
    return mFileSystem->rename(mPath, newFile.mPath);
}

GFile GFile::concat(const GFile &b)
{
    if (!b.isRelative()) {
        return b;
    }
    if (b.mPath.isEmpty()) {
        return *this;
    }
    return GFile(*this, b.mPath.toStdString());
}

GFile GFile::concat(const std::string &b)
{
    return concat(GFile(b));
}

std::vector<GFile> GFile::listFiles(const FileFilter &filter) const
{
    std::vector<GString> filePaths = mFileSystem->listFiles(absoluteFilePath());

    std::vector<GFile> files;
    for (const auto &path : filePaths) {
        GFile nf(*this, path.toStdString());
        if (!filter || filter(nf)) {
            files.emplace_back(nf);
        }
    }

    return files;
}

bool GFile::mkdir() const
{
    return mFileSystem->mkdir(mPath);
}

bool GFile::mkdirs() const
{
    return mFileSystem->mkdirs(mPath);
}

bool GFile::open(OpenModeFlags mode)
{
    const std::string cMode = transOpenMode(mode);
    return open(cMode);
}

bool GFile::open(const std::string &mode)
{
    mFilePtr = mFileSystem->open(mPath, mode);
    return mFilePtr != nullptr;
}

bool GFile::flush()
{
    if (mFilePtr) {
        return mFileSystem->flush(mFilePtr);
    }
    return false;
}

void GFile::close()
{
    if (mFilePtr) {
        mFileSystem->close(mFilePtr);
    }
    mFilePtr = nullptr;
}

bool GFile::isOpen() const
{
    return mFilePtr != nullptr;
}

int64_t GFile::fileSize() const
{
    return mFileSystem->fileSize(mPath);
}

int64_t GFile::read(void *buffer, int64_t elementSize, int64_t count)
{
    if (mFilePtr) {
        return mFileSystem->read(mFilePtr, buffer, elementSize, count);
    }
    return -1;
}

int64_t GFile::read(char *buffer, int64_t maxSize)
{
    return this->read(buffer, sizeof(char), maxSize);
}

GByteArray GFile::read(int64_t maxSize)
{
    if (!isOpen()) {
        return GByteArray();
    }
    maxSize = maxSize > 0 ? maxSize : fileSize();
    GByteArray buffer(maxSize);
    maxSize = read(reinterpret_cast<char *>(buffer.ptr()), maxSize);
    buffer.seekWritePos(SEEK_SET, static_cast<int32_t>(maxSize));
    return buffer;
}

GString GFile::readAll()
{
    seek(0, SEEK_SET);

    const GByteArray all = read();
    GString str(reinterpret_cast<const char *>(all.data()), static_cast<int32_t>(all.size()));

    return str;
}

GString GFile::readLine()
{
    return mFileSystem->readLine(mFilePtr);
}

bool GFile::atEnd() const
{
    return mFileSystem->atEnd(mFilePtr);
}

int64_t GFile::write(const void *buffer, int64_t elementSize, int64_t count)
{
    if (mFilePtr) {
        return mFileSystem->write(mFilePtr, buffer, elementSize, count);
    }
    return -1;
}

int64_t GFile::write(const char *buffer, int64_t size)
{
    return write(buffer, sizeof(char), size);
}

int64_t GFile::write(const GByteArray &data)
{
    return write(reinterpret_cast<const char *>(data.data()), data.size());
}

int64_t GFile::write(const GString &str)
{
    return write(str.data(), str.count());
}

bool GFile::seek(int64_t offset, int origin)
{
    if (mFilePtr) {
        return mFileSystem->seek(mFilePtr, offset, origin);
    }
    return false;
}

bool GFile::seek(int64_t offset)
{
    return this->seek(offset, SEEK_CUR);
}

int64_t GFile::tell() const
{
    return mFileSystem->tell(mFilePtr);
}

void *GFile::getFilePtr() const
{
    return mFilePtr;
}

std::string GFile::transOpenMode(OpenModeFlags mode)
{
    const bool isBinary = (mode & Binary) == Binary;
    const bool isAppend = (mode & Append) == Append;
    if ((mode & ReadWrite) == ReadWrite) {
        if (isAppend) {
            return isBinary ? "ab+" : "at+";
        }
        return isBinary ? "wb+" : "wt+";
    }
    if ((mode & ReadOnly) == ReadOnly) {
        return isBinary ? "rb" : "rt";
    }
    if ((mode & WriteOnly) == WriteOnly) {
        if (isAppend) {
            return isBinary ? "ab" : "at";
        }
        return isBinary ? "wb" : "wt";
    }
    return isBinary ? "rb" : "rt";
}

#if !GX_PLATFORM_OSX && !GX_PLATFORM_IOS

GFile GFile::mainDirectory()
{
#if GX_PLATFORM_WINDOWS
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    const GString path(buffer);
    GFile file(path.toStdString());
    return file.parent();
#elif GX_PLATFORM_LINUX
    char buffer[PATH_MAX];
    readlink("/proc/self/exe", buffer, PATH_MAX);
    GFile file(buffer);
    return file.parent();
#elif GX_PLATFORM_ANDROID
    GAny clazzAndroidBridge = GAny::Import("AndroidBridge");
    if (!clazzAndroidBridge.isClass()) {
        return GFile("./");
    }
    std::string dirPath = clazzAndroidBridge.call("getMainDirectory").toString();
    return GFile(dirPath);
#else
    return GFile("./");
#endif
}

GFile GFile::temporaryDirectory()
{
#if GX_PLATFORM_WINDOWS
    wchar_t buffer[MAX_PATH];
    GetTempPathW(MAX_PATH, buffer);
    const GString path(buffer);
    return GFile(path.toStdString());
#elif GX_PLATFORM_LINUX
    return GFile("/tmp/");
#elif GX_PLATFORM_ANDROID
    GAny clazzAndroidBridge = GAny::Import("AndroidBridge");
    if (!clazzAndroidBridge.isClass()) {
        return GFile("./");
    }
    std::string dirPath = clazzAndroidBridge.call("getTemporaryDirectory").toString();
    return GFile(dirPath);
#else
    return GFile("./");
#endif
}

#endif
