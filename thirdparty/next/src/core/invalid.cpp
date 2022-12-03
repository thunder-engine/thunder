#include "invalid.h"

Invalid::Invalid() {

}

void Invalid::loadData(const VariantList &data) {
    m_data = data;
}

VariantList Invalid::saveData() const {
    return m_data;
}

string Invalid::typeName() const {
    return m_data.begin()->toString();
}
