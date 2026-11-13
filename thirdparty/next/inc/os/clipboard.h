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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <astring.h>

/*!
    \class Clipboard
    \brief Provides access to the system text clipboard.
    \inmodule OS
*/
class NEXT_LIBRARY_EXPORT Clipboard {
public:
    //! Returns text currently stored in the system clipboard.
    static TString text();
    //! Replaces the clipboard contents with \a text.
    static bool setText(const TString &text);
    //! Clears the system clipboard.
    static bool clear();

private:
    Clipboard() = delete;
};

#endif // CLIPBOARD_H
