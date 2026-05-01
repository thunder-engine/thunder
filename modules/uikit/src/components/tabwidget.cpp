#include "components/tabwidget.h"

#include "components/recttransform.h"
#include "components/frame.h"
#include "components/layout.h"
#include "components/tabbar.h"

#include "resources/stylesheet.h"

/*!
    \class TabWidget
    \brief A widget that provides a tab bar and a stack of pages, allowing users to switch between different content areas.
    \inmodule Gui

    The TabWidget class is a container widget that organizes multiple pages into a tabbed interface.
    Each tab has a label and can contain any widget as its content.
*/

TabWidget::TabWidget() :
        m_currentIndex(-1),
        m_tabBar(nullptr),
        m_contentArea(nullptr),
        m_tabBarHeight(30.0f) {

}

TabWidget::~TabWidget() {

}

/*!
    Adds a new tab with the specified \a title and returns its index.
    The \a content widget will be displayed when this tab is selected.
*/
int TabWidget::addTab(const TString &title, Widget *content) {
    return insertTab(-1, title, content);
}
/*!
    Inserts a new tab at the specified \a index with the given \a title and \a content.
*/
int TabWidget::insertTab(int index, const TString &title, Widget *content) {
    return insertTab(index, title, content->rectTransform());
}
/*!
    Removes the tab at the specified \a index.
*/
void TabWidget::removeTab(int index) {
    if(index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return;
    }

    // Remove tab button
    m_tabBar->blockSignals(true);
    m_tabBar->removeTab(index);
    m_tabBar->blockSignals(false);

    auto it = m_tabs.begin();
    std::advance(it, index);

    // Remove content from layout
    RectTransform *rect = *it;
    m_tabs.erase(it);

    if(rect) {
        rect->actor()->deleteLater();
    }

    if(index >= m_tabs.size()) {
        m_currentIndex = m_tabs.size() - 1;
    }
    setCurrentIndex(m_currentIndex);
}
/*!
    Returns the number of tabs.
*/
int TabWidget::count() const {
    return m_tabs.size();
}
/*!
    Returns the index of the currently selected tab, or -1 if no tab is selected.
*/
int TabWidget::currentIndex() const {
    return m_currentIndex;
}
/*!
    Sets the currently selected tab by \a index.
    Emits currentChanged signal if the selection changes.
*/
void TabWidget::setCurrentIndex(int index) {
    if(index < -1 || index >= static_cast<int>(m_tabs.size())) {
        return;
    }

    // Hide current content
    if(m_currentIndex >= 0) {
        auto it = m_tabs.begin();
        std::advance(it, m_currentIndex);
        Actor *actor = (*it)->actor();
        actor->setEnabled(false);
    }

    m_currentIndex = index;

    // Show new content
    if(m_currentIndex >= 0) {
        auto it = m_tabs.begin();
        std::advance(it, m_currentIndex);
        Actor *actor = (*it)->actor();
        actor->setEnabled(true);
        m_contentArea->layout()->invalidate();
    }

    m_tabBar->blockSignals(true);
    m_tabBar->setCurrentIndex(m_currentIndex);
    m_tabBar->blockSignals(false);
}
/*!
    Returns the title of the tab at the specified \a index.
*/
TString TabWidget::tabTitle(int index) const {
    return m_tabBar->tabTitle(index);
}
/*!
    Sets the \a title of the tab at the specified \a index.
*/
void TabWidget::setTabTitle(int index, const TString &title) {
    m_tabBar->setTabTitle(index, title);
}
/*!
    Returns true if tabs can be closed.
*/
bool TabWidget::tabsClosable() const {
    return m_tabBar->tabsClosable();
}
/*!
    Sets whether tabs can be closed by the user.
    \param \a closable True if tabs can be closed, false otherwise.
*/
void TabWidget::setTabsClosable(bool closeable) {
    m_tabBar->setTabsClosable(closeable);
}
/*!
    Returns the content widget of the tab at the specified \a index.
*/
Widget *TabWidget::tabContent(int index) const {
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return nullptr;
    }
    auto it = m_tabs.begin();
    std::advance(it, index);
    return (*it)->widget();
}
/*!
    Sets the tab \a bar associated with TabWidget.
*/
void TabWidget::setTabBar(TabBar *bar) {
    setSubWidget(bar);

    m_tabBar = bar;
    if(m_tabBar) {
        RectTransform *rect = m_tabBar->rectTransform();
        rect->setSize(Vector2(rect->size().x, m_tabBarHeight));
        rect->setHorizontalPolicy(RectTransform::Expanding);
        rect->setVerticalPolicy(RectTransform::Fixed);

        connect(m_tabBar, _SIGNAL(currentChanged(int)), this, _SLOT(setCurrentIndex(int)));
        connect(m_tabBar, _SIGNAL(tabCloseRequested(int)), this, _SLOT(removeTab(int)));
    }
}
/*!
    Sets the content \a area associated with TabWidget.
*/
void TabWidget::setContentArea(Frame *area) {
    setSubWidget(area);

    m_contentArea = area->rectTransform();
    if(m_contentArea && !m_contentArea->layout()) {
        m_contentArea->setHorizontalPolicy(RectTransform::Expanding);
        m_contentArea->setVerticalPolicy(RectTransform::Expanding);

        Layout *contentLayout = new Layout();
        contentLayout->setOrientation(Widget::Vertical);
        m_contentArea->setLayout(contentLayout);
    }
}

