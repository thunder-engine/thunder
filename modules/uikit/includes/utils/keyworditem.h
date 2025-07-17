#ifndef KEYWORDITEM_H
#define KEYWORDITEM_H

#include <astring.h>

using namespace next;

class KeywordItem {
public:
    KeywordItem(const String &name);

    void setData(const String &data) {
        m_data = data;
    };

    String &data() {
        return m_data;
    }

    String name() {
        return m_name;
    }

private:
    String m_name;
    String m_data;

};

#endif /* KEYWORDITEM_H */
