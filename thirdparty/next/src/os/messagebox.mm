#include "os/message.h"

#ifdef __APPLE__

#include <string>
#include <vector>

#import <Cocoa/Cocoa.h>

class MessagePrivate {
public:
    MessagePrivate();
    ~MessagePrivate();

    bool execMacOS();

private:
    friend class Message;

    struct CustomButton {
        TString text;
        Message::ButtonRole role;
    };

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

bool MessagePrivate::execMacOS() {
    @autoreleasepool {
        NSAlert *alert = [[NSAlert alloc] init];

        // Message text
        if(!m_text.isEmpty()) {
            alert.messageText = [NSString stringWithUTF8String:m_text.data()];
        }

        // Informative text (with detailed text appended)
        TString informative = m_informativeText;
        if(!m_detailedText.isEmpty()) {
            if(!informative.isEmpty()) {
                informative += "\n\n";
            }
            informative += m_detailedText;
        }
        if(!informative.isEmpty()) {
            alert.informativeText = [NSString stringWithUTF8String:informative.data()];
        }

        // Alert style (icon)
        switch(m_icon) {
        case Message::Information:
            alert.alertStyle = NSAlertStyleInformational;
            break;
        case Message::Warning:
            alert.alertStyle = NSAlertStyleWarning;
            break;
        case Message::Critical:
            alert.alertStyle = NSAlertStyleCritical;
            break;
        case Message::Question:
            alert.alertStyle = NSAlertStyleInformational;
            break;
        default:
            break;
        }

        // Collect button texts in display order
        std::vector<TString> buttonTexts;
        std::vector<Message::StandardButton> buttonIds;

        if(m_standardButtons & Message::Ok) {
            buttonTexts.push_back("OK");
            buttonIds.push_back(Message::Ok);
        }
        if(m_standardButtons & Message::Yes) {
            buttonTexts.push_back("Yes");
            buttonIds.push_back(Message::Yes);
        }
        if(m_standardButtons & Message::YesToAll) {
            buttonTexts.push_back("Yes to All");
            buttonIds.push_back(Message::YesToAll);
        }
        if(m_standardButtons & Message::No) {
            buttonTexts.push_back("No");
            buttonIds.push_back(Message::No);
        }
        if(m_standardButtons & Message::NoToAll) {
            buttonTexts.push_back("No to All");
            buttonIds.push_back(Message::NoToAll);
        }
        if(m_standardButtons & Message::Save) {
            buttonTexts.push_back("Save");
            buttonIds.push_back(Message::Save);
        }
        if(m_standardButtons & Message::SaveAll) {
            buttonTexts.push_back("Save All");
            buttonIds.push_back(Message::SaveAll);
        }
        if(m_standardButtons & Message::Open) {
            buttonTexts.push_back("Open");
            buttonIds.push_back(Message::Open);
        }
        if(m_standardButtons & Message::Abort) {
            buttonTexts.push_back("Abort");
            buttonIds.push_back(Message::Abort);
        }
        if(m_standardButtons & Message::Retry) {
            buttonTexts.push_back("Retry");
            buttonIds.push_back(Message::Retry);
        }
        if(m_standardButtons & Message::Ignore) {
            buttonTexts.push_back("Ignore");
            buttonIds.push_back(Message::Ignore);
        }
        if(m_standardButtons & Message::Discard) {
            buttonTexts.push_back("Discard");
            buttonIds.push_back(Message::Discard);
        }
        if(m_standardButtons & Message::Cancel) {
            buttonTexts.push_back("Cancel");
            buttonIds.push_back(Message::Cancel);
        }
        if(m_standardButtons & Message::Close) {
            buttonTexts.push_back("Close");
            buttonIds.push_back(Message::Close);
        }
        if(m_standardButtons & Message::Help) {
            buttonTexts.push_back("Help");
            buttonIds.push_back(Message::Help);
        }
        if(m_standardButtons & Message::Apply) {
            buttonTexts.push_back("Apply");
            buttonIds.push_back(Message::Apply);
        }
        if(m_standardButtons & Message::Reset) {
            buttonTexts.push_back("Reset");
            buttonIds.push_back(Message::Reset);
        }
        if(m_standardButtons & Message::RestoreDefaults) {
            buttonTexts.push_back("Restore Defaults");
            buttonIds.push_back(Message::RestoreDefaults);
        }

        // Add standard buttons
        for(const auto &text : buttonTexts) {
            [alert addButtonWithTitle:[NSString stringWithUTF8String:text.data()]];
        }

        // Add custom buttons
        for(const auto &btn : m_customButtons) {
            [alert addButtonWithTitle:[NSString stringWithUTF8String:btn.text.data()]];
        }

        // If no buttons at all, add OK
        if(buttonTexts.empty() && m_customButtons.empty()) {
            [alert addButtonWithTitle:@"OK"];
            buttonTexts.push_back("OK");
            buttonIds.push_back(Message::Ok);
        }

        // Set window title on the alert's window
        if(!m_windowTitle.isEmpty()) {
            alert.window.title = [NSString stringWithUTF8String:m_windowTitle.data()];
        }

        // Run modal
        NSModalResponse response = [alert runModal];

        // response is 1-based index of the clicked button
        long index = static_cast<long>(response) - 1;

        if(index >= 0 && index < static_cast<long>(buttonIds.size())) {
            m_clickedStandardButton = buttonIds[index];
            m_clickedButtonText = buttonTexts[index];
        } else if(index >= static_cast<long>(buttonIds.size())) {
            long customIndex = index - static_cast<long>(buttonIds.size());
            if(customIndex >= 0 && customIndex < static_cast<long>(m_customButtons.size())) {
                m_clickedCustomButton = static_cast<int>(customIndex);
                m_clickedButtonText = m_customButtons[customIndex].text;
            }
        }

        [alert release];
    }

    return true;
}

#endif // __APPLE__
