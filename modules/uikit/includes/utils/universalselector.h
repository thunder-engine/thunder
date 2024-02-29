#ifndef UNIVERSALSELECTOR_H
#define UNIVERSALSELECTOR_H

#include "selector.h"

class UniversalSelector: public Selector {
public:
    UniversalSelector();

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

};

#endif /* UNIVERSALSELECTOR_H */
