#ifndef TYPESELECTOR_H
#define TYPESELECTOR_H

#include "selector.h"

class TypeSelector: public Selector {
public:
    TypeSelector(const TString &typeName);

    inline TString tagName();

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    TString m_typeName;

};

#endif /* TYPESELECTOR_H */
