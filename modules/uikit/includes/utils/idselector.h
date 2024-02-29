#ifndef IDSELECTOR_H
#define IDSELECTOR_H

#include "selector.h"

class IdSelector: public Selector {
public:
    IdSelector(const std::string &id);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    std::string m_id;

};

#endif /* IDSELECTOR_H */
