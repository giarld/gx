//
// Created by gxin on 18-6-9.
//

#ifndef GX_GSTRING_H
#define GX_GSTRING_H

#include <gx/gglobal.h>

#include "gx/gany.h"

#include <string_view>
#include <vector>
#include <sstream>
#include <ostream>

#include "common.h"


#if GX_PLATFORM_WINDOWS

#define ENABLE_WSTRING  1

class GString;

class GX_API GWString
{
public:
    GWString();

    GWString(const wchar_t *wStr, int32_t size = -1);

    GWString(const std::wstring &wStr);

    GWString(const GString &str);

    GWString(const GWString &b);

    GWString(GWString &&b) noexcept;

    GWString &operator=(const GWString &b);

    GWString &operator=(GWString &&b) noexcept;

public:
    const wchar_t *data() const;

    int32_t length() const;

private:
    friend class GString;

    std::vector<wchar_t> mData;
};

#endif

/**
 * UTF-8 String
 */
class GX_API GString final
{
public:
    GString();

    GString(const char *str, int32_t size = -1);

#ifdef ENABLE_WSTRING

    GString(const GWString &wstring);

#endif

    GString(const GString &str);

    GString(GString &&str) noexcept;

    GString(char c);

    GString(const std::string &str);

    GString(std::string_view str);

    ~GString() = default;

public:
    /**
     * Get ascii byte count
     * @return byte count
     */
    int32_t count() const;

    /**
     * Get utf-8 char length
     * @return char length
     */
    int32_t length() const;

    const char *data() const;

    const char *c_str() const;

    GString at(int32_t index) const;

    /**
     * Get the pointer to the word position
     * @param index
     * @return
     */
    const char *atPtr(int32_t index) const;

    const std::string &toStdString() const;

    void reset(const char *str, int32_t size = -1);

    void reset(const GString &str);

    void reset(const std::string &str);

    int compare(const GString &bStr) const;

    GString &append(const GString &str);

    GString &append(const uint32_t *codepoints, uint32_t count);

    GString &append(const std::vector<uint32_t> &codepoints);

    GString &insert(int32_t index, const GString &str);

    bool isEmpty() const;

    bool startWith(const GString &str) const;

    bool endWith(const GString &str) const;

    GString left(int32_t n) const;

    GString right(int32_t n) const;

    /**
     * Substring the string; when len is set to the default value, it is taken from begin to the end.
     * @param begin
     * @param len
     * @return
     */
    GString substring(int32_t begin, int32_t len = -1) const;

    /**
     * Replace the string
     * @param before    The substring to be replaced
     * @param after     Replace with
     * @param begin     Starting from the 'begin'th character
     * @return
     */
    GString replace(const GString &before, const GString &after, int32_t begin = 0) const;

    std::vector<GString> split(const GString &cs) const;

    /**
     * Forward search, return the starting position of the target string if found, or -1 if not found.
     * @param str
     * @param from
     * @return
     */
    int32_t indexOf(const GString &str, int32_t from = 0) const;

    /**
     * Reverse search, return the starting position of the target string if found, or -1 if not found.
     * @param str
     * @param from  Positive numbers indicate positions starting from the beginning, where 0 represents the first position,
     *              and negative numbers indicate positions starting from the end, where -1 represents the last position.
     * @return
     */
    int32_t lastIndexOf(const GString &str, int32_t from = -1) const;

    /**
     * Retrieve the Unicode character position corresponding to the byte offset
     * @param byteOffset
     * @return
     */
    int32_t charPosition(int32_t byteOffset) const;

    void swap(GString &b) noexcept;

    /**
     * Retrieve the Unicode of the specified character.
     * @param index
     * @return
     */
    uint32_t codepoint(int32_t index) const;

#ifdef ENABLE_WSTRING

    GWString toUtf16() const;

#endif

    GString toUpper() const;

    GString toLower() const;

    GString &operator=(const GString &bStr);

    GString &operator=(GString &&bStr) noexcept;

