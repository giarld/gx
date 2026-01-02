//
// Created by Gxin on 2020/9/7.
//

#include "gx/gbytearray.h"

#include <gx/ghashsum.h>
#include <gx/gany.h>
#include <gx/gstring.h>
#include <gx/debug.h>

#include <memory.h>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <sstream>

#include <zlib.h>

#include <math.h>


static constexpr char sCompFlag[4] = {
    '_',
    'C',
    'M',
    'P'
};

static const std::string sBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static bool uncompressBlob(const uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t *dstLen)
{
    uLongf tempDstLen = *dstLen;
    const int ret = uncompress(dst, &tempDstLen, src, srcLen);
    *dstLen = tempDstLen;

    return ret == Z_OK;
}

static bool isBase64(uint8_t c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

void writeGAnyToByteArray(GByteArray &ba, const GAny &obj);

GAny readGAnyFromByteArray(const GByteArray &ba);

/// ================ GByteArray ================

GByteArray::GByteArray(int64_t size)
{
    reset(std::max(size, static_cast<int64_t>(1)));
}

GByteArray::GByteArray(const uint8_t *data, int64_t size)
{
    if (size < 0) {
        size = static_cast<int64_t>(strlen(reinterpret_cast<const char *>(data))) + 1;
    }
    reset(size);
    write(data, size);
}

GByteArray::GByteArray(const std::vector<uint8_t> &data)
    : GByteArray(data.data(), static_cast<int64_t>(data.size()))
{
}

GByteArray::GByteArray(GByteArray &&other) noexcept
{
    std::swap(mWritePos, other.mWritePos);
    std::swap(mReadPos, other.mReadPos);
    std::swap(mByteOrder, other.mByteOrder);
    std::swap(mBufferRef, other.mBufferRef);
}

GByteArray::GByteArray(const GByteArray &other)
{
    mWritePos = other.mWritePos;
    mReadPos = other.mReadPos;
    mByteOrder = other.mByteOrder;
    mBufferRef = other.mBufferRef;
}

GByteArray::~GByteArray()
{
    mBufferRef = nullptr;
}

GByteArray &GByteArray::operator=(const GByteArray &b)
{
    if (this != &b) {
        mWritePos = b.mWritePos;
        mReadPos = b.mReadPos;
        mByteOrder = b.mByteOrder;
        mBufferRef = b.mBufferRef;
    }
    return *this;
}

GByteArray &GByteArray::operator=(GByteArray &&b) noexcept
{
    if (this != &b) {
        std::swap(mWritePos, b.mWritePos);
        std::swap(mReadPos, b.mReadPos);
        std::swap(mByteOrder, b.mByteOrder);
        std::swap(mBufferRef, b.mBufferRef);
    }
    return *this;
}

void GByteArray::setByteOrder(ByteOrder byteOrder)
{
    mByteOrder = byteOrder;
}

void GByteArray::reset(int64_t size)
{
    if (size > 0) {
        resize(size);
    }
    mWritePos = mReadPos = 0;
}

int64_t GByteArray::capacity() const
{
    return mBufferRef->size;
}

int64_t GByteArray::size() const
{
    return mWritePos;
}

bool GByteArray::isEmpty() const
{
    return size() == 0;
}

const uint8_t *GByteArray::data() const
{
    return mBufferRef->buffer;
}

void GByteArray::clear()
{
    mWritePos = mReadPos = 0;
}

void GByteArray::write(const void *data, int64_t size)
{
    if (size <= 0) {
        return;
    }
    copyOnWrite();

    const int64_t outSize = mWritePos + size;
    if (outSize > mBufferRef->size) {
        auto newSize = static_cast<int64_t>(static_cast<float>(mBufferRef->size) * 1.5f);
        if (newSize < outSize) {
            newSize = static_cast<int64_t>(static_cast<float>(outSize) * 1.5f);
        }
        resize(newSize);
    }
    uint8_t *p = mBufferRef->buffer + mWritePos;
    memcpy(p, data, size);
    byteOrder(p, size);

    mWritePos += size;
}

void GByteArray::write(const std::string &in)
{
    write(static_cast<int64_t>(in.size()));
    write(in.data(), in.size());
}

void GByteArray::write(const GString &in)
{
    write(static_cast<int64_t>(in.count()));
    write(in.data(), in.count());
}

void GByteArray::write(const GByteArray &in)
{
    write(in.size());
    write(in.data(), in.size());
}

void GByteArray::write(const std::vector<uint8_t> &in)
{
    write(static_cast<int64_t>(in.size()));
    write(in.data(), in.size());
}

void GByteArray::write(const GAny &any)
{
    GByteArray pack;
    writeGAnyToByteArray(pack, any);
    write(pack);
}

int64_t GByteArray::read(void *data, int64_t size) const
{
    if (mReadPos + size > mWritePos) {
        size = mWritePos - mReadPos;
    }
    if (size <= 0) {
        return 0;
    }

    memcpy(data, mBufferRef->buffer + mReadPos, size);
    byteOrder(static_cast<uint8_t *>(data), size);

    mReadPos += size;

    return size;
}

void GByteArray::read(std::string &out) const
{
    int64_t size;
    read(size);
    out.resize(size);
    read(out.data(), size);
}

void GByteArray::read(GString &out) const
{
    std::string temp;
    read(temp);
    out = temp;
}

void GByteArray::read(GByteArray &out) const
{
    int64_t size;
    this->read(size);
    out.reset(size);
    this->read(out.ptr(), size);
    out.seekWritePos(SEEK_SET, size);
}

void GByteArray::read(std::vector<uint8_t> &out) const
{
    int64_t size;
    this->read(size);
    out.resize(size);
    this->read(out.data(), size);
}

void GByteArray::read(GAny &any) const
{
    GByteArray pack;
    read(pack);
    any = readGAnyFromByteArray(pack);
}

void GByteArray::seekWritePos(int mode, int64_t size)
{
    int64_t pos;
    switch (mode) {
        case SEEK_SET:
            pos = size;
            break;
        default:
        case SEEK_CUR:
            pos = mWritePos + size;
            break;
        case SEEK_END:
            pos = mBufferRef->size + size;
            break;
    }

    GX_ASSERT_S(pos >= 0 && pos <= mBufferRef->size,
                "GByteArray::seekWritePos error (pos: %d out range(0-%d))", pos, mBufferRef->size);

    if (pos < 0) {
        pos = 0;
    }
    if (pos > mBufferRef->size) {
        pos = mBufferRef->size;
    }
    mWritePos = pos;
}

void GByteArray::seekReadPos(int mode, int64_t size) const
{
    int64_t pos;
    switch (mode) {
        case SEEK_SET:
            pos = size;
            break;
        default:
        case SEEK_CUR:
            pos = mReadPos + size;
            break;
        case SEEK_END:
            pos = mBufferRef->size + size;
            break;
    }

    GX_ASSERT_S(pos >= 0 && pos <= mBufferRef->size,
                "GByteArray::seekReadPos error (pos: %d out range(0-%d))", pos, mBufferRef->size);

    if (pos < 0) {
        pos = 0;
    }
    if (pos > mBufferRef->size) {
        pos = mBufferRef->size;
    }
    mReadPos = pos;
}

int64_t GByteArray::writePos() const
{
    return mWritePos;
}

int64_t GByteArray::readPos() const
{
    return mReadPos;
}

bool GByteArray::canReadMore() const
{
    return mReadPos < mWritePos;
}

bool GByteArray::compare(const GByteArray &other) const
{
    return size() == other.size() && memcmp(data(), other.data(), size()) == 0;
}

std::string GByteArray::toHexString(bool uppercase) const
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    if (uppercase) {
        ss << std::uppercase;
    }
    for (int64_t i = 0; i < size(); i++) {
        ss << std::setw(2) << static_cast<int>(mBufferRef->buffer[i]);
    }
    return ss.str();
}

