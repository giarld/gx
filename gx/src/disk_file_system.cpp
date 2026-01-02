//
// Created by Gxin on 25-1-24.
//

#include "disk_file_system.h"


GDiskFileSystem *getDiskFileSystem()
{
    static GDiskFileSystem diskFileSystem;
    return &diskFileSystem;
}

GDiskFileSystem::GDiskFileSystem()
{
}

GDiskFileSystem::~GDiskFileSystem()
{
}

GString GDiskFileSystem::formatPath(GString path) const
{
    if (path.isEmpty()) {
        return "";
    }
    path = path.replace("\\", FILE_SEPARATOR);
    if (path.endWith(FILE_SEPARATOR)) {
        path = path.left(path.length() - 1);
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
        if (path.endWith(":")) {
            path += FILE_SEPARATOR;
        }
#else
        if (path.isEmpty()) {
            path += FILE_SEPARATOR;
        }
#endif
    }
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    if (path.length() == 2 && path.endWith(":")) {
        path += FILE_SEPARATOR;
    }
#endif

    while (path.indexOf("//") >= 0) {
        path = path.replace("//", FILE_SEPARATOR);
    }

    return path;
}

GString GDiskFileSystem::absoluteFilePath(GString path) const
{
    if (path.isEmpty()) {
        return "";
    }
    GString absString;
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    wchar_t wAbsPath[MAX_ABS_PATH];
    _wfullpath(wAbsPath, path.toUtf16().data(), MAX_ABS_PATH);
    absString = GString(GWString(wAbsPath));
#elif GX_PLATFORM_POSIX
    char absPath[PATH_MAX];
    if (realpath(path.c_str(), absPath)) {
        absString = absPath;
    } else {
        absString = path;
    }
#else
#error "Unsupported platform!!"
#endif
    return formatPath(absString);
}

bool GDiskFileSystem::exists(const GString &filePath) const
{
    GString path(filePath);
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    return _waccess(path.toUtf16().data(), 0) == 0;
#else
    struct stat fstat{};
    return stat(path.c_str(), &fstat) == 0;
#endif
}

bool GDiskFileSystem::isFile(const GString &filePath) const
{
    GString path(filePath);

    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.toUtf16().data(), &findData);

    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return true;
    }
    return false;
#else
    struct stat fstat{};
    stat(path.c_str(), &fstat);
    return S_ISREG(fstat.st_mode);
#endif
}

bool GDiskFileSystem::isDirectory(const GString &filePath) const
{
    GString path(filePath);

    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.toUtf16().data(), &findData);

    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }
    return false;
#else
    struct stat fstat{};
    stat(path.c_str(), &fstat);
    return S_ISDIR(fstat.st_mode);
#endif
}

bool GDiskFileSystem::isRelative(const GString &filePath) const
{
    GString path(filePath);

#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    return path.at(1) != ":";
#else
    return !path.startWith(FILE_SEPARATOR);
#endif
}

bool GDiskFileSystem::isRoot(const GString &filePath) const
{
    if (!isDirectory(filePath)) {
        return false;
    }

    GString path(filePath);
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    return path.length() == 3 && path.at(1) == ":";
#else
    return path.length() == 1 && path.at(0) == FILE_SEPARATOR;
#endif
}

bool GDiskFileSystem::isReadOnly(const GString &filePath) const
{
    GString path(filePath);
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.toUtf16().data(), &findData);

    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
        return true;
    }
    return false;
#else
    struct stat fstat{};
    if (stat(path.c_str(), &fstat) != -1) {
        return (fstat.st_mode & S_IWRITE) != S_IWRITE && (fstat.st_mode & S_IREAD) == S_IREAD;
    }
    return false;
#endif
}

bool GDiskFileSystem::setReadOnly(const GString &filePath, bool isReadOnly)
{
    GString path(filePath);

    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    return SetFileAttributesW(path.toUtf16().data(), isReadOnly ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL) == 0;
#else
    struct stat fstat{};
    if (stat(path.c_str(), &fstat) != -1) {
        if (isReadOnly) {
            fstat.st_mode &= ~S_IWUSR;
        } else {
            fstat.st_mode |= S_IWUSR;
        }
        return chmod(path.c_str(), fstat.st_mode) == 0;
    }
    return false;
#endif
}

