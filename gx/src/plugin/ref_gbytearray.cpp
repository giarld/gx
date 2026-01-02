//
// Created by Gxin on 2022/6/5.
//

#include <gx/gany.h>

#include "gx/gbytearray.h"


using namespace gany;

void refGByteArray()
{
    Class<GByteArray::ByteOrder>("Gx", "GByteArrayByteOrder", "")
            .defEnum({
                         {"BigEndian", GByteArray::BigEndian},
                         {"LittleEndian", GByteArray::LittleEndian}
                     })
            .func(MetaFunction::ToString, [](GByteArray::ByteOrder &self) {
                switch (self) {
                    case GByteArray::BigEndian:
                        return "BigEndian";
                    case GByteArray::LittleEndian:
                        return "LittleEndian";
                }
                return "";
            })
            REF_ENUM_OPERATORS(GByteArray::ByteOrder);

    Class<GByteArray>("Gx", "GByteArray", "Gx byte array")
            .construct<>()
            .construct<int64_t>({.doc = "Construct a GByteArray of size.", .args = {"size"}})
            .construct<const uint8_t *, int64_t>(
                {.doc = "Constructing through memory blocks.", .args = {"data:GAnyConstBytePtr", "size"}})
            .construct<const std::vector<uint8_t> &>(
                {.doc = "Constructing through byte array containers.", .args = {"bytes:vector<uint8_t>"}})
            .construct<const GByteArray &>({.doc = "Copy construction.", .args = {"other"}})
            .inherit<GObject>()
            .defEnum({
                         {"BigEndian", GByteArray::BigEndian},
                         {"LittleEndian", GByteArray::LittleEndian}
                     })
            .defEnum({
                         {"Set", SEEK_SET},
                         {"Cur", SEEK_CUR},
                         {"End", SEEK_END}
                     })
            .func("setByteOrder", &GByteArray::setByteOrder,
                  {.doc = "Set bytes order.", .args = {"byteOrder:ByteOrder"}})
            .func("reset", [](GByteArray &self) {
                self.reset();
            }, {.doc = "Reset read and write position."})
            .func("reset", [](GByteArray &self, int64_t size) {
                self.reset(size);
            }, {.doc = "Reset byte array with size, and reset read and write position.", .args = {"size"}})
            .func("capacity", &GByteArray::capacity, {.doc = "Get capacity."})
            .func("size", &GByteArray::size, {.doc = "Get size(write size)."})
            .func("isEmpty", &GByteArray::isEmpty, {.doc = "Check if byte array is empty."})
            .func("data", &GByteArray::data, {.doc = "Get data pointer.", .args = {}, .returnType = "GAnyConstBytePtr"})
            .func("clear", &GByteArray::clear, {.doc = "Clear byte array."})
            .func("write", [](GByteArray &self, GAnyConstBytePtr data, int64_t size) {
                self.write(data, size);
            }, {.doc = "Write data to byte array. arg1: data; arg2: size.", .args = {"data", "size"}})
            .func("writeInt8", [](GByteArray &self, GAny &data) {
                self.write(static_cast<int8_t>(data.toInt32()));
            }, {.doc = "Write int8 to byte array.", .args = {"data"}})
            .func("writeUInt8", [](GByteArray &self, GAny &data) {
                self.write(static_cast<uint8_t>(data.toInt32()));
            }, {.doc = "Write uint8 to byte array.", .args = {"data"}})
            .func("writeInt16", [](GByteArray &self, GAny &data) {
                self.write(static_cast<int16_t>(data.toInt32()));
            }, {.doc = "Write int16 to byte array.", .args = {"data"}})
            .func("writeUInt16", [](GByteArray &self, GAny &data) {
                self.write(static_cast<uint16_t>(data.toInt32()));
            }, {.doc = "Write uint16 to byte array.", .args = {"data"}})
            .func("writeInt32", [](GByteArray &self, GAny &data) {
                self.write(data.toInt32());
            }, {.doc = "Write int32 to byte array.", .args = {"data"}})
            .func("writeUInt32", [](GByteArray &self, GAny &data) {
                self.write(static_cast<uint32_t>(data.toInt32()));
            }, {.doc = "Write uint32 to byte array.", .args = {"data"}})
            .func("writeInt64", [](GByteArray &self, GAny &data) {
                self.write(data.toInt64());
            }, {.doc = "Write int64 to byte array.", .args = {"data"}})
            .func("writeUInt64", [](GByteArray &self, GAny &data) {
                self.write(static_cast<uint64_t>(data.toInt64()));
            }, {.doc = "Write uint64 to byte array.", .args = {"data"}})
            .func("writeBool", [](GByteArray &self, GAny &data) {
                self.write(data.toBool());
            }, {.doc = "Write bool to byte array.", .args = {"data"}})
            .func("writeFloat", [](GByteArray &self, GAny &data) {
                self.write(data.toFloat());
            }, {.doc = "Write float to byte array.", .args = {"data"}})
            .func("writeDouble", [](GByteArray &self, GAny &data) {
                self.write(data.toDouble());
            }, {.doc = "Write double to byte array.", .args = {"data"}})
            .func("writeString", [](GByteArray &self, const std::string &data) {
                self.write(data);
            }, {.doc = "Write string to byte array.", .args = {"data"}})
            .func("writeStringData", [](GByteArray &self, const std::string &data) {
                self.write(data.data(), (int64_t) data.size());
            }, {.doc = "Write a string as a block to byte array.", .args = {"data"}})
            .func("writeBytes", [](GByteArray &self, const GByteArray &data) {
                self.write(data);
            }, {.doc = "Write GByteArray to byte array.", .args = {"data"}})
            .func("writeGAny", [](GByteArray &self, const GAny &data) {
                self.write(data);
            }, {.doc = "Write GAny to byte array.", .args = {"data"}})
            .func("read", [](GByteArray &self, GAnyBytePtr data, int64_t size) {
                self.read(data, size);
            }, {.doc = "Read data from byte array. arg1: data; arg2: size.", .args = {"data", "size"}})
            .func("readInt8", [](GByteArray &self) {
                int8_t v;
                self.read(v);
                return v;
            }, {.doc = "Read int8 from byte array."})
            .func("readUInt8", [](GByteArray &self) {
                uint8_t v;
                self.read(v);
                return v;
            }, {.doc = "Read uint8 from byte array."})
            .func("readInt16", [](GByteArray &self) {
                int16_t v;
                self.read(v);
                return v;
            }, {.doc = "Read int16 from byte array."})
            .func("readUInt16", [](GByteArray &self) {
                uint16_t v;
                self.read(v);
                return v;
            }, {.doc = "Read uint16 from byte array."})
            .func("readInt32", [](GByteArray &self) {
                int32_t v;
                self.read(v);
                return v;
            }, {.doc = "Read int32 from byte array."})
            .func("readUInt32", [](GByteArray &self) {
                uint32_t v;
                self.read(v);
                return v;
            }, {.doc = "Read uint32 from byte array."})
            .func("readInt64", [](GByteArray &self) {
                int64_t v;
                self.read(v);
                return v;
            }, {.doc = "Read int64 from byte array."})
            .func("readUInt64", [](GByteArray &self) {
                uint64_t v;
                self.read(v);
                return v;
            }, {.doc = "Read uint64 from byte array."})
            .func("readBool", [](GByteArray &self) {
                bool v;
                self.read(v);
                return v;
            }, {.doc = "Read bool from byte array."})
            .func("readFloat", [](GByteArray &self) {
                float v;
                self.read(v);
                return v;
            }, {.doc = "Read float from byte array."})
            .func("readDouble", [](GByteArray &self) {
                double v;
                self.read(v);
                return v;
            }, {.doc = "Read double from byte array."})
            .func("readString", [](GByteArray &self) {
                std::string v;
                self.read(v);
                return v;
            }, {.doc = "Read string from byte array."})
            .func("readStringData", [](GByteArray &self, int64_t len) {
                std::vector<char> buffer(len);
                self.read(buffer.data(), len);
                return std::string(buffer.data(), len);
            }, {.doc = "Read a string as a block from byte array.", .args = {"len"}})
            .func("readBytes", [](GByteArray &self) {
                GByteArray v;
                self.read(v);
                return v;
            }, {.doc = "Read GByteArray from byte array."})
            .func("readGAny", [](GByteArray &self) {
                GAny v;
                self.read(v);
                return v;
            }, {.doc = "Read GAny from byte array."})
            .func("seekWritePos", &GByteArray::seekWritePos, {.doc = "Seek write position.", .args = {"mode", "size"}})
            .func("seekReadPos", &GByteArray::seekReadPos, {.doc = "Seek read position", .args = {"mode", "size"}})
            .func("writePos", &GByteArray::writePos, {.doc = "Get write position."})
            .func("readPos", &GByteArray::readPos, {.doc = "Get read position."})
            .func("canReadMore", &GByteArray::canReadMore, {.doc = "Check if can read more."})
            .func("compare", &GByteArray::compare, {.doc = "Compare two byte array.", .args = {"other"}})
            .func("toHexString", [](GByteArray &self) {
                return self.toHexString();
            }, {.doc = "Convert to hex string."})
            .func("toHexString", [](GByteArray &self, bool uppercase) {
                return self.toHexString(uppercase);
            }, {.doc = "Convert to hex string. arg1: uppercase.", .args = {"uppercase"}})
            .func(MetaFunction::EqualTo, [](GByteArray &self, GByteArray &other) {
                return self == other;
            }, {.doc = "Equal operator.", .args = {"other"}})
            .staticFunc("fromHexString", &GByteArray::fromHexString, {.doc = "Create from hex string", .args = {"hexString"}})
            .staticFunc("compress", [](GAnyConstBytePtr data, int64_t dataSize) {
                return GByteArray::compress(reinterpret_cast<const uint8_t *>(data), dataSize);
            }, {.doc = "Compress data.", .args = {"data", "dataSize"}, .returnType = "vector<uint8_t>"})
            .staticFunc("compress", [](const GByteArray &in) {
                return GByteArray::compress(in);
            }, {.doc = "Compress data.", .args = {"in"}})
            .staticFunc("isCompressed", &GByteArray::isCompressed)
            .staticFunc("uncompress", [](GAnyConstBytePtr data, int64_t dataSize, int64_t uncompSize) {
                return GByteArray::uncompress(reinterpret_cast<const uint8_t *>(data), dataSize, uncompSize);
            }, {.doc = "Uncompress data", .args = {"data", "dataSize", "uncompSize"}, .returnType = "vector<uint8_t>"})
            .staticFunc("uncompress", [](const GByteArray &in) {
                return GByteArray::uncompress(in);
            }, {.doc = "Uncompress data.", .args = {"in"}})
            .staticFunc("base64Encode", &GByteArray::base64Encode, {.doc = "Base64 encode data.", .args = {"data"}})
            .staticFunc("base64Decode", &GByteArray::base64Decode, {.doc = "Base64 decode data.", .args = {"codes"}})
            .staticFunc("md5Sum", &GByteArray::md5Sum, {.doc = "Get md5 sum of data.", .args = {"data"}})
            .staticFunc("sha1Sum", &GByteArray::sha1Sum, {.doc = "Get sha1 sum of data.", .args = {"data"}})
            .staticFunc("sha256Sum", &GByteArray::sha256Sum, {.doc = "Get sha256 sum of data.", .args = {"data"}});
}
