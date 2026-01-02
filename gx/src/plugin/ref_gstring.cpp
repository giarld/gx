//
// Created by Gxin on 2023/10/13.
//

#include "gx/gstring.h"
#include "gx/gany.h"


using namespace gany;

void refGString()
{
    Class<GString>("Gx", "GString", "Gx string, A UTF-8 string and a series of processing algorithms for UTF-8.")
            .construct<>()
            .construct<const char *>()
            .construct<const char *, int32_t>()
#if GX_PLATFORM_WINDOWS
            .construct<const GWString &>({"Create from wide string (Windows platforms only)."})
#endif
            .construct<const GString &>()
            .construct<char>()
            .construct<std::string>()
            .func("count", &GString::count, {.doc = "Gets the byte length of the string."})
            .func("length", &GString::length, {.doc = "Get string length (number of words)."})
            .func("data", &GString::data, {.doc = "Back buffer header pointer."})
            .func("c_str", &GString::c_str, {.doc = "Back buffer header pointer."})
            .func("at", [](const GString &self, int32_t index) {
                return self.at(index);
            }, {.doc = "Gets the character at the specified position.", .args = {"index"}})
            .func("toStdString", &GString::toStdString, {.doc = "Convert to std::string."})
            .func("reset", [](GString &self, const char *str) {
                self.reset(str);
            }, {.doc = "Reset to specified string.", .args = {"str"}})
            .func("reset", [](GString &self, const char *str, int32_t size) {
                self.reset(str, size);
            }, {.doc = "Reset to specified string.", .args = {"str", "size"}})
            .func("reset", [](GString &self, const GString &str) {
                self.reset(str);
            }, {.doc = "Reset to specified string.", .args = {"str"}})
            .func("reset", [](GString &self, const std::string &str) {
                self.reset(str);
            }, {.doc = "Reset to specified string.", .args = {"str"}})
            .func("compare", &GString::compare, {.doc = "Compare whether two strings are the same.", .args = {"bStr"}})
            .func("compare", [](const GString &self, const std::string &b) {
                return self.compare(b);
            }, {.doc = "Compare whether two strings are the same.", .args = {"bStr"}})
            .func("append", [](GString &self, const GString &str) {
                return self.append(str);
            }, {.doc = "Add a string at the end of the string.", .args = {"str"}})
            .func("append", [](GString &self, const std::vector<uint32_t> &codepoints) {
                return self.append(codepoints);
            }, {.doc = "Add a string at the end of the string.", .args = {"codepoints"}})
            .func("append", [](GString &self, const std::string &b) {
                return self.append(b);
            }, {.doc = "Add a string at the end of the string.", .args = {"str"}})
            .func("insert", &GString::insert, {.doc = "Inserts a string at the specified position.", .args = {"index", "str"}})
            .func("insert", [](GString &self, int32_t index, const std::string &b) {
                return self.insert(index, b);
            }, {.doc = "Inserts a string at the specified position.", .args = {"index", "str"}})
            .func("isEmpty", &GString::isEmpty, {.doc = "Check if the string is empty."})
            .func("startWith", &GString::startWith,
                  {.doc = "Check whether the string starts with the specified string.", .args = {"str"}})
            .func("startWith", [](const GString &self, const std::string &b) {
                return self.startWith(b);
            }, {.doc = "Check whether the string starts with the specified string.", .args = {"str"}})
            .func("endWith", &GString::endWith, {.doc = "Checks whether the string ends with the specified string.", .args = {"str"}})
            .func("endWith", [](const GString &self, const std::string &b) {
                return self.endWith(b);
            }, {.doc = "Checks whether the string ends with the specified string.", .args = {"str"}})
            .func("left", &GString::left, {.doc = "Intercepts the left specified length string.", .args = {"n"}})
            .func("right", &GString::right, {.doc = "Intercepts the right specified length string.", .args = {"n"}})
            .func("substring", &GString::substring,
                  {.doc = "Intercept the string with length len from begin.", .args = {"begin", "len"}})
            .func("substring", [](const GString &self, int32_t begin) {
                return self.substring(begin);
            }, {.doc = "Intercept the string with length len from begin.", .args = {"begin", "len"}})
            .func("replace", [](const GString &self, const GString &before, const GString &after, int32_t begin) {
                return self.replace(before, after, begin);
            }, {.doc = "Search from begin and replace all before in the string with after.", .args = {"before", "after", "begin"}})
            .func("replace", [](const GString &self, const std::string &before, const std::string &after, int32_t begin) {
                return self.replace(before, after, begin);
            }, {.doc = "Search from begin and replace all before in the string with after.", .args = {"before", "after", "begin"}})
            .func("replace", [](const GString &self, const GString &before, const GString &after) {
                return self.replace(before, after);
            }, {.doc = "Search from 0 and replace all before in the string with after.", .args = {"before", "after"}})
            .func("replace", [](const GString &self, const std::string &before, const std::string &after) {
                return self.replace(before, after);
            }, {.doc = "Search from 0 and replace all before in the string with after.", .args = {"before", "after"}})
            .func("split", &GString::split, {.doc = "Cut the string with the specified string as the separator.", .args = {"cs"}})
            .func("split", [](const GString &self, const std::string &cs) {
                return self.split(cs);
            }, {.doc = "Cut the string with the specified string as the separator.", .args = {"cs"}})
            .func("indexOf", [](const GString &self, const GString &str, int32_t from) {
                return self.indexOf(str, from);
            }, {.doc = "Find the position subscript of the first matching str starting from.", .args = {"str", "from"}})
            .func("indexOf", [](const GString &self, const std::string &str, int32_t from) {
                return self.indexOf(str, from);
            }, {.doc = "Find the position subscript of the first matching str starting from.", .args = {"str", "from"}})
            .func("indexOf", [](const GString &self, const GString &str) {
                return self.indexOf(str);
            }, {.doc = "Find the position subscript of the first matching str starting from 0.", .args = {"str"}})
            .func("indexOf", [](const GString &self, const std::string &str) {
                return self.indexOf(str);
            }, {.doc = "Find the position subscript of the first matching str starting from 0.", .args = {"str"}})
            .func(
                "lastIndexOf",
                [](const GString &self, const GString &str, int32_t from) {
                    return self.lastIndexOf(str, from);
                },
                {
                    .doc = "Reverse search the position subscript of the first matching \"str\". "
                    "When the parameter \"from\" is a positive number, it indicates the position from the beginning. "
                    "For example, 0 indicates the first position, a negative number indicates the position from the end, "
                    "for example, -1 indicates the last position.",
                    .args = {"str", "from"}
                })
            .func(
                "lastIndexOf",
                [](const GString &self, const std::string &str, int32_t from) {
                    return self.lastIndexOf(str, from);
                },
                {
                    .doc = "Reverse search the position subscript of the first matching \"str\". "
                    "When the parameter \"from\" is a positive number, it indicates the position from the beginning. "
                    "For example, 0 indicates the first position, a negative number indicates the position from the end, "
                    "for example, -1 indicates the last position.",
                    .args = {"str", "from"}
                })
            .func(
                "lastIndexOf",
                [](const GString &self, const GString &str) {
                    return self.lastIndexOf(str);
                },
                {
                    .doc = "Reverse search the position subscript of the first matching \"str\".",
                    .args = {"str"}
                })
            .func(
                "lastIndexOf",
                [](const GString &self, const std::string &str) {
                    return self.lastIndexOf(str);
                },
                {
                    .doc = "Reverse search the position subscript of the first matching \"str\".",
                    .args = {"str"}
                })
            .func("charPosition", &GString::charPosition, {.doc = "Retrieve the Unicode character position corresponding to the byte offset.", .args = {"byteOffset"}})
            .func("swap", &GString::swap, {.doc = "Swap with another string.", .args = {"b"}})
            .func("codepoint", [](const GString &self, int32_t index) {
                return static_cast<int32_t>(self.codepoint(index));
            }, {.doc = "Get Unicode of the specified character.", .args = {"index"}})
#if GX_PLATFORM_WINDOWS
            .func("toUtf16", &GString::toUtf16, {"Convert string to UTF-16, windows platform only."})
#endif
            .func("toUpper", &GString::toUpper, {.doc = "Converts all lowercase letters in a string to uppercase letters."})
            .func("toLower", &GString::toLower, {.doc = "Converts all uppercase letters in a string to lowercase letters."})
            .func("arg", [](const GString &self, const GAny &value) {
                return self.arg(value.toString());
            }, {.doc = "Replace the string of '{}' area with the string of value.", .args = {"value"}})
            .func("arg", [](const GString &self, const std::string &key, const GAny &value) {
                return self.arg(key, value.toString());
            }, {.doc = "Replace the string of key area with the string of value.", .args = {"key", "value"}})
            .staticFunc("toString", [](const GAny &value) {
                return GString(value.toString());
            }, {
                .doc = "Convert any type to GString, as long as the original MetaFunction::ToString of this type is valid.",
                .args = {"value"}
            })
            .staticFunc("fromCodepoint", [](int32_t codepoint) {
                return GString::fromCodepoint(codepoint);
            }, {.doc = "Create a string from Unicode.", .args = {"codepoint"}})
            .staticFunc("isUtf8", [](const std::string &str) {
                return GString::isUtf8(str.c_str(), str.length());
            }, {.doc = "Check if it is a UTF-8 string", .args = {"str"}})
            .staticFunc("isGBK", [](const std::string &str) {
                return GString::isGBK(str.c_str(), str.length());
            }, {.doc = "Check if it is a GBK string", .args = {"str"}})
            .staticFunc("fromGBK", [](const std::string &str) {
                return GString::fromGBK(str);
            }, {.doc = "Constructing from GBK string will convert character encoding to UTF-8.", .args = {"str"}})
            .func(MetaFunction::ToString, [](const GString &self) {
                return self.toString();
            })
            .func(MetaFunction::Addition, [](const GString &self, const GString &b) {
                return self + b;
            })
            .func(MetaFunction::Length, [](const GString &self) {
                return static_cast<size_t>(self.length());
            })
            .func(MetaFunction::EqualTo, &GString::operator==)
            .func(MetaFunction::LessThan, &GString::operator<)
            .func(MetaFunction::GetItem, [](const GString &self, int32_t index) {
                return self.at(index);
            });

#if GX_PLATFORM_WINDOWS
    Class<GWString>("Gx", "GWString", "Gx wstring.")
            .construct<>()
            .construct<const wchar_t *>()
            .construct<const wchar_t *, int32_t>()
            .construct<const std::wstring &>()
            .construct<const GString &>()
            .construct<const GWString &>()
            .func("data", &GWString::data)
            .func("length", &GWString::length);
#endif
}
