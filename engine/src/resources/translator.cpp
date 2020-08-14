#include "resources/translator.h"

#define DATA  "Data"

class TranslatorPrivate {
public:
    typedef unordered_map<string, string> TranslationTable;
    TranslationTable m_Table;
};

/*!
    \class Translator
    \brief Translator resource provides a translation table.
    \inmodule Resource
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
    \internal
*/
void Translator::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        for(auto pair : (*it).second.toMap()) {
            p_ptr->m_Table[pair.first] = pair.second.toString();
        }
    }

    setState(Ready);
}
