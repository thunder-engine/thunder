#ifndef TABBAR_H
#define TABBAR_H

#include "components/button.h"

#include <list>

class UIKIT_EXPORT TabBar : public Widget {
    A_OBJECT(TabBar, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(bool, tabsClosable, TabBar::tabsClosable, TabBar::setTabsClosable)
    )
    A_METHODS(
        A_SIGNAL(TabBar::currentChanged),
        A_SIGNAL(TabBar::tabCloseRequested),
        A_SLOT(TabBar::onTabClicked),
        A_SLOT(TabBar::onCloseButtonClicked),
        A_SLOT(TabBar::onBackClicked),
        A_SLOT(TabBar::onFrontClicked)
    )
    A_NOENUMS()

public:
    TabBar();
    ~TabBar();

    int addTab(const TString &title);
    int insertTab(int index, const TString &title);
    void removeTab(int index);
    void removeTab(Button *button);

    int count() const;

    int currentIndex() const;
    void setCurrentIndex(int index);

    bool tabsClosable() const;
    void setTabsClosable(bool closable);

    TString tabTitle(int index) const;
    void setTabTitle(int index, const TString &title);

    Button *tabButton(int index) const;
    int indexOf(Button *button) const;

    Vector4 tabCornerRadius() const;
    void setTabCornerRadius(const Vector4 &radius);

public: //signals
    void currentChanged(int index);

    void tabCloseRequested(int index);

protected:
    bool isHovered(const Vector2 &pos) override;

    void update(const Vector2 &pos) override;

    void draw(CommandBuffer &buffer) override;

    void composeComponent() override;

    void applyStyle() override;

    void boundChanged(const Vector2 &size) override;

    void updateTabPositions();

    void onTabClicked();
    void onCloseButtonClicked();

    void onBackClicked();
    void onFrontClicked();

    void createCloseButton(Actor *parent);

    void recalcOffset();

private:
    std::list<Button *> m_tabs;

    Vector4 m_tabCornerRadius;

    Vector4 m_tabPadding;

    Button *m_frontButton;

    Button *m_backButton;

    int m_currentIndex;

    int m_visualIndex;

    float m_positionOffset;

    bool m_showClose;

    bool m_blockUpdates;

};

#endif // TABBAR_H
