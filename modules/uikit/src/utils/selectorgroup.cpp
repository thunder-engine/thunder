#include "utils/selectorgroup.h"

#include <algorithm>

GroupSelector::GroupSelector() {
    m_selectorType = Selector::SelectorGroup;
}

GroupSelector::~GroupSelector() {
    for(auto it : m_selectors) {
        delete it;
    }
    m_selectors.clear();
}

void GroupSelector::addSelector(Selector *s) {
    if(!s) {
        return;
    }

    m_selectors.push_back(s);
}

bool GroupSelector::isMeet(Widget *widget) {
    for(const auto &s : m_selectors) {
        if(s->isMeet(widget)) {
            targetSelector = (std::find(m_selectors.begin(), m_selectors.end(), s) - m_selectors.begin());

            return true;
        }
    }
    return false;
}
    
bool GroupSelector::isBaseSelector() const {
    return false;
}

int GroupSelector::weight() {
    if(m_selectors.size() < targetSelector + 1) {
        return 0;
    }

    return m_selectors[targetSelector]->weight();
}
