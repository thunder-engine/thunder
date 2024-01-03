#include "resources/translator.h"

#include <metaproperty.h>

namespace  {
    const char *gData = "Data";
}

/*!
    \class Translator
    \brief Translator resource provides a translation table.
    \inmodule Resources
*/

Translator::Translator() {

}

Translator::~Translator() {

}
/*!
    Returns the translated \a source string.
*/
string Translator::translate(const string &source) const {
    auto it = m_table.find(source);
    if(it != m_table.end()) {
        return it->second;
    }
    return source;
}
/*!
    Sets new \a translation for the \a source string.
*/
void Translator::setPair(const string &source, const string &translation) {
    m_table[source] = translation;
}
/*!
    \internal
*/
void Translator::loadUserData(const VariantMap &data) {
    auto it = data.find(gData);
    if(it != data.end()) {
        for(auto &pair : (*it).second.toMap()) {
            m_table[pair.first] = pair.second.toString();
        }
    }
}
/*!
    \internal
*/
VariantMap Translator::saveUserData() const {
    VariantMap result;
    VariantMap data;

    for(auto &it : m_table) {
        data[it.first] = it.second;
    }

    result[gData] = data;
    return result;
}

