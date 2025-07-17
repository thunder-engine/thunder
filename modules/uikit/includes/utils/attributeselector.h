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
    AttributeSelector(const TString &key, const TString &value, AttributeFilterRule rule);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    TString m_key;
    TString m_value;

    AttributeFilterRule m_filterRule;

};

#endif /* ATTRIBUTESELECTOR_H */
