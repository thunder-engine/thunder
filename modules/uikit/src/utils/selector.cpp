#include "utils/selector.h"

Selector::Selector() {
}

Selector::~Selector() {
}

inline const TString &Selector::ruleData() const {
    return m_ruleData;
}

void Selector::setRuleData(const TString &data) {
    m_ruleData = data;
    m_ruleDataMap.clear();
}

std::map<TString, TString> &Selector::ruleDataMap() {
    if(m_ruleDataMap.empty()) {
        m_ruleData = m_ruleData.trimmed();

        auto keyValues = splitButSkipBrackets(m_ruleData.toStdString(), ';');

        for(const auto &pair : keyValues) {
            auto keyAndValue = splitButSkipBrackets(pair.toStdString(), ':');

            if(keyAndValue.size() < 2) {
                continue;
            }
            m_ruleDataMap[keyAndValue[0].trimmed()] = keyAndValue[1].trimmed();
        }
    }

    return m_ruleDataMap;
}

Selector::SelectorType Selector::type() {
    return m_selectorType;
}

const TString &Selector::hostCSSFilePath() const {
    return m_hostCSSFilePath;
}

void Selector::setHostCSSFilePath(const TString &path) {
    m_hostCSSFilePath = path;
}

std::vector<TString> Selector::splitButSkipBrackets(const std::string &s, char separator) {
    std::vector<TString> container;
    size_t length = s.length();
    size_t i = 0, start = 0;
    bool insideBracket = false;

    for (; i < length; i++) {
        if (s[i] == '(' || s[i] == '[' || s[i] == '{') {
            insideBracket = true;
        }
        else if (s[i] == ')' || s[i] == ']' || s[i] == '}') {
            insideBracket = false;
        }
        else if (s[i] == separator && !insideBracket) {
            container.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }

    container.push_back(s.substr(start));
    return container;
}
