#ifndef ATTRIBUTESELECTOR_H
#define ATTRIBUTESELECTOR_H

#include "selector.h"

class AttributeSelector: public Selector {
public:
    enum AttributeFilterRule {
        Prefix,
        Suffix,
        Include,
        Equal,
        Substring,
        DashMatch,
        NoRule
    };

public:
    AttributeSelector(const String &key, const String &value, AttributeFilterRule rule);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    String m_key;
    String m_value;

    AttributeFilterRule m_filterRule;

};

#endif /* ATTRIBUTESELECTOR_H */
