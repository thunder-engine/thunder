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
    AttributeSelector(const std::string &key, const std::string &value, AttributeFilterRule rule);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    std::string m_key;
    std::string m_value;

    AttributeFilterRule m_filterRule;

};

#endif /* ATTRIBUTESELECTOR_H */
