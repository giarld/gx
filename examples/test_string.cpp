//
// Created by Gxin on 2020/5/12.
//
#include <string>

#include <gx/debug.h>
#include <gx/gstring.h>
#include <gx/platform.h>

#include <fstream>


int main(int argc, char *argv[])
{
#if GX_PLATFORM_WINDOWS
    system("chcp 65001");
#endif

    GString ssss;

    GString sa = "Hello {1}, {2}";
    GString sb = sa.arg("World").arg("GX");
    GString sc = sa.arg("1", "世界").arg("2", true);

    Log(">>> {}", GString("{{{}}} {}").arg(1).arg(2));

    sc = sc.toUpper();

    Log("sa = {}", sa);
    Log("sb = {}", sb);
    Log("sc = {}", sc);

    GString pa = "1";
    pa.append("2");
    pa.append("3");
    pa.append("4");
    pa.append(pa);
    pa.append(pa);

    Log("pa = {}", pa);

    GString url = "https://www.bilibili.com";
    if (url.startWith("https")) {
        Log("is https url = {}", url);
        GString host = url.substring(GString("https://").length());
        Log("host = {}", host);
    }

    for(int i=0x4f60; i<0x4f80; i++) // 你 to 使
    {
        GString s = GString::fromCodepoint(i);
        Log("s = {} ({:#x})", s, s.codepoint(0));
    }

    // Test new format functionality (similar to fmt library)
    Log("\n=== Format Function Tests ===");
    
    // Integer formatting
    GString fmt1 = "Decimal: {}, Hex: {:x}, Octal: {:o}, Binary: {:b}";
    Log("{}", fmt1.arg(255).arg(255).arg(255).arg(255));
    
    // Floating point formatting
    GString fmt2 = "Pi: {:.2f}, E: {:.4f}";
    Log("{}", fmt2.arg(3.14159).arg(2.71828));
    
    // Width and alignment
    GString fmt3 = "[Right: {:>10}, Left: {:<10}, Center: {:^10}]";
    Log("{}", fmt3.arg(42).arg(42).arg(42));
    
    // Zero padding
    GString fmt4 = "Padded: {:05d}";
    Log("{}", fmt4.arg(42));
    
    // Sign formatting
    GString fmt5 = "Plus: {:+d}, Space: {: d}";
    Log("{}", fmt5.arg(42).arg(42));
    
    // Hex with prefix
    GString fmt6 = "Hex: {:#x}, Hex Upper: {:#X}";
    Log("{}", fmt6.arg(255).arg(255));
    
    // Scientific notation
    GString fmt7 = "Sci: {:e}, SciUpper: {:E}";
    Log("{}", fmt7.arg(1234.5).arg(1234.5));
    
    // Combined formatting
    GString fmt8 = "Value: {:>8.2f}";
    Log("{}", fmt8.arg(3.14159));

    return EXIT_SUCCESS;
}