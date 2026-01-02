//
// Created by Gxin on 24-5-6.
//

#include "gx/gstring.h"

#include <iomanip>
#include <sstream>

#if GX_PLATFORM_WINDOWS

#include <windows.h>

#elif GX_PLATFORM_LINUX || GX_PLATFORM_BSD || GX_PLATFORM_OSX || GX_PLATFORM_IOS

#include <iconv.h>

#endif


GString::GString()
{
}

GString::GString(const char *str, int32_t size)
{
    if (!str) {
        _build("");
    } else {
        _build(str, size);
    }
}

#ifdef ENABLE_WSTRING

GString::GString(const GWString &wstr)
{
    const int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.length(), nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8(len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.length(), utf8.data(), len, nullptr, nullptr);
    utf8[len] = '\0';
    _build(utf8.data());
}

#endif

GString::GString(const GString &str)
    : mString(str.mString)
{
    _rebuildIndex();
}

GString::GString(GString &&str) noexcept
    : mString(std::move(str.mString)),
      mCharOffsets(str.mCharOffsets)
{
    str.mString.clear();
    str.mCharOffsets.clear();
}

GString::GString(char c)
{
    _build(c);
}

GString::GString(const std::string &str)
{
    _build(str.c_str());
}

GString::GString(std::string_view str)
{
    _build(str.data(), str.size());
}

int32_t GString::count() const
{
    return static_cast<int32_t>(mString.size());
}

int32_t GString::length() const
{
    return mCharOffsets.size();
}

const char *GString::data() const
{
    return mString.data();
}

const char *GString::c_str() const
{
    return data();
}

GString GString::at(int32_t index) const
{
    return _atChar(index);
}

const char *GString::atPtr(int32_t index) const
{
    const int32_t i = _seek(index);
    return data() + i;
}

const std::string &GString::toStdString() const
{
    return mString;
}

void GString::reset(const char *str, int32_t size)
{
    _build(str, size);
}

void GString::reset(const GString &str)
{
    _build(str);
}

void GString::reset(const std::string &str)
{
    _build(str.c_str());
}

int GString::compare(const GString &bStr) const
{
    return this->mString.compare(bStr.mString);
}

GString &GString::append(const GString &str)
{
    mString.append(str.mString);
    _rebuildIndex();
    return *this;
}

GString &GString::append(const uint32_t *codepoints, uint32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        const uint32_t codepoint = codepoints[i];
        uint8_t c[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
        if (codepoint <= 0x7F) {
            c[0] = codepoint;
        } else if (codepoint <= 0x7FF) {
            c[0] = (codepoint >> 6) + 192;
            c[1] = (codepoint & 63) + 128;
        } else if (0xd800 <= codepoint && codepoint <= 0xdfff) {
        } //invalid block of utf8
        else if (codepoint <= 0xFFFF) {
            c[0] = (codepoint >> 12) + 224;
            c[1] = ((codepoint >> 6) & 63) + 128;
            c[2] = (codepoint & 63) + 128;
        } else if (codepoint <= 0x10FFFF) {
            c[0] = (codepoint >> 18) + 240;
            c[1] = ((codepoint >> 12) & 63) + 128;
            c[2] = ((codepoint >> 6) & 63) + 128;
            c[3] = (codepoint & 63) + 128;
        }
        mString.append(reinterpret_cast<const char *>(c), static_cast<int32_t>(strlen(reinterpret_cast<char *>(c))));
    }
    _rebuildIndex();
    return *this;
}

GString &GString::append(const std::vector<uint32_t> &codepoints)
{
    return append(codepoints.data(), codepoints.size());
}

GString &GString::insert(int32_t index, const GString &str)
{
    if (str.isEmpty()) {
        return *this;
    }
    if (index >= this->length()) {
        return this->append(str);
    }

    const int32_t byteOffset = _seek(index);

    const int32_t sourceCount = this->count();
    const int32_t insertCount = str.count();
    const char *sourceData = this->data();

    std::string resultBuffer(sourceData, byteOffset);
    resultBuffer.append(str.data(), insertCount);
    resultBuffer.append(sourceData + byteOffset, sourceCount - byteOffset);

    this->mString = std::move(resultBuffer);
    this->_rebuildIndex();

    return *this;
}

bool GString::isEmpty() const
{
    return mString.empty();
}

