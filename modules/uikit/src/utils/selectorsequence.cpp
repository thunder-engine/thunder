#include "utils/selectorsequence.h"

SequenceSelector::SequenceSelector() {
    m_selectorType = Selector::SelectorSequence;
}

SequenceSelector::~SequenceSelector() {
    for(auto it : m_selectors) {
        delete it;
    }
}

void SequenceSelector::appendSelector(Selector *s) {
    if(!s) {
        return;
    }
    m_selectors.push_back(s);
}

bool SequenceSelector::isMeet(Widget *widget) {
    for(const auto &s : m_selectors) {
        if(!s->isMeet(widget)) {
            return false;
        }
    }
    return true;
}

bool SequenceSelector::isBaseSelector() const {
    return false;
}

int SequenceSelector::weight() {
    auto it = m_selectors.begin();

    int w = 0;
    while(it != m_selectors.end()) {
        w += (*it++)->weight();
    }

    return w;
}