void TabWidget::updateContentVisibility() {
    int idx = 0;
    for (auto &tab : m_tabs) {
        if (tab) {
            tab->actor()->setEnabled(idx == m_currentIndex);
        }
        idx++;
    }
}

void TabWidget::onTabClicked(int index) {
    if(m_currentIndex != index) {
        setCurrentIndex(index);
    }
}
/*!
    \internal
    Internal method called to compose the tab widget component.
*/
void TabWidget::composeComponent() {
    // Create tab bar container
    Actor *tabBarActor = Engine::composeActor<TabBar>("tabBar", actor());
    setTabBar(tabBarActor->getComponent<TabBar>());

    // Create content area
    Actor *contentAreaActor = Engine::composeActor<Frame>("contentArea", actor());
    setContentArea(contentAreaActor->getComponent<Frame>());

    // Need to call it after sub widgets creation
    Widget::composeComponent();

    // Set up layout for the main widget
    Layout *mainLayout = new Layout();
    mainLayout->setOrientation(Widget::Vertical);

    RectTransform *rect = rectTransform();
    rect->setLayout(mainLayout);

    // Initially add tab bar and content area to layout
    mainLayout->addTransform(m_tabBar->rectTransform());
    mainLayout->addTransform(m_contentArea);

    rect->setSize(Vector2(200.0f, 200.0f));
}
/*!
    \internal
*/
void TabWidget::childAdded(RectTransform *rect) {
    if(rect && rect != m_tabBar->rectTransform() && rect != m_contentArea) {
        for(auto it : m_tabs) {
            if(it == rect) {
                return;
            }
        }

        insertTab(-1, rect->actor()->name(), rect);
    }
}
/*!
    \internal
*/
int TabWidget::insertTab(int index, const TString &title, RectTransform *content) {
    if(!content) {
        return -1;
    }

    Actor *actor = content->actor();
    if(actor) {
        connect(actor, _SIGNAL(objectNameChanged(TString)), this, _SLOT(onObjectNameChanged(TString)));
        if(m_currentIndex != -1) {
            actor->setEnabled(false);
        }
    }

    m_tabBar->insertTab(index, title);

    // Calculate insertion position
    auto it = m_tabs.begin();
    if(index >= 0 && index < static_cast<int>(m_tabs.size())) {
        std::advance(it, index);
        m_tabs.insert(it, content);
    } else {
        m_tabs.push_back(content);
        index = m_tabs.size() - 1;
    }

    content->setHorizontalPolicy(RectTransform::Expanding);
    content->setVerticalPolicy(RectTransform::Expanding);
    m_contentArea->layout()->addTransform(content);

    // If this is the first tab, select it
    if(m_currentIndex == -1) {
        setCurrentIndex(0);
    }

    return index;
}
/*!
    \internal
*/
void TabWidget::onObjectNameChanged(const TString &objectName) {
    Actor *actor = dynamic_cast<Actor *>(sender());
    if(actor) {
        RectTransform *rect = static_cast<RectTransform *>(actor->transform());
        int idx = 0;
        for(auto &tab : m_tabs) {
            if(tab == rect) {
                m_tabBar->setTabTitle(idx, objectName);
                return;
            }
            idx++;
        }
    }
}