bool GString::startWith(const GString &str) const
{
    if (str.count() == 0 && this->count() == 0) {
        return true;
    }
    if (str.count() > this->count()) {
        return false;
    }
    return memcmp(this->data(), str.data(), str.count()) == 0;
}

bool GString::endWith(const GString &str) const
{
    if (str.count() == 0 && this->count() == 0) {
        return true;
    }
    if (str.count() > this->count()) {
        return false;
    }
    return memcmp(this->data() + this->count() - str.count(), str.data(), str.count()) == 0;
}

GString GString::left(int32_t n) const
{
    if (n == 0) {
        return "";
    }

    if (n >= this->length()) {
        return *this;
    }

    int32_t i = _seek(n);
    return {this->data(), i};
}

GString GString::right(int32_t n) const
{
    if (n == 0) {
        return "";
    }

    const int32_t sSize = this->length();
    if (n > sSize) {
        return *this;
    }

    const int32_t i = _seek(sSize - n);
    return {this->data() + i, (this->count() - i)};
}

GString GString::substring(int32_t begin, int32_t len) const
{
    const int32_t b = _seek(begin);
    int32_t e;
    if (len < 0) {
        e = this->count();
    } else {
        e = _seek(begin + len);
    }
    if (e <= b) {
        return "";
    }
    return {this->data() + b, e - b};
}

GString GString::replace(const GString &before, const GString &after, int32_t begin) const
{
    if (this->isEmpty() || before.isEmpty()) {
        return *this;
    }

    const char *sourceData = this->data();
    const int32_t sourceCount = this->count();
    const char *beforeData = before.data();
    const int32_t beforeCount = before.count();
    const char *afterData = after.data();
    const int32_t afterCount = after.count();

    const int32_t startByteOffset = _seek(begin);
    if (startByteOffset >= sourceCount) {
        return *this;
    }

    std::string resultBuffer(sourceData, startByteOffset);

    int32_t currentPos = startByteOffset;
    int32_t lastMatchEndPos = startByteOffset;

    while (currentPos <= sourceCount - beforeCount) {
        if (memcmp(sourceData + currentPos, beforeData, beforeCount) == 0) {
            if (currentPos > lastMatchEndPos) {
                resultBuffer.append(sourceData + lastMatchEndPos, currentPos - lastMatchEndPos);
            }
            resultBuffer.append(afterData, afterCount);
            currentPos += beforeCount;
            lastMatchEndPos = currentPos;
        } else {
            currentPos++;
        }
    }

    if (lastMatchEndPos < sourceCount) {
        resultBuffer.append(sourceData + lastMatchEndPos, sourceCount - lastMatchEndPos);
    }

    GString result;
    result.mString = std::move(resultBuffer);
    result._rebuildIndex();
    return result;
}

std::vector<GString> GString::split(const GString &cs) const
{
    std::vector<GString> clips;

    if (this->isEmpty()) {
        return clips;
    }

    if (cs.isEmpty()) {
        clips.push_back(*this);
        return clips;
    }

    const char *sourceData = this->data();
    const int32_t sourceCount = this->count();
    const char *separatorData = cs.data();
    const int32_t separatorCount = cs.count();

    int32_t currentPos = 0;
    while (currentPos < sourceCount) {
        const char *match = strstr(sourceData + currentPos, separatorData);

        if (match) {
            const int32_t matchIndex = match - sourceData;
            clips.emplace_back(sourceData + currentPos, matchIndex - currentPos);
            currentPos = matchIndex + separatorCount;
        } else {
            break;
        }
    }

    if (currentPos <= sourceCount) {
        clips.emplace_back(sourceData + currentPos, sourceCount - currentPos);
    }

    return clips;
}

int32_t GString::indexOf(const GString &str, int32_t from) const
{
    if (from < 0) {
        from = 0;
    }

    if (from >= this->length() || str.isEmpty() || str.count() > this->count()) {
        return -1;
    }

    const char *sourceData = this->data();
    const char *targetData = str.data();
    const int32_t sourceByteCount = this->count();

    const char *searchStartPtr = sourceData + _seek(from);

    while (searchStartPtr < sourceData + sourceByteCount) {
        const char *matchPtr = strstr(searchStartPtr, targetData);

        if (!matchPtr) {
            return -1;
        }

        const int32_t matchByteOffset = static_cast<int32_t>(matchPtr - sourceData);

        if (std::binary_search(mCharOffsets.begin(), mCharOffsets.end(), matchByteOffset)) {
            return this->charPosition(matchByteOffset);
        }

        searchStartPtr = matchPtr + 1;
    }

    return -1;
}