    bool operator==(const GString &bStr) const;

    bool operator>(const GString &bStr) const;

    bool operator<(const GString &bStr) const;

    bool operator>=(const GString &bStr) const;

    bool operator<=(const GString &bStr) const;

    bool operator!=(const GString &bStr) const;

    GString &operator+=(const GString &str);

    explicit operator std::string() const;

    GString &operator<<(const GString &b);

    GString &operator<<(char c);

    template<typename T>
    friend GString &operator<<(GString &a, const T &b);

    friend std::ostream &operator<<(std::ostream &os, const GString &string);

    friend std::istream &operator>>(std::istream &is, GString &string);

    friend GString operator+(const GString &a, const GString &b);

    GString arg(const GString &key, char c) const;

    GString arg(const GString &key, const char *a) const;

    GString arg(const GString &key, bool b) const;

    GString arg(const GString &key, const GString &a) const;

    template<typename T>
    GString arg(const GString &key, const T &a) const
    {
        return arg(key, GString::toString(a));
    }

    GString arg(const GString &key, const std::string &a) const;

    GString arg(char c) const;

    GString arg(const char *str) const;

    GString arg(char *str) const;

    GString arg(const GString &str) const;

    GString arg(bool b) const;

    GString arg(const std::string &s) const;

    GString arg(const std::string_view &view) const;

    /**
     * Format argument with format specifier (similar to fmt library)
     * Supports both simple replacement {} and format specifiers {:format}
     * 
     * Format Specifier Syntax: {:[fill][align][sign][#][0][width][.precision][type]}
     * 
     * - fill: Any character (default space)
     * - align: '<' (left), '>' (right, default for numbers), '^' (center)
     * - sign: '+' (always show sign), '-' (only negative, default), ' ' (space for positive)
     * - '#': Alternate form (e.g., 0x for hex, 0b for binary)
     * - '0': Zero padding (equivalent to fill='0')
     * - width: Minimum field width
     * - .precision: Decimal precision for floating point
     * - type: 'd' (decimal), 'x'/'X' (hex), 'o' (octal), 'b'/'B' (binary),
     *         'f'/'F' (fixed point), 'e'/'E' (scientific), 'g'/'G' (general)
     * 
     * Examples:
     *   GString("{}").arg(42)                  -> "42"
     *   GString("{:.2f}").arg(3.14159)        -> "3.14"
     *   GString("{:x}").arg(255)              -> "ff"
     *   GString("{:#X}").arg(255)             -> "0XFF"
     *   GString("{:>10d}").arg(42)            -> "        42"
     *   GString("{:05d}").arg(42)             -> "00042"
     *   GString("{:+d}").arg(42)              -> "+42"
     *   GString("{:b}").arg(5)                -> "101"
     * 
     * @tparam T Type of the value
     * @param a Value to format
     * @return Formatted string
     */
    template<typename T>
    GString arg(const T &a) const
    {
        return _argWithFormat(a);
    }

    template<typename... Args>
    static GString format(GString fmt, const Args &... args)
    {
        if constexpr (sizeof...(args) > 0) {
            int dummy[] = {
                (fmt = fmt.arg(args), 0)...
            };
        }
        return fmt;
    }

    std::string toString() const
    {
        return toStdString();
    }

public: // static
    template<typename T>
    static GString toString(const T &num)
    {
        std::ostringstream o;
        o << num;
        return o.str();
    }

    static GString toString(bool num)
    {
        std::ostringstream o;
        o << (num ? "true" : "false");
        return o.str();
    }

    static bool isUtf8(const char *str, size_t len);

    static bool isGBK(const char *str, size_t len);

    static GString fromGBK(const std::string &str);

