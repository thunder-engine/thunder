#ifndef KEYWORDITEM_H
#define KEYWORDITEM_H

#include <astring.h>

class KeywordItem {
public:
    KeywordItem(const TString &name);

    void setData(const TString &data) {
        m_data = data;
    };

    TString &data() {
        return m_data;
    }

    TString name() {
        return m_name;
    }

private:
    TString m_name;
    TString m_data;

};

#endif /* KEYWORDITEM_H */