int32_t GString::lastIndexOf(const GString &str, int32_t from) const
{
    if (this->isEmpty() || str.isEmpty() || str.count() > this->count()) {
        return -1;
    }

    const int32_t sourceCount = this->count();
    const int32_t targetCount = str.count();
    const char *sourceData = this->data();
    const char *targetData = str.data();

    const int32_t length = mCharOffsets.size();
    int32_t startCharIndex;
    if (from < 0) {
        startCharIndex = length - 1;
    } else {
        startCharIndex = std::min(from, length - 1);
    }

    for (int32_t i = startCharIndex; i >= 0; --i) {
        const int32_t byteOffset = mCharOffsets[i];
        if (byteOffset + targetCount > sourceCount) {
            continue;
        }
        if (memcmp(sourceData + byteOffset, targetData, targetCount) == 0) {
            return i;
        }
    }

    return -1;
}

int32_t GString::charPosition(int32_t byteOffset) const
{
    if (byteOffset <= 0) {
        return 0;
    }
    if (byteOffset >= count()) {
        return mCharOffsets.size();
    }

    const auto it = std::upper_bound(mCharOffsets.begin(), mCharOffsets.end(), byteOffset);
    return static_cast<int32_t>(std::distance(mCharOffsets.begin(), it) - 1);
}

void GString::swap(GString &b) noexcept
{
    std::swap(this->mString, b.mString);
    std::swap(this->mCharOffsets, b.mCharOffsets);
}

uint32_t GString::codepoint(int32_t index) const
{
    const int32_t i = _seek(index);
    const int32_t j = _seek(index + 1);
    if (j <= i) {
        return 0;
    }

    const char *u = data() + i;
    const size_t l = j - i;
    if (l < 1) {
        return 0;
    }
    const unsigned char u0 = u[0];
    if (u0 <= 127) {
        return u0;
    }
    if (l < 2) {
        return 0;
    }
    const unsigned char u1 = u[1];
    if (u0 >= 192 && u0 <= 223) {
        return (u0 - 192) * 64 + (u1 - 128);
    }
    if (static_cast<uint8_t>(u[0]) == 0xed && (u[1] & 0xa0) == 0xa0) {
        return 0;
    } //code points, 0xd800 to 0xdfff
    if (l < 3) {
        return 0;
    }
    const unsigned char u2 = u[2];
    if (u0 >= 224 && u0 <= 239) {
        return (u0 - 224) * 4096 + (u1 - 128) * 64 + (u2 - 128);
    }
    if (l < 4) {
        return 0;
    }
    const unsigned char u3 = u[3];
    if (u0 >= 240 && u0 <= 247) {
        return (u0 - 240) * 262144 + (u1 - 128) * 4096 + (u2 - 128) * 64 + (u3 - 128);
    }
    return 0;
}

#ifdef ENABLE_WSTRING

GWString GString::toUtf16() const
{
    return {*this};
}

#endif

GString GString::toUpper() const
{
    if (isEmpty()) {
        return *this;
    }

    GString result = *this;
    char* resultData = const_cast<char*>(result.c_str());
    const int32_t length = mCharOffsets.size();

    for (int32_t i = 0; i < length; ++i)
    {
        const int32_t byteOffset = mCharOffsets[i];
        const int32_t nextByteOffset = (i + 1 < length) ? mCharOffsets[i + 1] : this->count();
        const int32_t charByteLen = nextByteOffset - byteOffset;

        if (charByteLen == 1) {
            char& c = resultData[byteOffset];
            if (c >= 'a' && c <= 'z') {
                c = c - ('a' - 'A');
            }
        }
    }

    return result;
}

GString GString::toLower() const
{
    if (isEmpty()) {
        return *this;
    }

    GString result = *this;
    char* resultData = const_cast<char*>(result.c_str());
    const int32_t length = mCharOffsets.size();

    for (int32_t i = 0; i < length; ++i)
    {
        const int32_t byteOffset = mCharOffsets[i];

        const int32_t nextByteOffset = (i + 1 < length) ? mCharOffsets[i + 1] : this->count();
        const int32_t charByteLen = nextByteOffset - byteOffset;

        if (charByteLen == 1) {
            char& c = resultData[byteOffset];
            if (c >= 'A' && c <= 'Z') {
                c = c + ('a' - 'A');
            }
        }
    }

    return result;
}

