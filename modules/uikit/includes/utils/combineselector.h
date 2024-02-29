#ifndef COMBINESELECTOR_H
#define COMBINESELECTOR_H

#include "selector.h"

#include <vector>
#include <list>

class Widget;

class CombineSelector: public Selector {
public:
    enum CombineType {
        InstanceSibling,
        NormalSibling,
        InstanceInherical,
        NormalInherical,
        NoCombine
    };

public:
    CombineSelector();
    ~CombineSelector();

    void initialInstanceSiblingList(Selector *head, Selector *sibling);
    void initialNormalSiblingList(Selector *head, Selector *sibling);
    void initialInstanceInhericalList(Selector *root, Selector *child);
    void initialNormalInhericalList(Selector *root, Selector *child);

    bool isMeet(Widget *widget) override;
    bool isBaseSelector() const override;
    int weight() override;

    Selector *before();
    Selector *after();

    std::vector<Widget *> matchingWidgets;

private:
    std::list<Selector*> m_instanceSiblingList;
    std::list<Selector *> m_normalSiblingList;
    std::list<Selector *> m_instanceInhericalList;
    std::list<Selector *> m_normalInhericalList;

    CombineType m_combineType;

};

#endif /* COMBINESELECTOR_H */
