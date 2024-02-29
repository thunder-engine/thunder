#include "utils/selector.h"

#include "utils/stringutil.h"

Selector::Selector() {
}

Selector::~Selector() {
}

inline const std::string &Selector::ruleData() const {
    return m_ruleData;
}

void Selector::setRuleData(const std::string &data) {
    m_ruleData = data;
    m_ruleDataMap.clear();
}

std::map<std::string, std::string> &Selector::ruleDataMap() {
    if(m_ruleDataMap.empty()) {
        StringUtil::trim(m_ruleData);
        StringUtil::deletechar(m_ruleData, '\n');
        auto keyValues = StringUtil::splitButSkipBrackets(m_ruleData, ';');

        for(const auto &pair : keyValues) {
            auto keyAndValue = StringUtil::splitButSkipBrackets(pair, ':');

            if(keyAndValue.size() < 2) {
                continue;
            }
            m_ruleDataMap[StringUtil::trim(keyAndValue[0])] = StringUtil::trim(keyAndValue[1]);
        }
    }

    return m_ruleDataMap;
}

Selector::SelectorType Selector::type() {
    return m_selectorType;
}

const std::string &Selector::hostCSSFilePath() const {
    return m_hostCSSFilePath;
}

void Selector::setHostCSSFilePath(const std::string &path) {
    m_hostCSSFilePath = path;
}
