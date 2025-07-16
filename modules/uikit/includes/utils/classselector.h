#ifndef CLASSSELECTOR_H
#define CLASSSELECTOR_H

#include "selector.h"

class ClassSelector: public Selector {
public:
    ClassSelector(const String &cls);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    String m_class;

};

#endif /* CLASSSELECTOR_H */