GString &GString::operator=(const GString &bStr)
{
    if (&bStr == this) {
        return *this;
    }
    _build(bStr);
    return *this;
}

GString &GString::operator=(GString &&bStr) noexcept
{
    if (&bStr == this) {
        return *this;
    }
    std::swap(this->mString, bStr.mString);
    std::swap(this->mCharOffsets, bStr.mCharOffsets);

    bStr.mString.clear();
    bStr.mCharOffsets.clear();
    return *this;
}

bool GString::operator==(const GString &bStr) const
{
    return this->compare(bStr) == 0;
}

bool GString::operator>(const GString &bStr) const
{
    return this->compare(bStr) > 0;
}

bool GString::operator<(const GString &bStr) const
{
    return this->compare(bStr) < 0;
}

bool GString::operator>=(const GString &bStr) const
{
    return this->compare(bStr) >= 0;
}

bool GString::operator<=(const GString &bStr) const
{
    return this->compare(bStr) <= 0;
}

bool GString::operator!=(const GString &bStr) const
{
    return this->compare(bStr) != 0;
}

GString &GString::operator+=(const GString &str)
{
    return this->append(str);
}

GString::operator std::string() const
{
    return toStdString();
}

GString &GString::operator<<(const GString &b)
{
    this->append(b);
    return *this;
}

GString &GString::operator<<(char c)
{
    this->append(GString(c));
    return *this;
}

GString GString::arg(const GString &key, char c) const
{
    GString keys = "{";
    keys += key;
    keys += "}";
    GString newStr = replace(keys, GString(c));
    return newStr;
}

GString GString::arg(const GString &key, const char *a) const
{
    return arg(key, GString(a));
}

GString GString::arg(const GString &key, bool b) const
{
    return arg(key, GString(b ? "true" : "false"));
}

GString GString::arg(const GString &key, const GString &a) const
{
    GString keys = "{";
    keys += key;
    keys += "}";
    GString newStr = replace(keys, a);
    return newStr;
}

GString GString::arg(const GString &key, const std::string &a) const
{
    GString keys = "{";
    keys += key;
    keys += "}";
    return replace(keys, a);
}

GString GString::arg(char c) const
{
    return arg(GString(c));
}

GString GString::arg(const char *str) const
{
    return arg(GString(str));
}

GString GString::arg(char *str) const
{
    return arg(GString(str));
}

GString GString::arg(const GString &str) const
{
    return _argWithFormatString(str);
}

GString GString::arg(bool b) const
{
    return arg((b ? "true" : "false"));
}

GString GString::arg(const std::string &s) const
{
    return arg(GString(s));
}

GString GString::arg(const std::string_view &view) const
{
    return arg(GString(view.data(), static_cast<int32_t>(view.size())));
}

