#include <Foundation/Foundation.h>
#include <pwd.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

namespace {

// Копирует NSString в malloc-строку UTF-8. Возвращает nullptr при пустом входе.
char *copyUtf8(NSString *s) {
    if(!s) {
        return nullptr;
    }
    const char *utf8 = [s UTF8String];
    if(!utf8) {
        return nullptr;
    }
    size_t len = std::strlen(utf8);
    char *out = static_cast<char*>(std::malloc(len + 1));
    if(!out) {
        return nullptr;
    }
    std::memcpy(out, utf8, len);
    out[len] = '\0';
    return out;
}

char *copyStd(const std::string &s) {
    if(s.empty()) {
        return nullptr;
    }
    char *out = static_cast<char*>(std::malloc(s.size() + 1));
    if(!out) {
        return nullptr;
    }
    std::memcpy(out, s.c_str(), s.size());
    out[s.size()] = '\0';
    return out;
}

} // namespace

extern "C" {

char *sp_mac_search_path(int dir) {
    @autoreleasepool {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(
            (NSSearchPathDirectory)dir, NSUserDomainMask, YES);
        if([paths count] > 0) {
            return copyUtf8([paths objectAtIndex:0]);
        }
    }
    return nullptr;
}

char *sp_mac_temporary_directory() {
    @autoreleasepool {
        return copyUtf8(NSTemporaryDirectory());
    }
}

char *sp_mac_home_directory() {
    const char *home = std::getenv("HOME");
    if(home && *home) {
        return copyStd(std::string(home));
    }
    struct passwd *pw = getpwuid(getuid());
    if(pw && pw->pw_dir) {
        return copyStd(std::string(pw->pw_dir));
    }
    return nullptr;
}

} // extern "C"
