#include "utils/combineselector.h"

#include "components/widget.h"

#include <assert.h>

CombineSelector::CombineSelector() {
    m_selectorType = Selector::CombineSelector;
    m_combineType = NoCombine;
}

CombineSelector::~CombineSelector() {
    for(auto it : m_normalSiblingList) {
        delete it;
    }

    for(auto it : m_instanceSiblingList) {
        delete it;
    }

    for(auto it : m_normalInhericalList) {
        delete it;
    }

    for(auto it : m_instanceSiblingList) {
        delete it;
    }
}
    
void CombineSelector::initialNormalSiblingList(Selector *head, Selector *sibling) {
    if(m_combineType != NoCombine) {
        assert(0);
    }
    if(!head || !sibling) {
        return;
    }

    for(auto it : m_normalSiblingList) {
        delete it;
    }
    m_normalSiblingList.clear();

    m_normalSiblingList.push_back(head);
    m_normalSiblingList.push_back(sibling);
    m_combineType = NormalSibling;
}

void CombineSelector::initialInstanceSiblingList(Selector *head, Selector *sibling) {
    if(m_combineType != NoCombine) {
        assert(0);
    }

    if(!head || !sibling) {
        return;
    }

    for(auto it : m_instanceSiblingList) {
        delete it;
    }
    m_instanceSiblingList.clear();

    m_instanceSiblingList.push_back(head);
    m_instanceSiblingList.push_back(sibling);
    m_combineType = InstanceSibling;
}

void CombineSelector::initialNormalInhericalList(Selector *root, Selector *child) {
    if(m_combineType != NoCombine) {
        assert(0);
    }

    if(!root || !child) {
        return;
    }

    for(auto it : m_normalInhericalList) {
        delete it;
    }
    m_normalInhericalList.clear();

    m_normalInhericalList.push_back(root);
    m_normalInhericalList.push_back(child);
    m_combineType = NormalInherical;
}

void CombineSelector::initialInstanceInhericalList(Selector *root, Selector *child) {
    if (m_combineType != NoCombine) {
        assert(0);
    }

    if (!root || !child) {
        return;
    }

    for(auto it : m_instanceInhericalList) {
        delete it;
    }
    m_instanceInhericalList.clear();

    m_instanceInhericalList.push_back(root);
    m_instanceInhericalList.push_back(child);
    m_combineType = InstanceInherical;
}

bool CombineSelector::isMeet(Widget *widget) {
    matchingWidgets.clear();
    Selector *before = CombineSelector::before();
    Selector *after = CombineSelector::after();

    if(!before || !after) {
        return false;
    }

    Widget *parent = widget->parentWidget();

    if(!parent || !after->isMeet(widget)) {
        return false;
    }

    std::vector<Widget *> nodesToCheck;
    if(after->type() == Selector::CombineSelector) {
        auto cs = static_cast<CombineSelector *>(after);
        nodesToCheck.insert(nodesToCheck.end(), cs->matchingWidgets.begin(), cs->matchingWidgets.end());
    } else {
        nodesToCheck.push_back(widget);
    }

    for(size_t i = 0; i < nodesToCheck.size(); i++) {
        widget = nodesToCheck[i];
        parent = widget->parentWidget();

        if(!parent) {
            continue;
        }

        switch(m_combineType) {
            case CombineSelector::NormalSibling: { // Unsupported
                //int idx = widget->GetIndexWithinParent();
                //for(const auto &sibling : parent->childWidgets()) {
                //    if(sibling->GetIndexWithinParent() >= idx) {
                //        continue;
                //    }
                //    if(doesWidgetSelector(sibling, before)) {
                //        selector->matchingWidgets.push_back(sibling);
                //    }
                //}
                break;
            }
            case CombineSelector::InstanceSibling: {
                Widget *lastWidget = nullptr;
                for(const auto &sibling : parent->childWidgets()) {
                    if(lastWidget && sibling == widget) {
                        if(before->isMeet(lastWidget)) {
                            matchingWidgets.push_back(lastWidget);
                        }
                    }
                    lastWidget = sibling;
                }
                break;
            }
            case CombineSelector::InstanceInherical: {
                if(before->isMeet(parent)) {
                    matchingWidgets.push_back(parent);
                }
                break;
            }
            case CombineSelector::NormalInherical: {
                while(parent) {
                    if(before->isMeet(parent)) {
                        matchingWidgets.push_back(parent);
                        break;
                    }
                    parent = parent->parentWidget();
                }
                break;
            }
            default: break;
        }
    }

    return !matchingWidgets.empty();
}

bool CombineSelector::isBaseSelector() const {
    return false;
}

int CombineSelector::weight() {
    int w = 0;
    std::list<Selector *>::iterator one;
    std::list<Selector *>::iterator other;
    do {
        if (m_normalInhericalList.size() == 2) {
            one = m_normalInhericalList.begin();
            other = --m_normalInhericalList.end();
            break;
        }
        if (m_instanceInhericalList.size() == 2) {
            one = m_instanceInhericalList.begin();
            other = --m_instanceInhericalList.end();
            break;
        }
        if (m_normalSiblingList.size() == 2) {
            one = m_normalSiblingList.begin();
            other = --m_normalSiblingList.end();
            break;
        }
        if (m_instanceSiblingList.size() == 2) {
            one = m_instanceSiblingList.begin();
            other = --m_instanceSiblingList.end();
            break;
        }
    } while(0);

    if(!*one || !*other) {
        return w;
    }

    w += (*one)->weight() + (*other)->weight();
    return w;
}

Selector* CombineSelector::before() {
    std::list<Selector *>::iterator before;
    do {
        if(m_normalInhericalList.size() == 2) {
            before = m_normalInhericalList.begin();
            break;
        }

        if(m_instanceInhericalList.size() == 2) {
            before = m_instanceInhericalList.begin();
            break;
        }

        if(m_normalSiblingList.size() == 2) {
            before = m_normalSiblingList.begin();
            break;
        }

        if(m_instanceSiblingList.size() == 2) {
            before = m_instanceSiblingList.begin();
            break;
        }
    } while (0);
    return *before;
}

Selector* CombineSelector::after() {
    std::list<Selector *>::iterator after;
    do {
        if(m_normalInhericalList.size() == 2) {
            after = --m_normalInhericalList.end();
            break;
        }

        if(m_instanceInhericalList.size() == 2) {
            after = --m_instanceInhericalList.end();
            break;
        }

        if(m_normalSiblingList.size() == 2) {
            after = --m_normalSiblingList.end();
            break;
        }

        if(m_instanceSiblingList.size() == 2) {
            after = --m_instanceSiblingList.end();
            break;
        }
    } while (0);
    return *after;
}
