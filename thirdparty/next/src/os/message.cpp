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

#include "os/message.h"

#include <vector>

class MessagePrivate {
public:
    MessagePrivate() :
        m_icon(Message::NoIcon),
        m_standardButtons(Message::Ok),
        m_defaultButton(Message::NoButton),
        m_escapeButton(Message::NoButton),
        m_clickedStandardButton(Message::NoButton),
        m_clickedCustomButton(-1) {
    }

    ~MessagePrivate() = default;

    bool exec();

private:
    friend class Message;

    struct CustomButton {
        TString text;
        Message::ButtonRole role;
    };

    // Platform-specific implementations
#ifdef _WIN32
    bool execWindows();
#endif

#ifdef __APPLE__
    bool execMacOS();
#endif

#ifdef __linux__
    bool execLinux();
#endif

    // Helpers
    std::vector<TString> standardButtonTexts() const;

    std::vector<Message::StandardButton> standardButtonIds() const;

    TString m_windowTitle;
    TString m_text;
    TString m_informativeText;
    TString m_detailedText;

    Message::Icon m_icon;

    int m_standardButtons;
    std::vector<CustomButton> m_customButtons;

    Message::StandardButton m_defaultButton;
    Message::StandardButton m_escapeButton;

    Message::StandardButton m_clickedStandardButton;
    int m_clickedCustomButton;
    TString m_clickedButtonText;
};

bool MessagePrivate::exec() {
    m_clickedStandardButton = Message::NoButton;
    m_clickedCustomButton = -1;
    m_clickedButtonText.clear();

#ifdef _WIN32
    return execWindows();
#elif __APPLE__
    return execMacOS();
#elif __linux__
    return execLinux();
#else
#error "Unsupported platform"
#endif
}

std::vector<Message::StandardButton> MessagePrivate::standardButtonIds() const {
    std::vector<Message::StandardButton> ids;

    if(m_standardButtons & Message::Ok)              ids.push_back(Message::Ok);
    if(m_standardButtons & Message::Yes)             ids.push_back(Message::Yes);
    if(m_standardButtons & Message::YesToAll)        ids.push_back(Message::YesToAll);
    if(m_standardButtons & Message::No)              ids.push_back(Message::No);
    if(m_standardButtons & Message::NoToAll)         ids.push_back(Message::NoToAll);
    if(m_standardButtons & Message::Save)            ids.push_back(Message::Save);
    if(m_standardButtons & Message::SaveAll)         ids.push_back(Message::SaveAll);
    if(m_standardButtons & Message::Open)            ids.push_back(Message::Open);
    if(m_standardButtons & Message::Abort)           ids.push_back(Message::Abort);
    if(m_standardButtons & Message::Retry)           ids.push_back(Message::Retry);
    if(m_standardButtons & Message::Ignore)          ids.push_back(Message::Ignore);
    if(m_standardButtons & Message::Discard)         ids.push_back(Message::Discard);
    if(m_standardButtons & Message::Cancel)          ids.push_back(Message::Cancel);
    if(m_standardButtons & Message::Close)           ids.push_back(Message::Close);
    if(m_standardButtons & Message::Help)            ids.push_back(Message::Help);
    if(m_standardButtons & Message::Apply)           ids.push_back(Message::Apply);
    if(m_standardButtons & Message::Reset)           ids.push_back(Message::Reset);
    if(m_standardButtons & Message::RestoreDefaults) ids.push_back(Message::RestoreDefaults);

    return ids;
}

std::vector<TString> MessagePrivate::standardButtonTexts() const {
    std::vector<TString> result;

    if(m_standardButtons & Message::Ok)              result.push_back("OK");
    if(m_standardButtons & Message::Yes)             result.push_back("Yes");
    if(m_standardButtons & Message::YesToAll)        result.push_back("Yes to All");
    if(m_standardButtons & Message::No)              result.push_back("No");
    if(m_standardButtons & Message::NoToAll)         result.push_back("No to All");
    if(m_standardButtons & Message::Save)            result.push_back("Save");
    if(m_standardButtons & Message::SaveAll)         result.push_back("Save All");
    if(m_standardButtons & Message::Open)            result.push_back("Open");
    if(m_standardButtons & Message::Abort)           result.push_back("Abort");
    if(m_standardButtons & Message::Retry)           result.push_back("Retry");
    if(m_standardButtons & Message::Ignore)          result.push_back("Ignore");
    if(m_standardButtons & Message::Discard)         result.push_back("Discard");
    if(m_standardButtons & Message::Cancel)          result.push_back("Cancel");
    if(m_standardButtons & Message::Close)           result.push_back("Close");
    if(m_standardButtons & Message::Help)            result.push_back("Help");
    if(m_standardButtons & Message::Apply)           result.push_back("Apply");
    if(m_standardButtons & Message::Reset)           result.push_back("Reset");
    if(m_standardButtons & Message::RestoreDefaults) result.push_back("Restore Defaults");

    return result;
}

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "User32.lib")

