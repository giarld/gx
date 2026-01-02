//
// Created by Gxin on 2020/9/7.
//

#ifndef GX_GBYTEARRAY_H
#define GX_GBYTEARRAY_H

#include "gobject.h"

#include <vector>
#include <string>


class GAny;

class GString;

/**
 * @class GByteArray
 * @brief Byte array class, providing operations such as read and write, HASH calculation,
 * compression and decompression, base64 encoding and decoding for continuous binary data
 */
class GX_API GByteArray final : public GObject
{
public:
    enum ByteOrder
    {
        BigEndian,
        LittleEndian
    };

public:
    explicit GByteArray(int64_t size = 0);

    explicit GByteArray(const uint8_t *data, int64_t size = -1);

    explicit GByteArray(const std::vector<uint8_t> &data);

    GByteArray(GByteArray &&other) noexcept;

    GByteArray(const GByteArray &other);

    ~GByteArray() override;

    GByteArray &operator=(const GByteArray &b);

    GByteArray &operator=(GByteArray &&b) noexcept;

public:
    void setByteOrder(ByteOrder byteOrder);

    /**
     * @brief Reset<br>
     * When size is 0, only reset the write and read pointers. When size is not 0,
     * reset the buffer size and reset the write and read pointers.
     * @param size
     */
    void reset(int64_t size = 0);

    /**
     * @brief Obtain buffer capacity
     * @return
     */
    int64_t capacity() const;

    /**
     * @brief Obtain the size of the write range.
     * @return
     */
    int64_t size() const;

    /**
     * @brief Determine if it is empty
     * @note
     * size() == 0
     *
     * @return
     */
    bool isEmpty() const;

    /**
     * @brief Get buffer header pointer
     * @return
     */
    const uint8_t *data() const;

    /**
     * @brief Reset the read/write pointer to 0
     */
    void clear();

    /**
     * @brief Write data
     * @param data data pointer
     * @param size data size
     */
    void write(const void *data, int64_t size);

    /**
     * @brief Write data
     * @tparam Type data type
     * @param in    data
     */
    template<typename Type>
    void write(const Type &in)
    {
        //        align(&mWritePos, __alignof(Type));
        write(reinterpret_cast<const unsigned char *>(&in), sizeof(Type));
    }

    /**
     * @brief Write string
     * @param in
     */
    void write(const std::string &in);

    /**
     * @brief Write string
     * @param in
     */
    void write(const GString &in);

    /**
     * @brief Write a GByteArray
     * @param in
     */
    void write(const GByteArray &in);

    /**
     * @brief Write data
     * @param in
     */
    void write(const std::vector<uint8_t> &in);

    void write(const GAny &any);

    /**
     * @brief Read data
     * @param data
     * @param size
     * @return Real read size
     */
    int64_t read(void *data, int64_t size) const;

    /**
     * @brief Read data
     * @tparam Type type of output
     * @param out   output data
     */
    template<typename Type>
    void read(Type &out) const
    {
        //        align(&mReadPos, __alignof(Type));
        read(reinterpret_cast<unsigned char *>(&out), sizeof(Type));
    }

    /**
     * @brief Read String
     * @param out
     */
    void read(std::string &out) const;

    /**
     * @brief Read String
     * @param out
     */
    void read(GString &out) const;

    /**
     * @brief Reading a GByteArray
     * @param out
     */
    void read(GByteArray &out) const;

    /**
     * @brief Read data
     * @param out
     */
    void read(std::vector<uint8_t> &out) const;

    void read(GAny &any) const;

    friend std::ostream &operator<<(std::ostream &os, const GByteArray &out)
    {
        os << out.size();
        os.write(reinterpret_cast<const char *>(out.data()), out.size());

        return os;
    }

    friend std::istream &operator>>(std::istream &is, GByteArray &in)
    {
        int64_t size;
        is >> size;

        char *temp = new char[size];
        is.read(temp, size);

        in.reset(size);
        in.write(temp, size);

        delete[] temp;
        return is;
    }

    /**
     * @brief Seek write position
     *
     * @param mode SEEK_SET/SEEK_CUR/SEEK_END
     * @param size Positive and negative values represent direction, positive to right, negative to left
     */
    void seekWritePos(int mode, int64_t size);

    /**
     * @brief Seek read position
     *
     * @param mode SEEK_SET/SEEK_CUR/SEEK_END
     * @param size Positive and negative values represent direction, positive to right, negative to left
     */
    void seekReadPos(int mode, int64_t size) const;

