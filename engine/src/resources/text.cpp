#include "resources/text.h"

#include <variant.h>

#define DATA  "Data"

#include <log.h>

class TextPrivate {
public:
    ByteArray m_Data;
};

Text::Text() :
        p_ptr(new TextPrivate) {

}

Text::~Text() {
    delete p_ptr;
}

void Text::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        p_ptr->m_Data = (*it).second.toByteArray();
    }
}

char *Text::data() const {
    return reinterpret_cast<char *>(&p_ptr->m_Data[0]);
}

uint32_t Text::size() const {
    return p_ptr->m_Data.size();
}

void Text::setSize(uint32_t size) {
    p_ptr->m_Data.resize(size);
}

string Text::text() {
    return string(data(), size());
}