bool MessagePrivate::execWindows() {
    // Build message text
    TString message = m_text;
    if(!m_informativeText.isEmpty()) {
        if(!message.isEmpty()) {
            message += "\n\n";
        }
        message += m_informativeText;
    }
    if(!m_detailedText.isEmpty()) {
        if(!message.isEmpty()) {
            message += "\n\n";
        }
        message += m_detailedText;
    }

    // Determine icon
    UINT iconType = MB_ICONINFORMATION;
    switch(m_icon) {
    case Message::NoIcon:      iconType = 0; break;
    case Message::Information: iconType = MB_ICONINFORMATION; break;
    case Message::Warning:     iconType = MB_ICONWARNING; break;
    case Message::Critical:    iconType = MB_ICONERROR; break;
    case Message::Question:    iconType = MB_ICONQUESTION; break;
    }

    // Determine buttons
    UINT buttonType = MB_OK;
    switch(m_standardButtons) {
    case Message::Ok:
        buttonType = MB_OK;
        break;
    case Message::Ok | Message::Cancel:
        buttonType = MB_OKCANCEL;
        break;
    case Message::Yes | Message::No:
        buttonType = MB_YESNO;
        break;
    case Message::Yes | Message::No | Message::Cancel:
        buttonType = MB_YESNOCANCEL;
        break;
    case Message::Abort | Message::Retry | Message::Ignore:
        buttonType = MB_ABORTRETRYIGNORE;
        break;
    case Message::Retry | Message::Cancel:
        buttonType = MB_RETRYCANCEL;
        break;
    case Message::Save | Message::Discard | Message::Cancel:
        buttonType = MB_YESNOCANCEL; // closest match
        break;
    default:
        buttonType = MB_OK;
        break;
    }

    // Determine default button
    UINT defaultType = 0;
    if(m_defaultButton == Message::Yes || m_defaultButton == Message::Ok) {
        defaultType = MB_DEFBUTTON1;
    } else if(m_defaultButton == Message::No) {
        defaultType = MB_DEFBUTTON2;
    } else if(m_defaultButton == Message::Cancel) {
        defaultType = MB_DEFBUTTON3;
    }

    // Show the message box
    int result = MessageBoxA(GetActiveWindow(),
                             message.data(),
                             m_windowTitle.isEmpty() ? "Message" : m_windowTitle.data(),
                             iconType | buttonType | defaultType);

    // Map result back to standard buttons
    switch(result) {
    case IDOK:      m_clickedStandardButton = Message::Ok; break;
    case IDCANCEL:  m_clickedStandardButton = Message::Cancel; break;
    case IDYES:     m_clickedStandardButton = Message::Yes; break;
    case IDNO:      m_clickedStandardButton = Message::No; break;
    case IDABORT:   m_clickedStandardButton = Message::Abort; break;
    case IDRETRY:   m_clickedStandardButton = Message::Retry; break;
    case IDIGNORE:  m_clickedStandardButton = Message::Ignore; break;
    default:        m_clickedStandardButton = Message::NoButton; break;
    }

    // Store the clicked button text
    switch(m_clickedStandardButton) {
    case Message::Ok:     m_clickedButtonText = "OK"; break;
    case Message::Cancel: m_clickedButtonText = "Cancel"; break;
    case Message::Yes:    m_clickedButtonText = "Yes"; break;
    case Message::No:     m_clickedButtonText = "No"; break;
    case Message::Abort:  m_clickedButtonText = "Abort"; break;
    case Message::Retry:  m_clickedButtonText = "Retry"; break;
    case Message::Ignore: m_clickedButtonText = "Ignore"; break;
    default: break;
    }

    return true;
}
#endif // _WIN32

