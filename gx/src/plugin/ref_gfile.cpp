//
// Created by Gxin on 2022/6/5.
//

#include <gx/gany.h>

#include "gx/gfile.h"


using namespace gany;

void refGFile()
{
    Class<GFileSystem_I>("Gx", "GFileSystem_I", "Class GFileSystem_I.")
        .func("formatPath", &GFileSystem_I::formatPath, {
                  .doc = "Format the provided file path.",
                  .args = {"path"}
              })
        .func("exists", &GFileSystem_I::exists, {
                  .doc = "Check if the file or directory exists.",
                  .args = {"filePath"}
              })
        .func("isFile", &GFileSystem_I::isFile, {
                  .doc = "Check if the path corresponds to a file.",
                  .args = {"filePath"}
              })
        .func("isDirectory", &GFileSystem_I::isDirectory, {
                  .doc = "Check if the path corresponds to a directory.",
                  .args = {"filePath"}
              })
        .func("isRelative", &GFileSystem_I::isRelative, {
                  .doc = "Check if the path is relative.",
                  .args = {"filePath"}
              })
        .func("isRoot", &GFileSystem_I::isRoot, {
                  .doc = "Check if the path is a root directory.",
                  .args = {"filePath"}
              })
        .func("isReadOnly", &GFileSystem_I::isReadOnly, {
                  .doc = "Check if the file is read-only.",
                  .args = {"filePath"}
              })
        .func("setReadOnly", &GFileSystem_I::setReadOnly, {
                  .doc = "Set the read-only attribute of the file.",
                  .args = {"filePath", "isReadOnly"}
              })
        .func("createTime", &GFileSystem_I::createTime, {
                  .doc = "Get the creation time of the file.",
                  .args = {"filePath"}
              })
        .func("lastModifiedTime", &GFileSystem_I::lastModifiedTime, {
                  .doc = "Get the last modified time of the file.",
                  .args = {"filePath"}
              })
        .func("lastAccessTime", &GFileSystem_I::lastAccessTime, {
                  .doc = "Get the last access time of the file.",
                  .args = {"filePath"}
              })
        .func("remove", &GFileSystem_I::remove, {
                  .doc = "Delete the specified file or directory.",
                  .args = {"filePath"}
              })
        .func("rename", &GFileSystem_I::rename, {
                  .doc = "Rename the file to the new name.",
                  .args = {"filePath", "newFileName"}
              })
        .func("mkdir", &GFileSystem_I::mkdir, {
                  .doc = "Create a directory.",
                  .args = {"filePath"}
              })
        .func("mkdirs", &GFileSystem_I::mkdirs, {
                  .doc = "Create directories along the specified path.",
                  .args = {"filePath"}
              })
        .func("fileSize", &GFileSystem_I::fileSize, {
                  .doc = "Get the size of the file.",
                  .args = {"filePath"}
              })
        .func("listFiles", &GFileSystem_I::listFiles, {
                  .doc = "List files in the specified directory.",
                  .args = {"filePath"}
              })
        .func("open", &GFileSystem_I::open, {
                  .doc = "Open a file with the specified mode.",
                  .args = {"filePath", "mode"}
              })
        .func("flush", &GFileSystem_I::flush, {
                  .doc = "Flush the file stream.",
                  .args = {"filePtr"}
              })
        .func("close", &GFileSystem_I::close, {
                  .doc = "Close the file stream.",
                  .args = {"filePtr"}
              })
        .func("read", &GFileSystem_I::read, {
                  .doc = "Read data from the file stream.",
                  .args = {"filePtr", "buffer", "elementSize", "count"}
              })
        .func("readLine", &GFileSystem_I::readLine, {
                  .doc = "Read a line from the file stream.",
                  .args = {"filePtr"}
              })
        .func("atEnd", &GFileSystem_I::atEnd, {
                  .doc = "Check if the file stream has reached the end.",
                  .args = {"filePtr"}
              })
        .func("write", &GFileSystem_I::write, {
                  .doc = "Write data to the file stream.",
                  .args = {"filePtr", "buffer", "elementSize", "count"}
              })
        .func("seek", &GFileSystem_I::seek, {
                  .doc = "Seek to a position in the file stream.",
                  .args = {"filePtr", "offset", "origin"}
              })
        .func("tell", &GFileSystem_I::tell, {
                  .doc = "Get the current position in the file stream.",
                  .args = {"filePtr"}
              });

    Class<GFile::OpenMode>("Gx", "GFileOpenMode", "")
        .defEnum({
            {"ReadOnly", static_cast<int32_t>(GFile::OpenMode::ReadOnly)},
            {"WriteOnly", static_cast<int32_t>(GFile::OpenMode::WriteOnly)},
            {"ReadWrite", static_cast<int32_t>(GFile::OpenMode::ReadWrite)},
            {"Append", static_cast<int32_t>(GFile::OpenMode::Append)},
            {"Binary", static_cast<int32_t>(GFile::OpenMode::Binary)}
        })
        .func(MetaFunction::ToString, [](GFile::OpenMode &self) {
            switch (self) {
                case GFile::ReadOnly:
                    return "ReadOnly";
                case GFile::WriteOnly:
                    return "WriteOnly";
                case GFile::ReadWrite:
                    return "ReadWrite";
                case GFile::Append:
                    return "Append";
                case GFile::Binary:
                    return "Binary";
            }
            return "";
        })
        REF_ENUM_OPERATORS(GFile::OpenMode);

    Class<GFile>("Gx", "GFile", "Gx file.")
        .construct<>()
        .construct<const std::string &>({.doc = "Construct from file path.", .args = {"path"}})
        .construct<const GFile &, const std::string &>(
            {.doc = "Construct from file path and child path.", .args = {"parent", "child"}})
        .construct<const std::string &, GFileSystem_I *>({.doc = "Construct from file path.", .args = {"path", "fs"}})
        .construct<const GFile &>({.doc = "Construct from file.", .args = {"file"}})
        .inherit<GObject>()
        .constant("FILE_SEPARATOR", FILE_SEPARATOR)
        .defEnum({
            {"ReadOnly", static_cast<int32_t>(GFile::OpenMode::ReadOnly)},
            {"WriteOnly", static_cast<int32_t>(GFile::OpenMode::WriteOnly)},
            {"ReadWrite", static_cast<int32_t>(GFile::OpenMode::ReadWrite)},
            {"Append", static_cast<int32_t>(GFile::OpenMode::Append)},
            {"Binary", static_cast<int32_t>(GFile::OpenMode::Binary)}
        })
        .defEnum({
            {"SEEK_SET", SEEK_SET},
            {"SEEK_CUR", SEEK_CUR},
            {"SEEK_END", SEEK_END}
        })
        .func("setFile", [](GFile &self, const std::string &path) {
            self.setFile(path);
        }, {.doc = "Set file by path.", .args = {"path"}})
        .func("setFile", [](GFile &self, const GFile &parent, const std::string &child) {
            self.setFile(parent, child);
        }, {.doc = "Set the file through the file path and the path.", .args = {"parent", "child"}})
        .func("swap", &GFile::swap, {.doc = "Swap two objects.", .args = {"other"}})
        .func("exists", [](const GFile &self) {
            return self.exists();
        }, {.doc = "Check if the file exists."})
        .func("isFile", [](const GFile &self) {
            return self.isFile();
        }, {.doc = "Check if the file is a file."})
        .func("isDirectory", [](const GFile &self) {
            return self.isDirectory();
        }, {.doc = "Check if the file is a directory."})
        .func("isRelative", &GFile::isRelative, {.doc = "Check if the directory is relative."})
        .func("isRoot", &GFile::isRoot, {.doc = "Check if the directory is root."})
        .func("isReadOnly", [](const GFile &self) {
            return self.isReadOnly();
        }, {.doc = "Check the file or directory is read-only."})
        .func("setReadOnly", [](GFile &self, bool isReadOnly) {
            return self.setReadOnly(isReadOnly);
        }, {.doc = "Modify the read-only attribute of a file or directory", .args = {"isReadOnly"}})
        .func("createTime", [](GFile &self) {
            return self.createTime();
        }, {.doc = "Get the creation time of the file."})
        .func("lastModifiedTime", [](GFile &self) {
            return self.lastModifiedTime();
        }, {.doc = "Get the last modified time of the file."})
        .func("lastAccessTime", [](GFile &self) {
            return self.lastAccessTime();
        }, {.doc = "Get the last accessed time of the file."})
        .func("filePath", &GFile::filePath, {.doc = "Get the file path."})
        .func("fileName", &GFile::fileName, {.doc = "Get the file name."})
        .func("fileNameWithoutExtension", &GFile::fileNameWithoutExtension,
              {.doc = "Get the file name without extension."})
        .func("fileSuffix", &GFile::fileSuffix, {.doc = "Get the file suffix."})
        .func("absoluteFilePath", &GFile::absoluteFilePath, {.doc = "Get the absolute file path."})
        .func("absolutePath", &GFile::absolutePath, {.doc = "Get the absolute path."})
        .func("parent", &GFile::parent, {.doc = "Get the parent directory."})
        .func("remove", &GFile::remove,
              {.doc = "Delete a file or directory. The directory must be empty when deleting a directory."})
        .func("rename", &GFile::rename, {.doc = "Rename a file or directory.", .args = {"newFile"}})
        .func("concat", [](GFile &self, const GFile &b) {
            return self.concat(b);
        }, {.doc = "Concatenate two files.", .args = {"b"}})
        .func("concat", [](GFile &self, const std::string &b) {
            return self.concat(b);
        }, {.doc = "Concatenate two files.", .args = {"b"}})
        .func("listFiles", [](GFile &self) {
            return self.listFiles();
        }, {.doc = "List the files in the directory."})
        .func("listFiles", [](GFile &self, GFile::FileFilter filterFunc) {
            return self.listFiles(filterFunc);
        }, {
            .doc = "Get all the files in the directory according to the conditions. "
            "The parameter is a function. The function parameter is GFile. The return value is bool. "
            "If true is returned, the file is returned.",
            .args = {"filterFunc"}
        })
        .func("mkdir", [](GFile &self) {
            return self.mkdir();
        }, {.doc = "Create a directory if it does not exist."})
        .func("mkdirs", [](GFile &self) {
            return self.mkdirs();
        }, {.doc = "Create a directory recursively, if the directory does not exist."})
        .func("open", [](GFile &self, int mode) {
            return self.open(static_cast<GFile::OpenModeFlags>(mode));
        }, {.doc = "Open the file, param: OpenModeFlags.", .args = {"mode"}})
        .func("open", [](GFile &self, const std::string &mode) {
            return self.open(mode);
        }, {.doc = "Open the file, param: mode, C-style file opening mode.", .args = {"mode"}})
        .func("flush", &GFile::flush, {.doc = "Flush the file."})
        .func("close", &GFile::close, {.doc = "Close the file."})
        .func("isOpen", &GFile::isOpen, {.doc = "Check if the file is open."})
        .func("fileSize", &GFile::fileSize, {.doc = "Get the file size."})
        .func("read", [](GFile &self, GAnyBytePtr buffer, int64_t elementSize, int64_t count) {
            return self.read(buffer, elementSize, count);
        }, {.doc = "Read file in bytes.", .args = {"buffer", "elementSize", "count"}})
        .func("read", [](GFile &self, GAnyBytePtr buffer, int64_t maxSize) {
            return self.read(buffer, maxSize);
        }, {.doc = "Read file in bytes", .args = {"buffer", "maxSize"}})
        .func("read", [](GFile &self) {
            return self.read();
        }, {.doc = "Read file as GByteArray."})
        .func("read", [](GFile &self, int64_t maxSize) {
            return self.read(maxSize);
        }, {.doc = "Read file as GByteArray.", .args = {"maxSize"}})
        .func("readAll", [](GFile &self) {
            return self.readAll().toStdString();
        }, {.doc = "Read file as string."})
        .func("readLine", [](GFile &self) {
            return self.readLine().toStdString();
        }, {.doc = "Read file line string."})
        .func("atEnd", &GFile::atEnd, {.doc = "Check if the file is at end."})
        .func("write", [](GFile &self, GAnyConstBytePtr buffer, int64_t elementSize, int64_t count) {
            return self.write(buffer, elementSize, count);
        }, {.doc = "Write file in bytes.", .args = {"buffer", "elementSize", "count"}})
        .func("write", [](GFile &self, GAnyConstBytePtr buffer, int64_t size) {
            return self.write(buffer, size);
        }, {.doc = "Write file in bytes", .args = {"buffer", "size"}})
        .func("write", [](GFile &self, const GByteArray &data) {
            return self.write(data);
        }, {.doc = "Write file.", .args = {"data"}})
        .func("write", [](GFile &self, const GString &str) {
            return self.write(str);
        }, {.doc = "Write file.", .args = {"str"}})
        .func("write", [](GFile &self, const std::string &str) {
            return self.write(str);
        }, {.doc = "Write file.", .args = {"str"}})
        .func("seek", [](GFile &self, int64_t offset, int32_t origin) {
            return self.seek(offset, origin);
        }, {.doc = "Seek the file.", .args = {"offset", "origin"}})
        .func("seek", [](GFile &self, int64_t offset) {
            return self.seek(offset);
        }, {.doc = "Seek the file.", .args = {"offset"}})
        .func("tell", &GFile::tell, {.doc = "Return the location of the current file stream."})
        .staticFunc("mainDirectory", &GFile::mainDirectory, {.doc = "Get the main program directory."})
        .staticFunc("temporaryDirectory", &GFile::temporaryDirectory)
        .func(MetaFunction::Addition, [](const GFile &file, const std::string &child) {
            return file + child;
        }, {.doc = "Concatenate two files.", .args = {"child"}})
        .func(MetaFunction::Addition, [](const GFile &file, const GFile &child) {
            return file + child;
        }, {.doc = "Concatenate two files.", .args = {"child"}})
        .func(MetaFunction::Division, [](const GFile &file, const std::string &child) {
            return file / child;
        })
        .func(MetaFunction::Division, [](const GFile &file, const GFile &child) {
            return file / child;
        });
}