    /**
     * @brief Get write pointer position
     * @return
     */
    int64_t writePos() const;

    /**
     * @brief Get read pointer position
     * @return
     */
    int64_t readPos() const;

    /**
     * @brief Can read more
     * @return
     */
    bool canReadMore() const;

    /**
     * @brief Determine whether the content stored in this is the same as that in other
     * @param other
     * @return
     */
    bool compare(const GByteArray &other) const;

    std::string toHexString(bool uppercase = false) const;

    std::string toString() const override;

public:
    /**
     * @brief Parsing data from a string representing hexadecimal
     *
     * @param hexString
     * @return
     */
    static GByteArray fromHexString(const std::string &hexString);

    /**
     * @brief Compressed data
     * @param data
     * @param dataSize
     * @return
     */
    static std::vector<uint8_t> compress(const uint8_t *data, int64_t dataSize);

    /**
     * @brief Compressed data
     * @param in
     * @return
     */
    static GByteArray compress(const GByteArray &in);

    /**
     * @brief Determine whether the data is compressed data
     * @param data
     * @return
     */
    static bool isCompressed(const GByteArray &data);

    /**
     * @brief Decompress data
     * @param data
     * @param dataSize
     * @param uncompSize
     * @return
     */
    static std::vector<uint8_t> uncompress(const uint8_t *data, int64_t dataSize, int64_t uncompSize);

    /**
     * @brief Decompress data
     * @param in
     * @return
     */
    static GByteArray uncompress(const GByteArray &in);

    /**
     * @brief Encoded data is base64
     * @param data
     * @return
     */
    static std::string base64Encode(const GByteArray &data);

    /**
     * @brief Decoding base64
     * @param codes
     * @return
     */
    static GByteArray base64Decode(const std::string &codes);

    /**
     * @brief Calculate the md5 value of input data
     * @param data
     * @return
     */
    static GByteArray md5Sum(const GByteArray &data);

    /**
     * @brief Calculate the sha1 value of input data
     * @param data
     * @return
     */
    static GByteArray sha1Sum(const GByteArray &data);

    /**
     * @brief Calculate the sha256 value of input data
     * @param data
     * @return
     */
    static GByteArray sha256Sum(const GByteArray &data);

private:
    void resize(int64_t size);

    uint8_t *ptr();

    void byteOrder(uint8_t *data, int64_t len) const;

    void copyOnWrite();

    static void align(int64_t *pos, int64_t alignment);

private:
    friend class GFile;

    mutable int64_t mWritePos = 0;
    mutable int64_t mReadPos = 0;
    ByteOrder mByteOrder = LittleEndian;

    struct BufferRef
    {
        uint8_t *buffer = nullptr;
        int64_t size = 0;

        explicit BufferRef(int64_t size);

        ~BufferRef();

        void resize(int64_t newSize);
    };

    std::shared_ptr<BufferRef> mBufferRef;
};

template<typename T>
using is_supported_gb_stream_t = std::enable_if_t<
    std::is_same_v<char, T> ||
    std::is_same_v<int8_t, T> ||
    std::is_same_v<uint8_t, T> ||
    std::is_same_v<int16_t, T> ||
    std::is_same_v<uint16_t, T> ||
    std::is_same_v<int32_t, T> ||
    std::is_same_v<uint32_t, T> ||
    std::is_same_v<int64_t, T> ||
    std::is_same_v<uint64_t, T> ||
    std::is_same_v<std::string, T> ||
    std::is_same_v<GString, T> ||
    std::is_same_v<GByteArray, T> ||
    std::is_same_v<float, T> ||
    std::is_same_v<double, T> ||
    std::is_same_v<bool, T> ||
    std::is_same_v<GAny, T>
>;

template<typename Type, typename = is_supported_gb_stream_t<Type> >
GByteArray &operator<<(GByteArray &ba, const Type &in)
{
    ba.write(in);
    return ba;
}

template<typename Type, typename = is_supported_gb_stream_t<Type> >
const GByteArray &operator>>(const GByteArray &ba, Type &out)
{
    ba.read(out);
    return ba;
}

template<typename Type, typename = is_supported_gb_stream_t<Type> >
GByteArray &operator>>(GByteArray &ba, Type &out)
{
    ba.read(out);
    return ba;
}

inline bool operator==(const GByteArray &lhs, const GByteArray &rhs)
{
    return lhs.compare(rhs);
}

inline bool operator!=(const GByteArray &lhs, const GByteArray &rhs)
{
    return !lhs.compare(rhs);
}

#endif //GX_GBYTEARRAY_H
