#include "resources/angelscript.h"

#define DATA    "Data"

AngelScript::~AngelScript() {

}

void AngelScript::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        m_array = (*it).second.toByteArray();
    }
    setState(Ready);
}

VariantMap AngelScript::saveUserData() const {
    VariantMap result;

    result[DATA] = m_array;

    return result;
}
