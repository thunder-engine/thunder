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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <astring.h>

class MessagePrivate;

class NEXT_LIBRARY_EXPORT Message {
public:
    enum Icon {
        NoIcon,
        Information,
        Warning,
        Critical,
        Question
    };

    enum StandardButton {
        NoButton        = 0x00000000,
        Ok              = 0x00000400,
        Save            = 0x00000800,
        SaveAll         = 0x00001000,
        Open            = 0x00002000,
        Yes             = 0x00004000,
        YesToAll        = 0x00008000,
        No              = 0x00010000,
        NoToAll         = 0x00020000,
        Abort           = 0x00040000,
        Retry           = 0x00080000,
        Ignore          = 0x00100000,
        Close           = 0x00200000,
        Cancel          = 0x00400000,
        Discard         = 0x00800000,
        Help            = 0x01000000,
        Apply           = 0x02000000,
        Reset           = 0x04000000,
        RestoreDefaults = 0x08000000
    };

    enum ButtonRole {
        InvalidRole = -1,
        AcceptRole,
        RejectRole,
        DestructiveRole,
        ActionRole,
        HelpRole,
        YesRole,
        NoRole,
        ResetRole,
        ApplyRole
    };

    Message();
    ~Message();

    Message(const Message&) = delete;
    Message &operator=(const Message&) = delete;

    Message(Message&&) noexcept;
    Message &operator=(Message&&) noexcept;

    void setWindowTitle(const TString &title);
    TString text() const;
    void setText(const TString &text);
    void setInformativeText(const TString &text);
    void setDetailedText(const TString &text);
    void setIcon(Icon icon);

    void setStandardButtons(int buttons);
    void addButton(const TString &text, ButtonRole role);
    void setDefaultButton(StandardButton button);
    void setEscapeButton(StandardButton button);

    int exec();

    StandardButton standardButtonClicked() const;
    int customButtonClicked() const;
    TString clickedButtonText() const;

private:
    MessagePrivate *m_ptr;

};

#endif // MESSAGE_H
