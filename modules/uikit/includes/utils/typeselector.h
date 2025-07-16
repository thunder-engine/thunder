#ifndef TYPESELECTOR_H
#define TYPESELECTOR_H

#include "selector.h"

class TypeSelector: public Selector {
public:
    TypeSelector(const String &typeName);

    inline String tagName();

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    String m_typeName;

};

#endif /* TYPESELECTOR_H */