#ifdef __linux__
#include <gtk/gtk.h>

bool MessagePrivate::execLinux() {
    static bool gtkInitialized = false;
    if(!gtkInitialized) {
        gtk_init(nullptr, nullptr);
        gtkInitialized = true;
    }

    // Determine message type
    GtkMessageType msgType = GTK_MESSAGE_INFO;
    switch(m_icon) {
    case Message::NoIcon:      msgType = GTK_MESSAGE_OTHER; break;
    case Message::Information: msgType = GTK_MESSAGE_INFO; break;
    case Message::Warning:     msgType = GTK_MESSAGE_WARNING; break;
    case Message::Critical:    msgType = GTK_MESSAGE_ERROR; break;
    case Message::Question:    msgType = GTK_MESSAGE_QUESTION; break;
    }

    // Build message text
    TString message = m_text;
    if(!m_informativeText.isEmpty()) {
        if(!message.isEmpty()) {
            message += "\n\n";
        }
        message += m_informativeText;
    }
    if(!m_detailedText.isEmpty()) {
        if(!message.isEmpty()) {
            message += "\n\n";
        }
        message += m_detailedText;
    }

    // Determine buttons
    GtkButtonsType buttonsType = GTK_BUTTONS_OK;
    switch(m_standardButtons) {
    case Message::Ok:
        buttonsType = GTK_BUTTONS_OK;
        break;
    case Message::Ok | Message::Cancel:
        buttonsType = GTK_BUTTONS_OK_CANCEL;
        break;
    case Message::Yes | Message::No:
        buttonsType = GTK_BUTTONS_YES_NO;
        break;
    case Message::Close:
        buttonsType = GTK_BUTTONS_CLOSE;
        break;
    case Message::Cancel:
        buttonsType = GTK_BUTTONS_CANCEL;
        break;
    default:
        buttonsType = GTK_BUTTONS_NONE;
        break;
    }

    // Create dialog
    GtkWidget *dialog = gtk_message_dialog_new(
        nullptr,
        GTK_DIALOG_MODAL,
        msgType,
        buttonsType,
        "%s",
        message.data()
        );

    if(!m_windowTitle.isEmpty()) {
        gtk_window_set_title(GTK_WINDOW(dialog), m_windowTitle.data());
    }

    // For custom button combinations, we need to add buttons manually
    if(buttonsType == GTK_BUTTONS_NONE) {
        std::vector<TString> texts = standardButtonTexts();
        std::vector<Message::StandardButton> ids = standardButtonIds();

        for(size_t i = 0; i < texts.size(); i++) {
            gtk_dialog_add_button(GTK_DIALOG(dialog), texts[i].data(),
                                  static_cast<int>(i) + 1);
        }
    }

    // Add custom buttons
    for(size_t i = 0; i < m_customButtons.size(); i++) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), m_customButtons[i].text.data(),
                              1000 + static_cast<int>(i));
    }

    // Set default button
    if(m_defaultButton != Message::NoButton) {
        std::vector<Message::StandardButton> ids = standardButtonIds();

        for(size_t i = 0; i < ids.size(); i++) {
            if(ids[i] == m_defaultButton) {
                gtk_dialog_set_default_response(GTK_DIALOG(dialog),
                                                static_cast<int>(i) + 1);
                break;
            }
        }
    }

    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    // Map response back
    if(response >= 1000) {
        int customIndex = response - 1000;
        if(customIndex >= 0 && customIndex < static_cast<int>(m_customButtons.size())) {
            m_clickedCustomButton = customIndex;
            m_clickedButtonText = m_customButtons[customIndex].text;
        }
    } else if(response > 0) {
        std::vector<Message::StandardButton> ids = standardButtonIds();

        int idx = response - 1;
        if(idx >= 0 && idx < static_cast<int>(ids.size())) {
            m_clickedStandardButton = ids[idx];
        }

        // For GTK_BUILTIN buttons, map directly
        switch(response) {
        case GTK_RESPONSE_OK:     m_clickedStandardButton = Message::Ok; break;
        case GTK_RESPONSE_CANCEL: m_clickedStandardButton = Message::Cancel; break;
        case GTK_RESPONSE_YES:    m_clickedStandardButton = Message::Yes; break;
        case GTK_RESPONSE_NO:     m_clickedStandardButton = Message::No; break;
        case GTK_RESPONSE_CLOSE:  m_clickedStandardButton = Message::Close; break;
        default: break;
        }

        // Set button text
        switch(m_clickedStandardButton) {
        case Message::Ok:     m_clickedButtonText = "OK"; break;
        case Message::Cancel: m_clickedButtonText = "Cancel"; break;
        case Message::Yes:    m_clickedButtonText = "Yes"; break;
        case Message::No:     m_clickedButtonText = "No"; break;
        case Message::Close:  m_clickedButtonText = "Close"; break;
        default: break;
        }
    }

    gtk_widget_destroy(dialog);
    return true;
}
#endif // __linux__

