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

#include <os/menubar.h>

#ifdef __APPLE__

#import <Cocoa/Cocoa.h>

@interface NextMenuActionTarget : NSObject {
@public
    NativeMenu *menu;
    size_t itemIndex;
}
- (void)invoke:(id)sender;
@end

@implementation NextMenuActionTarget
- (void)invoke:(id)sender {
    (void)sender;
    if(menu) {
        menu->triggerAction(itemIndex);
    }
}
@end

namespace {
    NSString *toNSString(const TString &text) {
        return [NSString stringWithUTF8String:text.data()];
    }

    void appendMenuItems(NSMenu *nativeMenu, NativeMenu *menu) {
        const auto &items = menu->items();
        for(size_t index = 0; index < items.size(); ++index) {
            const NativeMenu::Item &item = items[index];
            if(item.separator) {
                [nativeMenu addItem:[NSMenuItem separatorItem]];
            } else if(item.submenu) {
                NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:toNSString(item.submenu->title())
                    action:nil keyEquivalent:@""];
                NSMenu *submenu = [[NSMenu alloc] initWithTitle:toNSString(item.submenu->title())];
                appendMenuItems(submenu, item.submenu);
                [menuItem setSubmenu:submenu];
                [nativeMenu addItem:menuItem];
                [menuItem release];
                [submenu release];
            } else {
                NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:toNSString(item.text)
                    action:@selector(invoke:) keyEquivalent:@""];
                NextMenuActionTarget *target = [[NextMenuActionTarget alloc] init];
                target->menu = menu;
                target->itemIndex = index;
                [menuItem setTarget:target];
                [menuItem setRepresentedObject:target];
                [menuItem setEnabled:item.enabled];
                if(item.checkable) {
                    [menuItem setState:item.checked ? NSControlStateValueOn : NSControlStateValueOff];
                }
                [nativeMenu addItem:menuItem];
                [target release];
                [menuItem release];
            }
        }
    }

    NSMenu *createNativeMenuBar(NativeMenuBar *menuBar) {
        NSMenu *result = [[NSMenu alloc] initWithTitle:@""];
        for(const auto &menu : menuBar->menus()) {
            NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:toNSString(menu->title())
                action:nil keyEquivalent:@""];
            NSMenu *submenu = [[NSMenu alloc] initWithTitle:toNSString(menu->title())];
            appendMenuItems(submenu, menu.get());
            [menuItem setSubmenu:submenu];
            [result addItem:menuItem];
            [menuItem release];
            [submenu release];
        }
        return result;
    }
}

bool menuBarNativeSupported() {
    return true;
}

bool menuBarNativeAttach(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu) {
    (void)nativeWindow;
    @autoreleasepool {
        NSMenu *menu = createNativeMenuBar(menuBar);
        [NSApp setMainMenu:menu];
        *nativeMenu = menu;
        return true;
    }
}

bool menuBarNativeRefresh(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu) {
    (void)nativeWindow;
    @autoreleasepool {
        NSMenu *oldMenu = reinterpret_cast<NSMenu *>(*nativeMenu);
        NSMenu *menu = createNativeMenuBar(menuBar);
        [NSApp setMainMenu:menu];
        [oldMenu release];
        *nativeMenu = menu;
        return true;
    }
}

void menuBarNativeDetach(void *nativeWindow, void *nativeMenu) {
    (void)nativeWindow;
    @autoreleasepool {
        NSMenu *menu = reinterpret_cast<NSMenu *>(nativeMenu);
        if([NSApp mainMenu] == menu) {
            [NSApp setMainMenu:nil];
        }
        [menu release];
    }
}

#endif // __APPLE__
