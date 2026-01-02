//
// Created by Gxin on 24-4-18.
//

#include <gx/gany.h>

#include <gx/zipfile.h>


using namespace gany;

void refZipFile()
{
    Class<GxZipReader::FileInfo>("Gx", "GZipFileInfo", "")
            .construct<>()
            .readOnly("fileName", &GxZipReader::FileInfo::fileName)
            .readOnly("isDirectory", &GxZipReader::FileInfo::isDirectory)
            .readOnly("compressedSize", &GxZipReader::FileInfo::compressedSize)
            .readOnly("uncompressedSize", &GxZipReader::FileInfo::uncompressedSize)
            .readOnly("crc", &GxZipReader::FileInfo::crc)
            .readOnly("timeStamp", &GxZipReader::FileInfo::timeStamp);

    Class<GxZipReader>("Gx", "GZipReader", "Reading data from zip compressed file.")
            .inherit<GObject>()
            .construct<>()
            .func("open", [](GxZipReader &self, const GFile &file, const std::string &password) {
                return self.open(file, password);
            })
            .func("open", [](GxZipReader &self, const GFile &file) {
                return self.open(file);
            })
            .func("close", &GxZipReader::close)
            .func("getFileCount", &GxZipReader::getFileCount)
            .func("getFileInfos", &GxZipReader::getFileInfos)
            .func("getFileInfo", &GxZipReader::getFileInfo)
            .func("readFile", &GxZipReader::readFile);

    Class<GxZipWriter>("Gx", "GZipWriter", "Write data to a zip compressed file.")
            .inherit<GObject>()
            .construct<>()
            .func("open", [](GxZipWriter &self, const GFile &file, const std::string &password) {
                return self.open(file, password);
            })
            .func("open", [](GxZipWriter &self, const GFile &file) {
                return self.open(file);
            })
            .func("close", &GxZipWriter::close)
            .func("writeFile",
                  [](GxZipWriter &self, const std::string &fileName, const GByteArray &data, int32_t compressLevel) {
                      return self.writeFile(fileName, data, compressLevel);
                  })
            .func("writeFile", [](GxZipWriter &self, const std::string &fileName, const GByteArray &data) {
                return self.writeFile(fileName, data);
            });
}