std::string GByteArray::toString() const
{
    std::stringstream ss;
    ss << "<GByteArray at " << std::hex << this << std::dec << ""
            << ", size: " << size()
            << ", hash: " << md5Sum(*this).toHexString()
            << ">";
    return ss.str();
}

GByteArray GByteArray::fromHexString(const std::string &hexString)
{
    // Convert the string to lowercase first
    std::string str = hexString;
    std::transform(str.begin(), str.end(), str.begin(), tolower);

    // Determine whether a string is legal
    if (str.size() % 2 != 0) {
        return GByteArray();
    }

    // Convert a string to a byte array
    GByteArray bytes(static_cast<int64_t>(str.size()) / 2);
    for (int64_t i = 0; i < str.size(); i += 2) {
        char c1 = str[i];
        char c2 = str[i + 1];
        if (c1 >= '0' && c1 <= '9') {
            c1 -= '0';
        } else if (c1 >= 'a' && c1 <= 'f') {
            c1 -= 'a' - 10;
        } else {
            return GByteArray();
        }
        if (c2 >= '0' && c2 <= '9') {
            c2 -= '0';
        } else if (c2 >= 'a' && c2 <= 'f') {
            c2 -= 'a' - 10;
        } else {
            return GByteArray();
        }
        bytes.write(static_cast<char>((c1 << 4) | c2));
    }
    return bytes;
}