/*!
    \class Message
    \brief Platform-independent modal message dialog.
    \since Next 1.0
    \inmodule OS

    `Message` provides a portable interface for showing modal message
    dialogs with a title, primary text, informative text, detailed text,
    and a configurable set of buttons and an icon. The dialog is
    implemented on top of the native platform APIs:

    \list
        \li Windows: `MessageBoxA` from `User32.lib`.
        \li Linux:   GTK+ `GtkMessageDialog`.
        \li macOS:   not implemented in this version.
    \endlist

    Typical usage:

    \code
    Message msg;
    msg.setWindowTitle("Confirm");
    msg.setText("Save changes?");
    msg.setIcon(Message::Question);
    msg.setStandardButtons(Message::Yes | Message::No);
    msg.setDefaultButton(Message::Yes);

    if(msg.exec() == Message::Yes) {
        // user confirmed
    }
    \endcode

    \sa StandardButton, Icon, exec()
*/

/*!
    \enum Message::Icon

    Predefined icons that can be shown in the message dialog.

    \value NoIcon
           No icon is shown.
    \value Information
           An informational icon (i) is shown.
    \value Warning
           A warning icon (!) is shown.
    \value Critical
           A critical error icon is shown.
    \value Question
           A question mark icon is shown.
*/

/*!
    \enum Message::StandardButton

    Flags describing the set of standard buttons displayed in the
    message dialog. Values can be combined with the bitwise OR
    operator.

    \value NoButton        No button.
    \value Ok              An "OK" button.
    \value Save            A "Save" button.
    \value SaveAll         A "Save All" button.
    \value Open            An "Open" button.
    \value Yes             A "Yes" button.
    \value YesToAll        A "Yes to All" button.
    \value No              A "No" button.
    \value NoToAll         A "No to All" button.
    \value Abort           An "Abort" button.
    \value Retry           A "Retry" button.
    \value Ignore          An "Ignore" button.
    \value Close           A "Close" button.
    \value Cancel          A "Cancel" button.
    \value Discard         A "Discard" button.
    \value Help            A "Help" button.
    \value Apply           An "Apply" button.
    \value Reset           A "Reset" button.
    \value RestoreDefaults A "Restore Defaults" button.

    \sa setStandardButtons()
*/

/*!
    \enum Message::ButtonRole

    Roles that describe the semantic meaning of a custom button.
    The role is currently informational only; it is not used by the
    platform backends to change button behavior.

    \value InvalidRole     An invalid role.
    \value AcceptRole      The button accepts the operation.
    \value RejectRole      The button rejects the operation.
    \value DestructiveRole The button triggers a destructive action.
    \value ActionRole      The button performs a non-committing action.
    \value HelpRole        The button opens help.
    \value YesRole         The button answers "Yes".
    \value NoRole          The button answers "No".
    \value ResetRole       The button resets the dialog to defaults.
    \value ApplyRole       The button applies the current settings.

    \sa addButton()
*/

/*!
    Constructs an empty `Message` object with default settings:
    no text, no icon, and the \l{StandardButton}{Ok} standard button.
*/
Message::Message() :
    m_ptr(new MessagePrivate) {
}

Message::~Message() {
    delete m_ptr;
}

/*!
    Move-constructs a `Message` from \a other.
*/
Message::Message(Message&&) noexcept = default;

/*!
    Move-assigns \a other to this `Message`.
*/
Message& Message::operator=(Message&&) noexcept = default;

/*!
    Sets the dialog's window title to \a title.

    If the title is empty, the platform default title is used
    (for example, "Message" on Windows).

    \sa setText()
*/
void Message::setWindowTitle(const TString &title) {
    m_ptr->m_windowTitle = title;
}

