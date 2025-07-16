#include "resources/uidocument.h"

namespace {
    const char *gData = "Data";
}

/*!
    \class UiDocument
    \brief UiDocument class, which appears to handle the storage and management of UI data.
    \inmodule Gui
*/

UiDocument::UiDocument() {

}

/*!
    Returns content as a string.
*/
String UiDocument::data() const {
    return m_data;
}
/*!
    Sets a new content \a data.
*/
void UiDocument::setData(const String &data) {
    m_data = data;
}
/*!
    \internal
*/
void UiDocument::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        m_data = (*it).second.toString();
    }
}
/*!
    \internal
*/
VariantMap UiDocument::saveUserData() const {
    VariantMap result;
    result[gData] = m_data;
    return result;
}
