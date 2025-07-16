#ifndef IDSELECTOR_H
#define IDSELECTOR_H

#include "selector.h"

class IdSelector: public Selector {
public:
    IdSelector(const String &id);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    String m_id;

};

#endif /* IDSELECTOR_H */
