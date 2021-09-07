#include "resources/text.h"

#include <variant.h>

#define DATA  "Data"

#include <log.h>

class TextPrivate {
public:
    ByteArray m_Data;
};

/*!
    \class Text
    \brief Text file resource.
    \inmodule Resources
*/

Text::Text() :
        p_ptr(new TextPrivate) {

}

Text::~Text() {
    delete p_ptr;
}
/*!
    \internal
*/
void Text::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        p_ptr->m_Data = (*it).second.toByteArray();
    }

    setState(Ready);
}
/*!
    \internal
*/
VariantMap Text::saveUserData () const {
    VariantMap result;
    result["Data"] = data();
    return result;
}
/*!
    Returns text content as a raw byte array.
*/
uint8_t *Text::data() const {
    return reinterpret_cast<uint8_t *>(p_ptr->m_Data.data());
}
/*!
    Returns size of the text resource.
*/
uint32_t Text::size() const {
    return p_ptr->m_Data.size();
}
/*!
    Sets the new \a size of the text resource.
*/
void Text::setSize(uint32_t size) {
    p_ptr->m_Data.resize(size);
}
/*!
    Returns text content as a tring.
*/
string Text::text() {
    return string(reinterpret_cast<char *>(p_ptr->m_Data.data()), size());
}
