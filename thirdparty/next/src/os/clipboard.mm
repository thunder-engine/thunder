/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "os/clipboard.h"

#import <AppKit/AppKit.h>

#include <string>

TString Clipboard::text() {
    @autoreleasepool {
        NSString *value = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        const char *utf8 = value ? [value UTF8String] : nullptr;
        return utf8 ? TString(utf8) : TString();
    }
}

bool Clipboard::setText(const TString &text) {
    @autoreleasepool {
        const std::string value = text.toStdString();
        NSString *string = [[NSString alloc] initWithBytes:value.data()
                                                  length:value.size()
                                                encoding:NSUTF8StringEncoding];
        if(!string) {
            return false;
        }
        NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard clearContents];
        const bool result = [pasteboard setString:string forType:NSPasteboardTypeString] == YES;
        [string release];
        return result;
    }
}

bool Clipboard::clear() {
    @autoreleasepool {
        return [[NSPasteboard generalPasteboard] clearContents] >= 0;
    }
}