std::vector<uint8_t> GByteArray::compress(const uint8_t *data, int64_t dataSize)
{
    uLongf preDstLen = compressBound(dataSize);

    std::vector<uint8_t> buffer(preDstLen);
    if (::compress(buffer.data(), &preDstLen, data, dataSize) == Z_OK) {
        buffer.resize(preDstLen);
        return buffer;
    }

    return {};
}

GByteArray GByteArray::compress(const GByteArray &in)
{
    if (in.isEmpty()) {
        return GByteArray();
    }

    if (isCompressed(in)) {
        return GByteArray();
    }

    GByteArray out;
    out.write(sCompFlag, sizeof(sCompFlag));
    out.write(in.size());

    const auto comp = compress(in.data(), in.size());
    out.write(comp.data(), comp.size());

    return out;
}

bool GByteArray::isCompressed(const GByteArray &data)
{
    if (data.size() < sizeof(sCompFlag)) {
        return false;
    }

    char flagBuf[4];
    memcpy(flagBuf, data.data(), sizeof(sCompFlag));

    return memcmp(sCompFlag, flagBuf, sizeof(sCompFlag)) == 0;
}

std::vector<uint8_t> GByteArray::uncompress(const uint8_t *data, int64_t dataSize, int64_t uncompSize)
{
    std::vector<uint8_t> buffer(uncompSize);
    int64_t total = uncompSize;

    if (uncompressBlob(data, dataSize, buffer.data(), &total)) {
        return buffer;
    }
    return {};
}

GByteArray GByteArray::uncompress(const GByteArray &in)
{
    if (!isCompressed(in)) {
        return GByteArray();
    }
    const auto oldReadPos = in.mReadPos;
    in.seekReadPos(SEEK_CUR, sizeof(sCompFlag));
    int64_t uncompSize;
    in.read(uncompSize);

    GByteArray out(uncompress(in.data() + in.readPos(), in.size() - in.readPos(), uncompSize));
    in.seekReadPos(SEEK_SET, oldReadPos);
    return out;
}