    static GString fromCodepoint(uint32_t codepoint);

private:
    /**
     * Parse format specifier and format value
     * Format specifier syntax: {:[fill][align][sign][#][0][width][.precision][type]}
     * Examples: {:.2f}, {:d}, {:x}, {:>10}, {:0>5d}
     */
    template<typename T>
    GString _argWithFormat(const T &value) const
    {
        const int32_t startIdx = indexOf("{");
        if (startIdx < 0) {
            return *this;
        }
        
        const int32_t endIdx = indexOf("}", startIdx);
        if (endIdx < 0) {
            return *this;
        }
        
        // Extract format specifier
        const GString formatSpec = substring(startIdx + 1, endIdx - startIdx - 1);
        GString formattedValue;
        
        if (formatSpec.isEmpty() || !formatSpec.startWith(":")) {
            // No format specifier, use default formatting
            formattedValue = GString::toString(value);
        } else {
            // Parse and apply format specifier
            formattedValue = _formatValue(value, formatSpec.substring(1)); // Skip ':'
        }
        
        return substring(0, startIdx) + formattedValue + substring(endIdx + 1);
    }
    
    template<typename T>
    static GString _formatValue(const T &value, const GString &spec)
    {
        return _formatValueImpl(value, spec, std::is_integral<T>{}, std::is_floating_point<T>{});
    }
    
    template<typename T>
    static GString _formatValueImpl(const T &value, const GString &spec, std::true_type /*is_integral*/, std::false_type)
    {
        return _formatInteger(static_cast<int64_t>(value), spec);
    }
    
    template<typename T>
    static GString _formatValueImpl(const T &value, const GString &spec, std::false_type, std::true_type /*is_floating*/)
    {
        return _formatFloating(static_cast<double>(value), spec);
    }
    
    template<typename T>
    GString _formatValueImpl(const T &value, const GString &spec, std::false_type, std::false_type) const
    {
        return GString::toString(value);
    }

    GString _argWithFormatString(const GString &value) const;

    static GString _formatInteger(int64_t value, const GString &spec);

    static GString _formatFloating(double value, const GString &spec);
    
    static GString _formatString(const GString &value, const GString &spec);

private:
    void _build(const char *str, int32_t size = -1);

    void _build(const GString &str);

    void _build(char c);

    void _clear();

    /**
     * Retrieve the starting pointer position of the next character.
     * @param charIndex Starting pointer position.
     * @return
     */
    int32_t _next(int32_t charIndex) const;

    /**
     * Obtain the ASCII character index corresponding to the number of Unicode characters.
     * @param uniCharSize
     * @return
     */
    int32_t _seek(int32_t uniCharSize) const;

    /**
     * Calculate the number of Unicode characters.
     */
    void _rebuildIndex();

    GString _atChar(int32_t index) const;

private:
    friend struct std::hash<GString>;

    std::string mString;
    std::vector<uint32_t> mCharOffsets;
};

template<typename T>
GString &operator<<(GString &a, const T &b)
{
    a.append(GString::toString(b));
    return a;
}

inline GString operator+(const GString &a, const GString &b)
{
    GString temp = a;
    temp.append(b);
    return temp;
}

inline std::ostream &operator<<(std::ostream &os, const GString &string)
{
    os << string.toStdString();
    return os;
}

inline std::istream &operator>>(std::istream &is, GString &string)
{
    std::string temp;
    is >> temp;
    string.reset(temp);

    return is;
}

template<>
class caster<GString>
{
public:
    static GAny castTo(const GAny &var)
    {
        if (var.is<GString>()) {
            return var;
        }
        if (var.is<std::string>()) {
            return GString(*var.as<std::string>());
        }

        return var.toBool();
    }

    static GAny castFrom(const GString &v)
    {
        return GAny::create(v);
    }

    static GAny castFrom(GString &&v)
    {
        return GAny::create(std::move(v));
    }
};

namespace std
{
template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
    os << "[";
    for (auto it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin()) {
            os << ",";
        }
        os << *it;
    }
    os << "]";
    return os;
}

template<>
struct hash<GString>
{
    size_t operator()(const GString &type) const noexcept
    {
        return gx::hashOfByte(type.mString.data(), type.mString.size());
    }
};
}

#endif //GX_GSTRING_H