/*!
    Returns the primary text shown in the dialog.

    \sa setText()
*/
TString Message::text() const {
    return m_ptr->m_text;
}

/*!
    Sets the primary text of the dialog to \a text.

    This is the main message shown to the user.

    \sa text(), setInformativeText(), setDetailedText()
*/
void Message::setText(const TString &text) {
    m_ptr->m_text = text;
}

/*!
    Sets the informative text of the dialog to \a text.

    The informative text is displayed below the primary text and is
    intended to provide additional context. Platform backends concatenate
    it with the primary text separated by a blank line.

    \sa setText(), setDetailedText()
*/
void Message::setInformativeText(const TString &text) {
    m_ptr->m_informativeText = text;
}

/*!
    Sets the detailed text of the dialog to \a text.

    The detailed text is intended for technical details such as error
    dumps or file paths. Platform backends concatenate it after the
    informative text separated by a blank line.

    \sa setText(), setInformativeText()
*/
void Message::setDetailedText(const TString &text) {
    m_ptr->m_detailedText = text;
}

/*!
    Sets the icon shown in the dialog to \a icon.

    \sa Icon
*/
void Message::setIcon(Icon icon) {
    m_ptr->m_icon = icon;
}

/*!
    Sets the set of standard buttons shown in the dialog to \a buttons.

    The \a buttons parameter is a bitwise OR of the \l{StandardButton}
    values. For example, `Message::Yes | Message::No | Message::Cancel`.

    Not every combination is natively supported by all platforms; the
    backends fall back to the closest available set if necessary.

    \sa StandardButton, addButton()
*/
void Message::setStandardButtons(int buttons) {
    m_ptr->m_standardButtons = buttons;
}

/*!
    Adds a custom button with the given \a text and \a role to the dialog.

    Custom buttons appear after all standard buttons. The \a role
    parameter is currently informational only and does not affect
    the button's behavior on any platform.

    \sa ButtonRole, setStandardButtons()
*/
void Message::addButton(const TString &text, ButtonRole role) {
    m_ptr->m_customButtons.push_back({text, role});
}

/*!

    Sets the default button to \a button.

    The default button is focused when the dialog opens and is activated
    when the user presses Enter (unless overridden by the platform).

    \sa StandardButton, setEscapeButton()
*/
void Message::setDefaultButton(StandardButton button) {
    m_ptr->m_defaultButton = button;
}

/*!
    \fn void Message::setEscapeButton(StandardButton button)

    Sets the button that is activated when the user presses Escape to
    \a button.

    \note Currently this setting is stored but not forwarded to the
    platform backend on any supported platform.

    \sa StandardButton, setDefaultButton()
*/
void Message::setEscapeButton(StandardButton button) {
    m_ptr->m_escapeButton = button;
}

/*!
    Shows the dialog modally and blocks until the user closes it.

    Returns the \l{StandardButton} value of the button that was clicked,
    or \l{StandardButton}{NoButton} if the dialog was closed without a
    recognized button press.

    For custom buttons added via addButton(), use
    customButtonClicked() to obtain the index of the clicked button.

    \sa standardButtonClicked(), customButtonClicked(), clickedButtonText()
*/
int Message::exec() {
    m_ptr->exec();
    return m_ptr->m_clickedStandardButton;
}

/*!
    Returns the standard button that was clicked in the last exec() call,
    or \l{StandardButton}{NoButton} if a custom button was clicked or
    no button was pressed.

    \sa exec(), customButtonClicked()
*/
Message::StandardButton Message::standardButtonClicked() const {
    return m_ptr->m_clickedStandardButton;
}

/*!
    Returns the zero-based index of the custom button that was clicked
    in the last exec() call, or -1 if a standard button was clicked or
    no button was pressed.

    Custom buttons are indexed in the order they were added via
    addButton().

    \sa exec(), addButton(), standardButtonClicked()
*/
int Message::customButtonClicked() const {
    return m_ptr->m_clickedCustomButton;
}

/*!
    Returns the text of the button that was clicked in the last exec()
    call. For standard buttons, this is the localized (or, in this
    implementation, hard-coded English) label. For custom buttons, it
    is the text passed to addButton().

    If no button was clicked, an empty string is returned.

    \sa exec(), standardButtonClicked(), customButtonClicked()
*/
TString Message::clickedButtonText() const {
    return m_ptr->m_clickedButtonText;
}
