#ifndef IDSELECTOR_H
#define IDSELECTOR_H

#include "selector.h"

class IdSelector: public Selector {
public:
    IdSelector(const TString &id);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    TString m_id;

};

#endif /* IDSELECTOR_H */