GString GString::_formatInteger(int64_t value, const GString &spec)
{
    // Parse format spec: [fill][align][sign][#][0][width][type]
    const char *specStr = spec.c_str();
    int32_t specLen = spec.count();
    
    char fillChar = ' ';
    char align = '>';  // default right align for numbers
    char sign = '-';   // only negative numbers
    bool altForm = false;
    int width = 0;
    char type = 'd';   // default decimal
    
    int pos = 0;
    
    // Parse fill and align (fill char + align char)
    if (specLen >= 2) {
        if (specStr[1] == '<' || specStr[1] == '>' || specStr[1] == '^') {
            fillChar = specStr[0];
            align = specStr[1];
            pos = 2;
        } else if (specStr[0] == '<' || specStr[0] == '>' || specStr[0] == '^') {
            align = specStr[0];
            pos = 1;
        }
    } else if (specLen >= 1) {
        if (specStr[0] == '<' || specStr[0] == '>' || specStr[0] == '^') {
            align = specStr[0];
            pos = 1;
        }
    }
    
    // Parse sign
    if (pos < specLen && (specStr[pos] == '+' || specStr[pos] == '-' || specStr[pos] == ' ')) {
        sign = specStr[pos];
        pos++;
    }
    
    // Parse alternate form (#)
    if (pos < specLen && specStr[pos] == '#') {
        altForm = true;
        pos++;
    }
    
    // Parse zero padding
    if (pos < specLen && specStr[pos] == '0') {
        fillChar = '0';
        pos++;
    }
    
    // Parse width
    while (pos < specLen && specStr[pos] >= '0' && specStr[pos] <= '9') {
        width = width * 10 + (specStr[pos] - '0');
        pos++;
    }
    
    // Parse type
    if (pos < specLen) {
        type = specStr[pos];
    }
    
    // Format the value based on type
    std::ostringstream oss;
    
    switch (type) {
        case 'b': // binary
        case 'B': {
            if (value == 0) {
                oss << "0";
            } else {
                std::string binary;
                uint64_t absVal = (value < 0) ? -value : value;
                while (absVal > 0) {
                    binary = static_cast<char>('0' + (absVal & 1)) + binary;
                    absVal >>= 1;
                }
                if (altForm) {
                    oss << "0b";
                }
                oss << binary;
            }
            break;
        }
        case 'o': // octal
            if (altForm && value != 0) {
                oss << "0";
            }
            oss << std::oct << (value < 0 ? -value : value);
            break;
        case 'x': // hex lowercase
            if (altForm && value != 0) {
                oss << "0x";
            }
            oss << std::hex << std::nouppercase << (value < 0 ? -value : value);
            break;
        case 'X': // hex uppercase
            if (altForm && value != 0) {
                oss << "0X";
            }
            oss << std::hex << std::uppercase << (value < 0 ? -value : value);
            break;
        case 'd':
        default:
            oss << value;
            break;
    }
    
    GString result = oss.str();
    
    // Add sign
    if (value < 0) {
        result = GString("-") + result;
    } else if (sign == '+') {
        result = GString("+") + result;
    } else if (sign == ' ') {
        result = GString(" ") + result;
    }
    
    // Apply width and alignment
    if (width > 0 && result.count() < width) {
        int32_t padLen = width - result.count();
        GString padding;
        for (int i = 0; i < padLen; i++) {
            padding += fillChar;
        }
        
        if (align == '<') {
            result = result + padding;
        } else if (align == '^') {
            int32_t leftPad = padLen / 2;
            int32_t rightPad = padLen - leftPad;
            GString leftPadding, rightPadding;
            for (int i = 0; i < leftPad; i++) leftPadding += fillChar;
            for (int i = 0; i < rightPad; i++) rightPadding += fillChar;
            result = leftPadding + result + rightPadding;
        } else { // '>'
            result = padding + result;
        }
    }
    
    return result;
}

GString GString::_formatString(const GString &value, const GString &spec)
{
    // Parse format spec: [fill][align][width]
    const char *specStr = spec.c_str();
    int32_t specLen = spec.count();
    
    char fillChar = ' ';
    char align = '<';  // default left align for strings
    int width = 0;
    
    int pos = 0;
    
    // Parse fill and align
    if (specLen >= 2) {
        if (specStr[1] == '<' || specStr[1] == '>' || specStr[1] == '^') {
            fillChar = specStr[0];
            align = specStr[1];
            pos = 2;
        } else if (specStr[0] == '<' || specStr[0] == '>' || specStr[0] == '^') {
            align = specStr[0];
            pos = 1;
        }
    } else if (specLen >= 1) {
        if (specStr[0] == '<' || specStr[0] == '>' || specStr[0] == '^') {
            align = specStr[0];
            pos = 1;
        }
    }
    
    // Parse width
    while (pos < specLen && specStr[pos] >= '0' && specStr[pos] <= '9') {
        width = width * 10 + (specStr[pos] - '0');
        pos++;
    }
    
    GString result = value;
    
    // Apply width and alignment
    if (width > 0 && result.count() < width) {
        int32_t padLen = width - result.count();
        GString padding;
        for (int i = 0; i < padLen; i++) {
            padding += fillChar;
        }
        
        if (align == '<') {
            result = result + padding;
        } else if (align == '^') {
            int32_t leftPad = padLen / 2;
            int32_t rightPad = padLen - leftPad;
            GString leftPadding, rightPadding;
            for (int i = 0; i < leftPad; i++) leftPadding += fillChar;
            for (int i = 0; i < rightPad; i++) rightPadding += fillChar;
            result = leftPadding + result + rightPadding;
        } else { // '>'
            result = padding + result;
        }
    }
    
    return result;
}