GTime GDiskFileSystem::createTime(const GString &filePath) const
{
    GTime time(GTime::SystemClock, 0);
    if (!exists(filePath)) {
        return time;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(filePath.toUtf16().data(), &fstat);
    time.addSecs(fstat.st_ctime);
    return time;
#else
    struct stat fstat{};
    stat(filePath.c_str(), &fstat);
    time.addSecs(fstat.st_ctime);
    return time;
#endif
}

GTime GDiskFileSystem::lastModifiedTime(const GString &filePath) const
{
    GTime time(GTime::SystemClock, 0);
    if (!exists(filePath)) {
        return time;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(filePath.toUtf16().data(), &fstat);
    time.addSecs(fstat.st_mtime);
    return time;
#else
    struct stat fstat{};
    stat(filePath.c_str(), &fstat);
    time.addSecs(fstat.st_mtime);
    return time;
#endif
}

GTime GDiskFileSystem::lastAccessTime(const GString &filePath) const
{
    GTime time(GTime::SystemClock, 0);
    if (!exists(filePath)) {
        return time;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(filePath.toUtf16().data(), &fstat);
    time.addSecs(fstat.st_atime);
    return time;
#else
    struct stat fstat{};
    stat(filePath.c_str(), &fstat);
    time.addSecs(fstat.st_atime);
    return time;
#endif
}

bool GDiskFileSystem::remove(const GString &filePath) const
{
    if (!exists(filePath)) {
        return false;
    }
    errno = 0;
#if GX_PLATFORM_WINDOWS | GX_PLATFORM_WINRT
    if (isDirectory(filePath)) {
        return _wrmdir(filePath.toUtf16().data()) == 0;
    }
    return _wremove(filePath.toUtf16().data()) == 0;
#else
    return ::remove(filePath.c_str()) == 0;
#endif
}

bool GDiskFileSystem::rename(const GString &filePath, const GString &newFileName)
{
    if (!exists(filePath)) {
        return false;
    }
    if (exists(newFileName)) {
        return false;
    }
    errno = 0;
#if GX_PLATFORM_WINDOWS | GX_PLATFORM_WINRT
    return _wrename(filePath.toUtf16().data(), newFileName.toUtf16().data()) == 0;
#else
    return ::rename(filePath.c_str(), newFileName.c_str()) == 0;
#endif
}

bool GDiskFileSystem::mkdir(const GString &filePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }
    errno = 0;
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    return _wmkdir(filePath.toUtf16().data()) == 0;
#else
    return ::mkdir(filePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
}

bool GDiskFileSystem::mkdirs(const GString &filePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }
    int begin = 0;
    while (begin >= 0) {
        const int p = filePath.indexOf(FILE_SEPARATOR, begin);
        if (p < 0) {
            break;
        }
        GString pPath = filePath.left(p);

        if (!exists(pPath)) {
            mkdir(pPath);
        } else if (exists(pPath) && !isDirectory(pPath)) {
            return false;
        }

        begin = p + 1;
    }
    if (begin < filePath.length()) {
        return mkdir(filePath);
    }
    return exists(filePath) && isDirectory(filePath);
}

int64_t GDiskFileSystem::fileSize(const GString &filePath) const
{
    if (!isFile(filePath)) {
        return 0;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(filePath.toUtf16().data(), &fstat);
    return fstat.st_size;
#else
    struct stat fstat{};
    stat(filePath.c_str(), &fstat);
    return fstat.st_size;
#endif
}

std::vector<GString> GDiskFileSystem::listFiles(const GString &filePath) const
{
    if (!exists(filePath)) {
        return {};
    }
    std::vector<GString> files;
    if (isDirectory(filePath)) {
#if GX_PLATFORM_WINDOWS
        WIN32_FIND_DATAW findData;
        const GString strFindFile = filePath + FILE_SEPARATOR + "*";
        HANDLE hFind = FindFirstFileW(strFindFile.toUtf16().data(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            return {};
        }
        wchar_t wstr[MAX_PATH];
        do {
            memcpy(wstr, findData.cFileName, sizeof(wchar_t) * MAX_PATH);
            GString cFileName(wstr);
            if (cFileName == "." || cFileName == "..") {
                continue;
            }

            files.emplace_back(cFileName);
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
#else
        DIR *dir;
        struct dirent *_dirent;
        dir = opendir(filePath.c_str());
        if (!dir) {
            return files;
        } else {
            while ((_dirent = readdir(dir)) != nullptr) {
                GString name = _dirent->d_name;
                if (name != "." && name != "..") {
                    files.emplace_back(name);
                }
            }
            closedir(dir);
        }
#endif
    }
    return files;
}

void *GDiskFileSystem::open(const GString &filePath, const GString &mode)
{
    if (filePath.isEmpty()) {
        return nullptr;
    }
    errno = 0;

    FILE *filePtr = nullptr;

#if GX_PLATFORM_WINDOWS
    const GString cMode(mode);
    _wfopen_s(&filePtr, filePath.toUtf16().data(), cMode.toUtf16().data());
#else
    filePtr = ::fopen(filePath.c_str(), mode.c_str());
#endif
    return filePtr;
}

bool GDiskFileSystem::flush(void *filePtr)
{
    errno = 0;
    return fflush(static_cast<FILE *>(filePtr)) == 0;
}

void GDiskFileSystem::close(void *filePtr)
{
    errno = 0;
    fclose(static_cast<FILE *>(filePtr));
}

int64_t GDiskFileSystem::read(void *filePtr, void *buffer, int64_t elementSize, int64_t count)
{
    errno = 0;
    return fread(buffer, elementSize, count, static_cast<FILE *>(filePtr));
}

GString GDiskFileSystem::readLine(void *filePtr)
{
    FILE *cFP = static_cast<FILE *>(filePtr);

    int64_t bufferSize = 1024;
    const auto buffer = static_cast<char *>(malloc(bufferSize));

    int64_t len = 0;
    int c;
    while ((c = fgetc(cFP)) != EOF && static_cast<char>(c) != '\0' && static_cast<char>(c) != '\n') {
        if (len >= bufferSize) {
            bufferSize = len + 64;
        }
        buffer[len++] = static_cast<char>(c);
    }

    buffer[len] = '\0';

    GString str(buffer);
    free(buffer);
    return str;
}

bool GDiskFileSystem::atEnd(void *filePtr) const
{
    return feof(static_cast<FILE *>(filePtr)) != 0;
}

int64_t GDiskFileSystem::write(void *filePtr, const void *buffer, int64_t elementSize, int64_t count)
{
    errno = 0;
    return fwrite(buffer, elementSize, count, static_cast<FILE *>(filePtr));
}

bool GDiskFileSystem::seek(void *filePtr, int64_t offset, int origin)
{
    errno = 0;
    return fseek(static_cast<FILE *>(filePtr), offset, origin) == 0;
}

int64_t GDiskFileSystem::tell(void *filePtr) const
{
    errno = 0;
    return ftell(static_cast<FILE *>(filePtr));
}
