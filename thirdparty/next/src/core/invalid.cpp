#include "invalid.h"

Invalid::Invalid() {

}

void Invalid::loadData(const VariantList &data) {
    m_Data = data;
}

VariantList Invalid::saveData() const {
    return m_Data;
}

string Invalid::typeName() const {
    return m_Data.begin()->toString();
}
