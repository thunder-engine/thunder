#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "widget.h"

#include <list>

class RectTransform;
class Frame;
class TabBar;

class UIKIT_EXPORT TabWidget : public Widget {
    A_OBJECT(TabWidget, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(bool, tabsClosable, TabWidget::tabsClosable, TabWidget::setTabsClosable)
    )
    A_METHODS(
        A_SLOT(TabWidget::removeTab),
        A_SLOT(TabWidget::setCurrentIndex),
        A_SLOT(TabWidget::onObjectNameChanged)
    )
    A_NOENUMS()

public:
    TabWidget();
    ~TabWidget();

    int addTab(const TString &title, Widget *content);
    int insertTab(int index, const TString &title, Widget *content);
    void removeTab(int index);

    int count() const;

    int currentIndex() const;
    void setCurrentIndex(int index);

    TString tabTitle(int index) const;
    void setTabTitle(int index, const TString &title);

    bool tabsClosable() const;
    void setTabsClosable(bool closeable);

    Widget *tabContent(int index) const;

    void setTabBar(TabBar *bar);
    void setContentArea(Frame *area);

protected:
    void composeComponent() override;

    void childAdded(RectTransform *rect) override;

    int insertTab(int index, const TString &title, RectTransform *content);

    void onObjectNameChanged(const TString &objectName);

private:
    void updateContentVisibility();
    void onTabClicked(int index);
    void updateTabStyles();

    std::list<RectTransform *> m_tabs;

    TabBar *m_tabBar;
    RectTransform *m_contentArea;

    int m_currentIndex;

    float m_tabBarHeight;
};

#endif // TABWIDGET_H
