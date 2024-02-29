#ifndef SELECTORGROUP_H
#define SELECTORGROUP_H

#include "selector.h"

#include <vector>

class GroupSelector: public Selector {
public:
    GroupSelector();
    ~GroupSelector();

    void addSelector(Selector *);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    std::vector<Selector *> m_selectors;

    unsigned int targetSelector = 0;

};

#endif /* SELECTORGROUP_H */
