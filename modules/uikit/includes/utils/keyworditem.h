#ifndef KEYWORDITEM_H
#define KEYWORDITEM_H

#include <string>

class KeywordItem {
public:
    KeywordItem(const std::string &name);

    void setData(const std::string &data) {
        m_data = data;
    };

    std::string &data() {
        return m_data;
    }

    std::string name() {
        return m_name;
    }

private:
    std::string m_name;
    std::string m_data;

};

#endif /* KEYWORDITEM_H */