std::string GByteArray::base64Encode(const GByteArray &data)
{
    std::string ret;

    int i = 0;
    int j = 0;

    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    const uint8_t *buf = data.data();
    int64_t bufLen = data.size();

    while (bufLen--) {
        char_array_3[i++] = *(buf++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) {
                ret += sBase64Chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += sBase64Chars[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

GByteArray GByteArray::base64Decode(const std::string &codes)
{
    int in_len = static_cast<int>(codes.size());
    int i = 0;
    int j = 0;
    int in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];

    std::vector<uint8_t> ret;

    while (in_len-- && (codes[in_] != '=') && isBase64(codes[in_])) {
        char_array_4[i++] = codes[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = sBase64Chars.find(static_cast<char>(char_array_4[i]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = sBase64Chars.find(static_cast<char>(char_array_4[j]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) { ret.push_back(char_array_3[j]); }
    }

    return GByteArray(ret);
}

GByteArray GByteArray::md5Sum(const GByteArray &data)
{
    const auto hashSum = GHashSum::hashSum(GHashSum::Md5);
    hashSum->update(data);
    return hashSum->final();
}

GByteArray GByteArray::sha1Sum(const GByteArray &data)
{
    const auto hashSum = GHashSum::hashSum(GHashSum::Sha1);
    hashSum->update(data);
    return hashSum->final();
}

GByteArray GByteArray::sha256Sum(const GByteArray &data)
{
    const auto hashSum = GHashSum::hashSum(GHashSum::Sha256);
    hashSum->update(data);
    return hashSum->final();
}


void GByteArray::resize(int64_t size)
{
    if (mBufferRef) {
        if (size > mBufferRef->size) {
            copyOnWrite();
            mBufferRef->resize(size);
        }
    } else {
        mBufferRef = std::make_shared<BufferRef>(size);
    }
}

uint8_t *GByteArray::ptr()
{
    copyOnWrite();
    return mBufferRef->buffer;
}

void GByteArray::align(int64_t *pos, int64_t alignment)
{
    const int64_t mask = alignment - 1;
    const int64_t xpos = ((*pos) + mask) & (~mask);
    *pos = xpos;
}

void GByteArray::byteOrder(uint8_t *data, int64_t len) const
{
    static ByteOrder systemOrder =
#if GX_CPU_ENDIAN_LITTLE
            LittleEndian;
#else
            BigEndian;
#endif

    if (mByteOrder != systemOrder) {
        std::reverse(data, data + len);
    }
}

void GByteArray::copyOnWrite()
{
    if (mBufferRef.use_count() <= 1) {
        return;
    }

    const auto newBufferRef = std::make_shared<BufferRef>(mBufferRef->size);
    memcpy(newBufferRef->buffer, mBufferRef->buffer, mBufferRef->size);

    mBufferRef = newBufferRef;
}

GByteArray::BufferRef::BufferRef(int64_t size)
{
    this->size = size;
    buffer = static_cast<uint8_t *>(malloc(size));
    memset(buffer, 0, size);
}

GByteArray::BufferRef::~BufferRef()
{
    free(buffer);
}

void GByteArray::BufferRef::resize(int64_t newSize)
{
    if (newSize <= this->size) {
        return;
    }
    auto *newBuffer = static_cast<uint8_t *>(malloc(newSize));
    memset(newBuffer, 0, newSize);
    memcpy(newBuffer, this->buffer, std::min(this->size, newSize));
    free(this->buffer);
    this->buffer = newBuffer;
    this->size = newSize;
}

/// ================ GAny ================

void writeGAnyObjectToByteArray(GByteArray &ba, const GAny &obj)
{
    if (!obj.isObject()) {
        return;
    }

    const auto tObj = *obj.unsafeAs<std::unordered_map<std::string, GAny> >();
    const auto size = static_cast<int64_t>(obj.size());

    ba.write(size);
    for (const auto &it: tObj) {
        ba.write(it.first);
        writeGAnyToByteArray(ba, it.second);
    }
}

void writeGAnyArrayToByteArray(GByteArray &ba, const GAny &obj)
{
    if (!obj.isArray()) {
        return;
    }

    const auto &vec = *obj.unsafeAs<std::vector<GAny> >();
    const auto size = static_cast<int64_t>(vec.size());

    ba.write(size);
    for (const auto &it: vec) {
        writeGAnyToByteArray(ba, it);
    }
}

GAny readGAnyObjectFromByteArray(const GByteArray &value)
{
    int64_t size;
    value.read(size);

    GAny obj = GAny::object();
    for (int64_t i = 0; i < size; i++) {
        std::string key;
        value.read(key);
        obj[key] = readGAnyFromByteArray(value);
    }
    return obj;
}

GAny readGAnyArrayFromByteArray(const GByteArray &value)
{
    int64_t size;
    value.read(size);

    GAny obj = GAny::array();
    for (int64_t i = 0; i < size; i++) {
        obj.pushBack(readGAnyFromByteArray(value));
    }
    return obj;
}

void writeGAnyToByteArray(GByteArray &ba, const GAny &obj)
{
    switch (obj.type()) {
        case AnyType::undefined_t: {
            ba.write(static_cast<uint8_t>(0));
        }
        break;
        case AnyType::null_t: {
            ba.write(static_cast<uint8_t>(1));
        }
        break;
        case AnyType::boolean_t: {
            ba.write(static_cast<uint8_t>(2));
            ba.write(*obj.as<bool>());
        }
        break;
        case AnyType::int_t: {
            ba.write(static_cast<uint8_t>(3));
            ba.write(obj.toInt64());
        }
        break;
        case AnyType::float_t: {
            ba.write(static_cast<uint8_t>(4));
            ba.write(*obj.as<float>());
        }
        break;
        case AnyType::double_t: {
            ba.write(static_cast<uint8_t>(5));
            ba.write(*obj.as<double>());
        }
        break;
        case AnyType::string_t: {
            ba.write(static_cast<uint8_t>(6));
            ba.write(*obj.as<std::string>());
        }
        break;
        case AnyType::array_t: {
            ba.write(static_cast<uint8_t>(7));
            writeGAnyArrayToByteArray(ba, obj);
        }
        break;
        case AnyType::object_t: {
            ba.write(static_cast<uint8_t>(8));
            writeGAnyObjectToByteArray(ba, obj);
        }
        break;
        case AnyType::class_t: {
            ba.write(static_cast<uint8_t>(9));
            std::stringstream ss;
            auto &cl = *obj.as<GAnyClass>();
            ss << "<Class: " << cl.getName() << ">";
            ba.write(ss.str());
        }
        break;
        default: {
            if (obj.is<GByteArray>()) {
                ba.write(static_cast<uint8_t>(15));
                ba.write(*obj.as<GByteArray>());
            } else {
                ba.write(static_cast<uint8_t>(254));
                std::stringstream ss;
                ss << "<" << obj.classTypeName() << " at " << obj.value().get() << ">";
                ba.write(ss.str());
            }
        }
        break;
    }
}

GAny readGAnyFromByteArray(const GByteArray &ba)
{
    uint8_t type;
    ba.read(type);

    switch (type) {
        default:
        case 0:
            return GAny::undefined();
        case 1:
            return GAny::null();
        case 2: {
            bool value;
            ba.read(value);
            return value;
        }
        case 3: {
            int64_t value;
            ba.read(value);
            return value;
        }
        case 4: {
            float value;
            ba.read(value);
            return value;
        }
        case 5: {
            double value;
            ba.read(value);
            return value;
        }
        case 6: {
            std::string value;
            ba.read(value);
            return value;
        }
        case 7: {
            GAny obj = readGAnyArrayFromByteArray(ba);
            return obj;
        }
        case 8: {
            GAny obj = readGAnyObjectFromByteArray(ba);
            return obj;
        }
        case 15: {
            GByteArray value;
            ba.read(value);
            return value;
        }
        case 9:
        case 254: {
            std::string value;
            ba.read(value);
            return value;
        }
    }
}
