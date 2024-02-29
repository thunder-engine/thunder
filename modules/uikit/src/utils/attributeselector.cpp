#include "utils/attributeselector.h"

#include "utils/stringutil.h"

#include "components/widget.h"

AttributeSelector::AttributeSelector(const std::string &key, const std::string &value, AttributeFilterRule rule) {
    m_key = key;
    m_value = value;
    m_filterRule = rule;
    m_selectorType = Selector::AttributeSelector;
}

bool AttributeSelector::isMeet(Widget *widget) {
    std::string key = m_key;

    if(key.empty()) {
        return false;
    }

    const MetaObject *meta = widget->metaObject();

    int32_t index = meta->indexOfProperty(key.c_str());
    if(index < 0) {
        return false;
    }

    std::string propertyValue = meta->property(index).read(widget).toString();

    bool ret = false;
    switch(m_filterRule) {
        case AttributeSelector::Equal: return (propertyValue == m_value);
        case AttributeSelector::DashMatch: {
            if(propertyValue.find("-") == std::string::npos) {
                break;
            }
            auto attrs = StringUtil::split(propertyValue, '-');
            return *attrs.begin() == m_value;
        }
        case AttributeSelector::Prefix: return (propertyValue.find(m_value, 0) == 0);
        case AttributeSelector::Suffix: return (propertyValue.rfind(m_value) + m_value.length() == propertyValue.length());
        case AttributeSelector::Include: {
            if(propertyValue.find(" ") == std::string::npos) {
                break;
            }

            auto attrs = StringUtil::split(propertyValue, ' ');
            for(const auto &attr : attrs) {
                if(attr == m_value) {
                    return true;
                }
            }
            break;
        }
        case AttributeSelector::Substring: return (propertyValue.find(m_value, 0) != std::string::npos);
        case AttributeSelector::NoRule: return true;
        default: return false;
    }

    return ret;
}

bool AttributeSelector::isBaseSelector() const {
    return true;
}

int AttributeSelector::weight() {
    return 10;
}
