#include "utils/attributeselector.h"

#include "components/widget.h"

AttributeSelector::AttributeSelector(const TString &key, const TString &value, AttributeFilterRule rule) {
    m_key = key;
    m_value = value;
    m_filterRule = rule;
    m_selectorType = Selector::AttributeSelector;
}

bool AttributeSelector::isMeet(Widget *widget) {
    TString key = m_key;

    if(key.isEmpty()) {
        return false;
    }

    const MetaObject *meta = widget->metaObject();

    int32_t index = meta->indexOfProperty(key.data());
    if(index < 0) {
        return false;
    }

    TString propertyValue = meta->property(index).read(widget).toString();

    bool ret = false;
    switch(m_filterRule) {
        case AttributeSelector::Equal: return (propertyValue == m_value);
        case AttributeSelector::DashMatch: {
            if(propertyValue.indexOf('-') == -1) {
                break;
            }
            auto attrs = propertyValue.split('-');
            return attrs.front() == m_value;
        }
        case AttributeSelector::Prefix: return (propertyValue.indexOf(m_value) == 0);
        case AttributeSelector::Suffix: return (propertyValue.lastIndexOf(m_value) + m_value.length() == propertyValue.length());
        case AttributeSelector::Include: {
            if(propertyValue.indexOf(' ') == -1) {
                break;
            }

            for(const auto &attr : propertyValue.split(' ')) {
                if(attr == m_value) {
                    return true;
                }
            }
            break;
        }
        case AttributeSelector::Substring: return (propertyValue.indexOf(m_value) != -1);
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
