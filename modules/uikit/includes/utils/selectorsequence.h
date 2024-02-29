#ifndef SELECTORSEQUENCE_H
#define SELECTORSEQUENCE_H

#include "selector.h"

#include <list>

class SequenceSelector: public Selector {
public:
    SequenceSelector();
    ~SequenceSelector();

    void appendSelector(Selector *);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    std::list<Selector *> m_selectors;

};

#endif /* SELECTORSEQUENCE_H */
