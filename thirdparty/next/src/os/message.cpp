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
        std::vector<MessageBox::StandardButton> ids = standardButtonIds();

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
        std::vector<MessageBox::StandardButton> ids = standardButtonIds();

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
        std::vector<MessageBox::StandardButton> ids = standardButtonIds();

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

Message::Message() :
    m_ptr(new MessagePrivate) {
}

Message::~Message() {
    delete m_ptr;
}

Message::Message(Message&&) noexcept = default;
Message& Message::operator=(Message&&) noexcept = default;

void Message::setWindowTitle(const TString &title) {
    m_ptr->m_windowTitle = title;
}

TString Message::text() const {
    return m_ptr->m_text;
}

void Message::setText(const TString &text) {
    m_ptr->m_text = text;
}

void Message::setInformativeText(const TString &text) {
    m_ptr->m_informativeText = text;
}

void Message::setDetailedText(const TString &text) {
    m_ptr->m_detailedText = text;
}

void Message::setIcon(Icon icon) {
    m_ptr->m_icon = icon;
}

void Message::setStandardButtons(int buttons) {
    m_ptr->m_standardButtons = buttons;
}

void Message::addButton(const TString &text, ButtonRole role) {
    m_ptr->m_customButtons.push_back({text, role});
}

void Message::setDefaultButton(StandardButton button) {
    m_ptr->m_defaultButton = button;
}

void Message::setEscapeButton(StandardButton button) {
    m_ptr->m_escapeButton = button;
}

int Message::exec() {
    m_ptr->exec();
    return m_ptr->m_clickedStandardButton;
}

Message::StandardButton Message::standardButtonClicked() const {
    return m_ptr->m_clickedStandardButton;
}

int Message::customButtonClicked() const {
    return m_ptr->m_clickedCustomButton;
}

TString Message::clickedButtonText() const {
    return m_ptr->m_clickedButtonText;
}
