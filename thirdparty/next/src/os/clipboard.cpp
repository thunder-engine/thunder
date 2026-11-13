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

#include <cstring>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "User32.lib")
#elif defined(__linux__)
#include <gtk/gtk.h>
#endif

#ifdef __linux__
namespace {

bool ensureGtkInitialized() {
    static bool initialized = false;
    if(!initialized) {
        initialized = gtk_init_check(nullptr, nullptr) != FALSE;
    }
    return initialized;
}

} // namespace
#endif

#ifndef __APPLE__

#ifdef _WIN32
namespace {

std::wstring toWide(const TString &text) {
    const std::string utf8 = text.toStdString();
    if(utf8.empty()) {
        return std::wstring();
    }
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1, nullptr, 0);
    if(size <= 0) {
        return std::wstring();
    }
    std::wstring result(static_cast<size_t>(size), L'\0');
    if(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1, &result[0], size) <= 0) {
        return std::wstring();
    }
    result.resize(static_cast<size_t>(size - 1));
    return result;
}

TString fromWide(const wchar_t *text) {
    if(!text || !*text) {
        return TString();
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if(size <= 0) {
        return TString();
    }
    std::string result(static_cast<size_t>(size), '\0');
    if(WideCharToMultiByte(CP_UTF8, 0, text, -1, &result[0], size, nullptr, nullptr) <= 0) {
        return TString();
    }
    result.resize(static_cast<size_t>(size - 1));
    return TString(result);
}

bool openClipboard() {
    for(int attempt = 0; attempt < 5; ++attempt) {
        if(OpenClipboard(nullptr)) {
            return true;
        }
        Sleep(1);
    }
    return false;
}

} // namespace
#endif

TString Clipboard::text() {
#ifdef _WIN32
    if(!openClipboard()) {
        return TString();
    }

    TString result;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if(data) {
        const wchar_t *value = static_cast<const wchar_t *>(GlobalLock(data));
        if(value) {
            result = fromWide(value);
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
    return result;
#elif defined(__linux__)
    if(!ensureGtkInitialized()) {
        return TString();
    }

    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gchar *value = gtk_clipboard_wait_for_text(clipboard);
    if(!value) {
        return TString();
    }
    TString result(value);
    g_free(value);
    return result;
#else
    return TString();
#endif
}

bool Clipboard::setText(const TString &text) {
#ifdef _WIN32
    const std::wstring value = toWide(text);
    if(!text.isEmpty() && value.empty()) {
        return false;
    }
    if(!openClipboard()) {
        return false;
    }
    if(!EmptyClipboard()) {
        CloseClipboard();
        return false;
    }

    const SIZE_T size = (value.size() + 1) * sizeof(wchar_t);
    HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE, size);
    if(!data) {
        CloseClipboard();
        return false;
    }
    void *memory = GlobalLock(data);
    if(!memory) {
        GlobalFree(data);
        CloseClipboard();
        return false;
    }
    std::memcpy(memory, value.c_str(), size);
    GlobalUnlock(data);
    if(!SetClipboardData(CF_UNICODETEXT, data)) {
        GlobalFree(data);
        CloseClipboard();
        return false;
    }
    CloseClipboard();
    return true;
#elif defined(__linux__)
    if(!ensureGtkInitialized()) {
        return false;
    }

    const std::string value = text.toStdString();
    if(value.size() > static_cast<size_t>(G_MAXINT)) {
        return false;
    }
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, value.c_str(), static_cast<gint>(value.size()));
    gtk_clipboard_store(clipboard);
    return true;
#else
    (void)text;
    return false;
#endif
}

bool Clipboard::clear() {
#ifdef _WIN32
    if(!openClipboard()) {
        return false;
    }
    const bool result = EmptyClipboard() != FALSE;
    CloseClipboard();
    return result;
#elif defined(__linux__)
    if(!ensureGtkInitialized()) {
        return false;
    }
    gtk_clipboard_clear(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
    return true;
#else
    return false;
#endif
}

#endif // !__APPLE__
