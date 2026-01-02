//
// Created by Gxin on 2023/5/30.
//

#include "gx/gfile.h"

#if GX_PLATFORM_OSX
#import <mach-o/dyld.h>
#import <Foundation/Foundation.h>

GFile GFile::mainDirectory()
{
    char buffer[PATH_MAX];

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle) {
        CFURLRef resURL = CFBundleCopyResourcesDirectoryURL(bundle);
        if (resURL) {
            if (CFURLGetFileSystemRepresentation(resURL, true, (UInt8*)buffer, PATH_MAX)) {
                CFRelease(resURL);
                return GFile(std::string(buffer));
            }
            CFRelease(resURL);
        }
    }

    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return GFile(std::string(buffer)).parent();
    }
    return GFile("./");
}

GFile GFile::temporaryDirectory()
{
    NSString* tempDir = NSTemporaryDirectory();
    return GFile([tempDir cStringUsingEncoding:NSUTF8StringEncoding]);
}

#elif  GX_PLATFORM_IOS
#import <Foundation/Foundation.h>

GFile GFile::mainDirectory()
{
    NSString *documentsDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    return GFile([documentsDir UTF8String]);
}

GFile GFile::temporaryDirectory()
{
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) firstObject];
    return GFile([cacheDir UTF8String]);
}
#endif