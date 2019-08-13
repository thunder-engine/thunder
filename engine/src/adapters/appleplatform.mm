#import <Foundation/Foundation.h>

const char *configLocation() {
    return [[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) lastObject] cStringUsingEncoding:NSASCIIStringEncoding];
}

const char *assetsLocation() {
    return [[NSBundle.mainBundle.resourcePath stringByAppendingString:@"/assets"] cStringUsingEncoding:NSASCIIStringEncoding];
}

const char *appBundle() {
    return [[[NSBundle mainBundle] bundleIdentifier] cStringUsingEncoding:NSASCIIStringEncoding];
}