GString GString::_argWithFormatString(const GString &value) const
{
    const int32_t startIdx = indexOf("{");
    if (startIdx < 0) {
        return *this;
    }

    const int32_t endIdx = indexOf("}", startIdx);
    if (endIdx < 0) {
        return *this;
    }

    const GString formatSpec = substring(startIdx + 1, endIdx - startIdx - 1);
    GString formattedValue;

    if (formatSpec.isEmpty()) {
        formattedValue = value;
    } else if (formatSpec.startWith(":")) {
        formattedValue = _formatString(value, formatSpec.substring(1)); // Skip ':'
    } else {
        formattedValue = value;
    }

    return substring(0, startIdx) + formattedValue + substring(endIdx + 1);
}

GString GString::_formatFloating(double value, const GString &spec)
{
    // Parse format spec: [fill][align][sign][#][0][width][.precision][type]
    const char *specStr = spec.c_str();
    int32_t specLen = spec.count();
    
    char fillChar = ' ';
    char align = '>';  // default right align for numbers
    char sign = '-';   // only negative numbers
    bool altForm = false;
    int width = 0;
    int precision = -1;
    char type = 'g';   // default general format
    
    int pos = 0;
    
    // Parse fill and align
    if (specLen >= 2) {
        if (specStr[1] == '<' || specStr[1] == '>' || specStr[1] == '^') {
            fillChar = specStr[0];
            align = specStr[1];
            pos = 2;
        } else if (specStr[0] == '<' || specStr[0] == '>' || specStr[0] == '^') {
            align = specStr[0];
            pos = 1;
        }
    } else if (specLen >= 1) {
        if (specStr[0] == '<' || specStr[0] == '>' || specStr[0] == '^') {
            align = specStr[0];
            pos = 1;
        }
    }
    
    // Parse sign
    if (pos < specLen && (specStr[pos] == '+' || specStr[pos] == '-' || specStr[pos] == ' ')) {
        sign = specStr[pos];
        pos++;
    }
    
    // Parse alternate form (#)
    if (pos < specLen && specStr[pos] == '#') {
        altForm = true;
        pos++;
    }
    
    // Parse zero padding
    if (pos < specLen && specStr[pos] == '0') {
        fillChar = '0';
        pos++;
    }
    
    // Parse width
    while (pos < specLen && specStr[pos] >= '0' && specStr[pos] <= '9') {
        width = width * 10 + (specStr[pos] - '0');
        pos++;
    }
    
    // Parse precision
    if (pos < specLen && specStr[pos] == '.') {
        pos++;
        precision = 0;
        while (pos < specLen && specStr[pos] >= '0' && specStr[pos] <= '9') {
            precision = precision * 10 + (specStr[pos] - '0');
            pos++;
        }
    }
    
    // Parse type
    if (pos < specLen) {
        type = specStr[pos];
    }
    
    // Format the value based on type
    std::ostringstream oss;
    
    if (precision >= 0) {
        oss << std::setprecision(precision);
    }
    
    if (altForm) {
        oss << std::showpoint;
    }
    
    switch (type) {
        case 'f':
        case 'F':
            oss << std::fixed << value;
            break;
        case 'e':
            oss << std::scientific << std::nouppercase << value;
            break;
        case 'E':
            oss << std::scientific << std::uppercase << value;
            break;
        case 'g':
        case 'G':
        default:
            oss << value;
            break;
    }
    
    GString result = oss.str();
    
    // Add sign for positive numbers if requested
    if (value >= 0 && sign == '+') {
        result = GString("+") + result;
    } else if (value >= 0 && sign == ' ') {
        result = GString(" ") + result;
    }
    
    // Apply width and alignment
    if (width > 0 && result.count() < width) {
        int32_t padLen = width - result.count();
        GString padding;
        for (int i = 0; i < padLen; i++) {
            padding += fillChar;
        }
        
        if (align == '<') {
            result = result + padding;
        } else if (align == '^') {
            int32_t leftPad = padLen / 2;
            int32_t rightPad = padLen - leftPad;
            GString leftPadding, rightPadding;
            for (int i = 0; i < leftPad; i++) leftPadding += fillChar;
            for (int i = 0; i < rightPad; i++) rightPadding += fillChar;
            result = leftPadding + result + rightPadding;
        } else { // '>'
            result = padding + result;
        }
    }
    
    return result;
}

//=============== Begin Static ================//

