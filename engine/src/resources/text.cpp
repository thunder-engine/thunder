#include "resources/text.h"

#include <variant.h>

#define DATA  "Data"

Text::Text() {

}

Text::~Text() {
}

void Text::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        m_Data  = (*it).second.toByteArray();
    }
}

const int8_t *Text::data() const {
    return &m_Data[0];
}

uint32_t Text::size() const {
    return m_Data.size();
}

string Text::text() const {
    return string((const char *)data(), size());
}
