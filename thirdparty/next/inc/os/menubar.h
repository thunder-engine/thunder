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

#ifndef NATIVEMENUBAR_H
#define NATIVEMENUBAR_H

#include <os/nativemenu.h>

#include <memory>
#include <functional>
#include <cstdint>
#include <map>
#include <vector>

class NEXT_LIBRARY_EXPORT NativeMenuBar {
public:
    using ChangeSubscription = uint64_t;
    using ChangedHandler = std::function<void()>;

    NativeMenuBar();
    ~NativeMenuBar();

    NativeMenuBar(const NativeMenuBar &) = delete;
    NativeMenuBar &operator=(const NativeMenuBar &) = delete;

    NativeMenu *addMenu(const TString &title);
    const std::vector<std::unique_ptr<NativeMenu>> &menus() const;

    bool attach(void *nativeWindow);
    void detach();
    bool isAttached() const;
    bool isAttachedTo(void *nativeWindow) const;
    static bool isNativeSupported();
    bool triggerShortcut(int key, int modifiers);

    ChangeSubscription subscribeChanged(const ChangedHandler &handler);
    void unsubscribeChanged(ChangeSubscription subscription);

private:
    friend class NativeMenu;

    void rebuild();

    void *m_nativeWindow;
    void *m_nativeMenu;
    ChangeSubscription m_nextSubscription;
    std::map<ChangeSubscription, ChangedHandler> m_changedHandlers;
    std::vector<std::unique_ptr<NativeMenu>> m_menus;
};

#endif // NATIVEMENUBAR_H
