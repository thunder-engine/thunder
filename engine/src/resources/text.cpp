#include "resources/text.h"

#include <variant.h>

namespace {
    const char *gData = "Data";
}

/*!
    \class Text
    \brief Text file resource.
    \inmodule Resources
*/

Text::Text() {

}

Text::~Text() {

}
/*!
    \internal
*/
void Text::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        m_data = (*it).second.toByteArray();
    }
}
/*!
    \internal
*/
VariantMap Text::saveUserData () const {
    VariantMap result;
    result[gData] = m_data;
    return result;
}
/*!
    Returns text content as a raw byte array.
*/
uint8_t *Text::data() {
    return reinterpret_cast<uint8_t *>(m_data.data());
}
/*!
    Returns size of the text resource.
*/
uint32_t Text::size() const {
    return m_data.size();
}
/*!
    Sets the new \a size of the text resource.
*/
void Text::setSize(uint32_t size) {
    m_data.resize(size);
}
/*!
    Returns text content as string.
*/
std::string Text::text() {
    return std::string(reinterpret_cast<char *>(m_data.data()), size());
}
