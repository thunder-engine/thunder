#include "resources/translator.h"

#include <metaproperty.h>

#define DATA  "Data"

class TranslatorPrivate {
public:
    typedef unordered_map<string, string> TranslationTable;
    TranslationTable m_Table;
};

/*!
    \class Translator
    \brief Translator resource provides a translation table.
    \inmodule Resources
*/

Translator::Translator() :
        p_ptr(new TranslatorPrivate()) {

}

Translator::~Translator() {
    delete p_ptr;
}
/*!
    Returns the translated \a source string.
*/
string Translator::translate(const string &source) const {
    auto it = p_ptr->m_Table.find(source);
    if(it != p_ptr->m_Table.end()) {
        return it->second;
    }
    return source;
}
/*!
    Sets new \a translation for the \a source string.
*/
void Translator::setPair(const string &source, const string &translation) {
    p_ptr->m_Table[source] = translation;
}
/*!
    \internal
*/
void Translator::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        for(auto &pair : (*it).second.toMap()) {
            p_ptr->m_Table[pair.first] = pair.second.toString();
        }
    }

    setState(Ready);
}
/*!
    \internal
*/
VariantMap Translator::saveUserData() const {
    VariantMap result;
    VariantMap data;

    for(auto &it : p_ptr->m_Table) {
        data[it.first] = it.second;
    }

    result[DATA] = data;
    return result;
}