GString GString::fromCodepoint(uint32_t codepoint)
{
    uint8_t c[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    if (codepoint <= 0x7F) {
        c[0] = codepoint;
    } else if (codepoint <= 0x7FF) {
        c[0] = (codepoint >> 6) + 192;
        c[1] = (codepoint & 63) + 128;
    } else if (0xd800 <= codepoint && codepoint <= 0xdfff) {
    } //invalid block of utf8
    else if (codepoint <= 0xFFFF) {
        c[0] = (codepoint >> 12) + 224;
        c[1] = ((codepoint >> 6) & 63) + 128;
        c[2] = (codepoint & 63) + 128;
    } else if (codepoint <= 0x10FFFF) {
        c[0] = (codepoint >> 18) + 240;
        c[1] = ((codepoint >> 12) & 63) + 128;
        c[2] = ((codepoint >> 6) & 63) + 128;
        c[3] = (codepoint & 63) + 128;
    }

    GString newStr;
    newStr.mString.append(reinterpret_cast<const char *>(c), static_cast<int32_t>(strlen(reinterpret_cast<char *>(c))));
    newStr.mCharOffsets.push_back(0);
    return newStr;
}

int32_t preNUM(unsigned char byte)
{
    unsigned char mask = 0x80;
    int32_t num = 0;
    for (int32_t i = 0; i < 8; i++) {
        if ((byte & mask) == mask) {
            mask = mask >> 1;
            num++;
        } else {
            break;
        }
    }
    return num;
}

bool GString::isUtf8(const char *str, size_t len)
{
    size_t i = 0;
    while (i < len) {
        const unsigned char byte = static_cast<unsigned char>(str[i]);
        int bytesToCheck = 0;

        if (byte <= 0x7F) { // 0xxxxxxx (ASCII, 1-byte)
            i += 1;
            continue;
        }
        if (byte >= 0xC2 && byte <= 0xDF) { // 110xxxxx (2-bytes)
            bytesToCheck = 1;
        } else if (byte >= 0xE0 && byte <= 0xEF) { // 1110xxxx (3-bytes)
            bytesToCheck = 2;
        } else if (byte >= 0xF0 && byte <= 0xF4) { // 11110xxx (4-bytes)
            bytesToCheck = 3;
        } else {
            // 无效的起始字节，包括：
            // 0xC0, 0xC1 (过长编码)
            // 0xF5-0xFF (超出Unicode范围)
            return false;
        }

        if (i + bytesToCheck >= len) {
            return false;
        }

        for (int j = 0; j < bytesToCheck; ++j) {
            const unsigned char nextByte = static_cast<unsigned char>(str[i + 1 + j]);
            if (nextByte < 0x80 || nextByte > 0xBF) {
                return false;
            }
        }

        if (bytesToCheck == 1) { // 2-byte sequence
            // 1100000x ... is overlong
            if ((byte & 0xFE) == 0xC0)
                return false;
        } else if (bytesToCheck == 2) { // 3-byte sequence
            // 11100000 100xxxxx ... is overlong
            if (byte == 0xE0 && (static_cast<unsigned char>(str[i + 1]) & 0xE0) == 0x80)
                return false;
        } else { // 4-byte sequence
            // 11110000 100xxxxx ... is overlong
            if (byte == 0xF0 && (static_cast<unsigned char>(str[i + 1]) & 0xF0) == 0x80)
                return false;
        }

        i += (bytesToCheck + 1);
    }
    return true;
}

bool GString::isGBK(const char *str, size_t len)
{
    size_t i = 0;
    while (i < len) {
        if (str[i] <= 0x7f) {
            //编码小于等于127,只有一个字节的编码，兼容ASCII
            i++;
        } else {
            //大于127的使用双字节编码
            if (str[i] >= static_cast<char>(0x81) &&
                str[i] <= static_cast<char>(0xfe) &&
                str[i + 1] >= static_cast<char>(0x40) &&
                str[i + 1] <= static_cast<char>(0xfe) &&
                str[i + 1] != static_cast<char>(0xf7)) {
                i += 2;
            } else {
                return false;
            }
        }
    }
    return true;
}

GString GString::fromGBK(const std::string &str)
{
#if GX_PLATFORM_WINDOWS
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
    auto *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    char *cstr = new char[len + 1];
    memset(cstr, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, cstr, len, nullptr, nullptr);

    GString res;
    res.mString.clear();
    res.mString.append(cstr, len);

    delete[] wstr;
    delete[] cstr;

    return res;
#elif GX_USE_ICONV
    size_t srcLen = str.size();
    const size_t dstLen = srcLen * 2 + 1;
    char *dstStr = new char[dstLen];

    const iconv_t cd = iconv_open("utf8", "gbk");
    if (cd == nullptr) {
        delete[] dstStr;
        return str;
    }
    char *srcStr = const_cast<char *>(str.c_str());
    char *pDstStr = dstStr;
    size_t dstLenLeft = dstLen;
    if (iconv(cd, &srcStr, &srcLen, &pDstStr, &dstLenLeft) >= 0) {
        GString res(dstStr, static_cast<int32_t>(dstLen - dstLenLeft));
        iconv_close(cd);
        delete[] dstStr;
        return res;
    }
    iconv_close(cd);
    delete[] dstStr;
    return str;
#else
    return str;
#endif
}

//=============== End Static ================//

void GString::_build(const char *str, int32_t size)
{
    const size_t length = size >= 0 ? std::min(static_cast<size_t>(size), strlen(str)) : strlen(str);

    mString = std::string(str, length);
    // if (!isUtf8(str, length) && isGBK(str, length)) {
    //     *this = fromGBK(std::string(str, length));
    // }

    _rebuildIndex();
}

void GString::_build(const GString &str)
{
    mString = str.mString;
    _rebuildIndex();
}

void GString::_build(char c)
{
    mString = c;
    _rebuildIndex();
}

void GString::_clear()
{
    mString.clear();
    mCharOffsets.clear();
}

int32_t GString::_next(int32_t charIndex) const
{
    const int32_t byteCount = this->count();
    if (charIndex >= byteCount) {
        return byteCount;
    }

    const unsigned char firstByte = static_cast<unsigned char>(this->data()[charIndex]);
    int32_t charByteLen;

    if (firstByte < 0x80) { // 0xxxxxxx (ASCII)
        charByteLen = 1;
    } else if (firstByte >= 0xC2 && firstByte <= 0xDF) { // 110xxxxx (2Byte)
        charByteLen = 2;
    } else if (firstByte >= 0xE0 && firstByte <= 0xEF) { // 1110xxxx (3Byte)
        charByteLen = 3;
    } else if (firstByte >= 0xF0 && firstByte <= 0xF4) { // 11110xxx (4Byte)
        charByteLen = 4;
    } else {
        charByteLen = 1;
    }

    return charIndex + charByteLen;
}

int32_t GString::_seek(int32_t uniCharSize) const
{
    if (uniCharSize <= 0) {
        return 0;
    }
    if (uniCharSize >= mCharOffsets.size()) {
        return count();
    }
    return mCharOffsets[uniCharSize];
}

void GString::_rebuildIndex()
{
    mCharOffsets.clear();
    const int32_t byteCount = count();

    if (byteCount > 0) {
        mCharOffsets.reserve(byteCount / 2);
    }

    int32_t i = 0;
    while (i < byteCount) {
        mCharOffsets.push_back(i);
        i = _next(i);
    }
}

GString GString::_atChar(int32_t index) const
{
    const int32_t i = _seek(index);
    const int32_t j = _seek(index + 1);
    if (j <= i) {
        return GString();
    }
    return {data() + i, j - i};
}

#ifdef ENABLE_WSTRING

GWString::GWString()
    : mData(1)
{
}

GWString::GWString(const wchar_t *wStr, int32_t size)
{
    if (size <= 0) {
        size = static_cast<int32_t>(wcslen(wStr));
    }
    mData.resize(size + 1);
    memcpy(mData.data(), wStr, sizeof(wchar_t) * size);
    mData[size] = L'\0';
}

GWString::GWString(const std::wstring &wStr)
{
    mData.resize(wStr.size() + 1);
    memcpy(mData.data(), wStr.data(), sizeof(wchar_t) * wStr.size());
    mData[wStr.size()] = L'\0';
}

GWString::GWString(const GString &str)
{
    const int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.count(), nullptr, 0);
    mData.resize(len + 1);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.count(), mData.data(), len);
    mData[len] = L'\0';
}

GWString::GWString(const GWString &b) = default;

GWString::GWString(GWString &&b) noexcept
    : mData(std::move(b.mData))
{
}

GWString &GWString::operator=(const GWString &b)
{
    if (this != &b) {
        mData = b.mData;
    }
    return *this;
}

GWString &GWString::operator=(GWString &&b) noexcept
{
    if (this != &b) {
        std::swap(mData, b.mData);
    }
    return *this;
}

const wchar_t *GWString::data() const
{
    return mData.data();
}

int32_t GWString::length() const
{
    return static_cast<int32_t>(mData.size()) - 1;
}

#endif //ENABLE_WSTRING
