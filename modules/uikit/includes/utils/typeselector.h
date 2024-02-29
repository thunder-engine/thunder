#ifndef TYPESELECTOR_H
#define TYPESELECTOR_H

#include "selector.h"

class TypeSelector: public Selector {
public:
    TypeSelector(const std::string &typeName);

    inline std::string tagName();

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    std::string m_typeName;

};

#endif /* TYPESELECTOR_H */
