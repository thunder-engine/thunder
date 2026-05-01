#include "components/tabbar.h"

#include "components/recttransform.h"
#include "components/frame.h"
#include "components/button.h"

#include "resources/stylesheet.h"

#include "commandbuffer.h"

#define NAVI_SIZE 32

namespace {
    const char *gCssTabCornerRadius("tab-corner-radius");
};

/*!
    \class TabBar
    \brief The TabBar class provides a tab bar for switching between different pages or sections.
    \inmodule Gui

    TabBar is a widget that displays a list of tabs, allowing the user to select one.
    Each tab has a title and can be clicked to activate it.
    The TabBar automatically manages the layout and positioning of its tabs.
*/

TabBar::TabBar() :
        m_tabCornerRadius(4.0f, 4.0f, 0.0f, 0.0f),
        m_tabPadding(3.0f, 8.0f, 3.0f, 8.0f),
        m_frontButton(nullptr),
        m_backButton(nullptr),
        m_currentIndex(-1),
        m_visualIndex(-1),
        m_positionOffset(0.0f),
        m_showClose(false),
        m_blockUpdates(false) {

}

TabBar::~TabBar() {
}
/*!
    Adds a new tab with the specified \a title and returns its index.
*/
int TabBar::addTab(const TString &title) {
    return insertTab(-1, title);
}
/*!
    Inserts a new tab at the specified \a index with the given \a title.
*/
int TabBar::insertTab(int index, const TString &title) {
    Actor *buttonActor = Engine::composeActor<Button>(title, actor());
    Button *button = buttonActor->getComponent<Button>();
    if(button) {
        setSubWidget(button);

        button->setText(title);

        Frame *back = button->background();
        if(back) {
            back->setCorners(m_tabCornerRadius);
        }

        if(m_showClose) {
            createCloseButton(buttonActor);
        }

        connect(button, _SIGNAL(clicked()), this, _SLOT(onTabClicked()));

        // Calculate insertion position
        auto it = m_tabs.begin();
        if(index >= 0 && index < static_cast<int>(m_tabs.size())) {
            std::advance(it, index);
            m_tabs.insert(it, button);
        } else {
            m_tabs.push_back(button);
            index = m_tabs.size() - 1;
        }

        RectTransform *rect = button->rectTransform();
        rect->setPivot(Vector2(0.0f, 1.0f));
        rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));

        // If this is the first tab, select it
        if(m_currentIndex == -1) {
            setCurrentIndex(0);
        }

        updateTabPositions();

        return index;
    }

    return -1;
}
/*!
    Removes the tab at the specified \a index.
*/
void TabBar::removeTab(int index) {
    if(index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return;
    }

    auto it = m_tabs.begin();
    std::advance(it, index);

    Button *btn = (*it);
    m_tabs.erase(it);

    if(btn) {
        Actor *actor = btn->actor();
        actor->setEnabled(false);
        actor->deleteLater();
    }

    updateTabPositions();

    if(index >= m_tabs.size()) {
        m_currentIndex = m_tabs.size() - 1;
        m_visualIndex = m_currentIndex;
    }
    setCurrentIndex(m_currentIndex);
}
/*!
    Removes the tab associated with the given \a button.
*/
void TabBar::removeTab(Button *button) {
    if(!button) return;

    int index = 0;
    for(auto it = m_tabs.begin(); it != m_tabs.end(); ++it, ++index) {
        if((*it) == button) {
            removeTab(index);

            updateTabPositions();

            return;
        }
    }
}
/*!
    Returns the number of tabs.
*/
int TabBar::count() const {
    return m_tabs.size();
}
/*!
    Returns the index of the currently selected tab, or -1 if no tab is selected.
*/
int TabBar::currentIndex() const {
    return m_currentIndex;
}
/*!
    Sets the currently selected tab by \a index.
    Emits currentChanged signal if the selection changes.
*/
void TabBar::setCurrentIndex(int index) {
    if(index == m_currentIndex || index < -1 || index >= static_cast<int>(m_tabs.size())) {
        return;
    }

    m_currentIndex = index;
    m_visualIndex = m_currentIndex;

    if(m_backButton->isEnabled()) {
        recalcOffset();
    }

    int idx = 0;
    for(auto it : m_tabs) {
        it->blockSignals(true);
        it->setChecked(idx == m_currentIndex);
        it->blockSignals(false);
        idx++;
    }

    currentChanged(m_currentIndex);
}
/*!
    Returns true if tabs can be closed.
*/
bool TabBar::tabsClosable() const {
    return m_showClose;
}
/*!
    Sets whether tabs can be closed by the user.
    \param \a closable True if tabs can be closed, false otherwise.
*/
void TabBar::setTabsClosable(bool closable) {
    m_showClose = closable;

    for(auto tab : m_tabs) {
        Actor *actor = tab->actor();
        if(m_showClose) {
            createCloseButton(actor);
        } else {
            Object::ObjectList list = actor->getChildren();
            for(auto it : list) {
                if(it->name() == "close") {
                    delete it;
                }
            }
        }
    }

    updateTabPositions();
}
/*!
    Returns the title of the tab at the specified \a index.
*/
TString TabBar::tabTitle(int index) const {
    if(index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return TString();
    }
    auto it = m_tabs.begin();
    std::advance(it, index);
    return (*it)->text();
}
/*!
    Sets the \a title of the tab at the specified \a index.
*/
void TabBar::setTabTitle(int index, const TString &title) {
    if(index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return;
    }

    auto it = m_tabs.begin();
    std::advance(it, index);

    if(*it) {
        (*it)->setText(title);

        updateTabPositions();
    }
}
/*!
    Returns the button widget for the tab at the specified \a index.
*/
Button *TabBar::tabButton(int index) const {
    if(index < 0 || index >= static_cast<int>(m_tabs.size())) {
        return nullptr;
    }
    auto it = m_tabs.begin();
    std::advance(it, index);
    return *it;
}
/*!
    Returns the index of the tab that contains the specified \a button, or -1 if not found.
*/
int TabBar::indexOf(Button *button) const {
    if(!button) return -1;

    int index = 0;
    for(const auto &tab : m_tabs) {
        if(tab == button) {
            return index;
        }
        index++;
    }
    return -1;
}
/*!
    Sets the corner radius for tabs.
*/
void TabBar::setTabCornerRadius(const Vector4 &radius) {
    m_tabCornerRadius = radius;
    for(auto it : m_tabs) {
        if(it->background()) {
            it->background()->setCorners(m_tabCornerRadius);
        }
    }
#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        updateStyleProperty(gCssTabCornerRadius, m_tabCornerRadius.v, 4);
    }
#endif
}
/*!
    Returns the corner radius for tabs.
*/
Vector4 TabBar::tabCornerRadius() const {
    return m_tabCornerRadius;
}
/*!
    \internal
*/
bool TabBar::isHovered(const Vector2 &pos) {
    RectTransform *rect = rectTransform();
    Vector4 area = rect->scissorArea();

    if(m_backButton->isEnabled()) {
        area.z -= NAVI_SIZE;
    }

    return (pos.x > area.x && pos.x < area.x + area.z && pos.y > area.y && pos.y < area.y + area.w);
}
/*!
    \internal
*/
void TabBar::update(const Vector2 &pos) {
    m_blockUpdates = false;
    if(isHovered(pos)) {
        for(auto it : m_childWidgets) {
            // To protect from multiple close of tabs
            if(!m_blockUpdates && it->isEnabled() && it->actor()->isEnabled()) {
                it->update(pos);
            }
        }
    } else {
        for(auto it : m_tabs) {
            it->setHovered(false, true);
        }

        if(m_frontButton && m_frontButton->isEnabled()) {
            static_cast<Widget *>(m_frontButton)->update(pos);
        }
        if(m_backButton && m_backButton->isEnabled()) {
            static_cast<Widget *>(m_backButton)->update(pos);
        }
    }
}
/*!
    \internal
*/
void TabBar::draw(CommandBuffer &buffer) {
    RectTransform *rect = rectTransform();
    Vector4 area(rect->scissorArea());

    if(m_backButton->isEnabled()) {
        m_backButton->draw(buffer);
        area.z -= NAVI_SIZE;
    }

    if(m_frontButton->isEnabled()) {
        m_frontButton->draw(buffer);
    }

    buffer.enableScissor(area.x, area.y, area.z, area.w);

    for(auto it : m_childWidgets) {
        if(it->isSubWidget() && it->actor()->isEnabled() && it->isEnabled()) {
            if(it != m_frontButton && it != m_backButton) {
                it->draw(buffer);
            }
        }
    }

    buffer.disableScissor();
}
/*!
    \internal
*/
void TabBar::composeComponent() {
    Widget::composeComponent();

    Actor *frontActor = Engine::composeActor<Button>("front", actor());
    m_frontButton = frontActor->getComponent<Button>();
    if(m_frontButton) {
        setSubWidget(m_frontButton);
        m_frontButton->setIcon(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
        m_frontButton->setIconSize(Vector2(16.0f, 8.0f));
        m_frontButton->setEnabled(false);

        Image *image = m_frontButton->image();
        RectTransform *imageRect = image->rectTransform();
        imageRect->setRotation(Vector3(0.0f, 0.0f, 90.0f));

        RectTransform *rect = m_frontButton->rectTransform();
        rect->setSize(Vector2(16));
        rect->setPivot(Vector2(1.0f, 1.0f));
        rect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f));

        connect(m_frontButton, _SIGNAL(clicked()), this, _SLOT(onFrontClicked()));
    }

    Actor *backActor = Engine::composeActor<Button>("back", actor());
    m_backButton = backActor->getComponent<Button>();
    if(m_backButton) {
        setSubWidget(m_backButton);
        m_backButton->setIcon(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
        m_backButton->setIconSize(Vector2(16.0f, 8.0f));
        m_backButton->setEnabled(false);

        Image *image = m_backButton->image();
        RectTransform *imageRect = image->rectTransform();
        imageRect->setRotation(Vector3(0.0f, 0.0f,-90.0f));

        RectTransform *rect = m_backButton->rectTransform();
        rect->setSize(Vector2(16));
        rect->setPivot(Vector2(1.0f, 1.0f));
        rect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f));
        rect->setPosition(Vector3(-16.0f, 0.0f, 0.0f));

        connect(m_backButton, _SIGNAL(clicked()), this, _SLOT(onBackClicked()));
    }
}
/*!
    \internal
*/
void TabBar::applyStyle() {
    Widget::applyStyle();

    blockSignals(true);

    auto it = m_styleRules.find(gCssTabCornerRadius);
    if(it != m_styleRules.end()) {
        bool pixels;
        Vector4 radius = styleBlock4Length(gCssTabCornerRadius, m_tabCornerRadius, pixels);
        setTabCornerRadius(radius);
    }

    blockSignals(false);
}
/*!
    \internal
*/
void TabBar::boundChanged(const Vector2 &size) {
    updateTabPositions();
}
/*!
    \internal
*/
void TabBar::updateTabPositions() {
    float position = -m_positionOffset;
    float totalWidth = 0.0f;
    for(auto it : m_tabs) {
        RectTransform *rect = it->rectTransform();
        float width = m_tabPadding.y + m_tabPadding.w;
        Label *label = it->label();
        if(label) {
            width += label->textWidth();
            label->setAlign(Alignment::Middle | Alignment::Left);
            label->rectTransform()->setPosition(Vector3(m_tabPadding.w, 0.0f, 0.0f));
        }

        Image *image = it->image();
        if(image) {
            width += rect->padding().w;
            width += it->iconSize().x;
        }

        if(m_showClose) {
            width += m_tabPadding.w + 16;
        }

        rect->setSize(Vector2(width, rect->size().y));
        rect->setPosition(Vector3(position, 0.0f, 0.0f));
        position += width;
        totalWidth += width;
    }

    bool showBackFront = (totalWidth > 0.0f) && (totalWidth > rectTransform()->size().x);
    if(m_backButton) {
        bool enabled = m_backButton->isEnabled();
        m_backButton->setEnabled(showBackFront);
        if(enabled && !showBackFront) {
            m_positionOffset = 0.0f;
            updateTabPositions();
        }
    }
    if(m_frontButton) {
        m_frontButton->setEnabled(showBackFront);
    }
}
/*!
    \internal
*/
void TabBar::createCloseButton(Actor *parent) {
    Actor *closeActor = Engine::composeActor<Button>("close", parent);
    Button *closeBtn = closeActor->getComponent<Button>();
    if(closeBtn) {
        RectTransform *closeRect = closeBtn->rectTransform();
        if(closeRect) {
            closeRect->blockSignals(true);
            closeRect->setAnchors(Vector2(1.0f, 0.5f), Vector2(1.0f, 0.5f));
            closeRect->setPivot(Vector2(1.0f, 0.5f));
            closeRect->setPosition(Vector3(-4.0f, 0.0f, 0.0f));
            closeRect->setSize(Vector2(16));
            closeRect->blockSignals(false);
        }

        connect(closeBtn, _SIGNAL(clicked()), this, _SLOT(onCloseButtonClicked()));
    }
}
/*!
    \internal
*/
void TabBar::recalcOffset() {
    float barWidth = rectTransform()->size().x - NAVI_SIZE;
    float current = 0.0f;
    int idx = 0;
    for(auto it : m_tabs) {
        RectTransform *rect = it->rectTransform();
        current += rect->size().x;
        if(idx == m_visualIndex) {
            m_positionOffset = (current > barWidth) ? (current - barWidth) : 0.0f;
            updateTabPositions();
            break;
        }
        ++idx;
    }
}
/*!
    This signal is emitted when the tab bar's current tab changes.
    The new current has the given \a index, or -1 if there isn't a new one
*/
void TabBar::currentChanged(int index) {
    emitSignal(_SIGNAL(currentChanged(int)), index);
}
/*!
    This signal is emitted when the close button on a tab is clicked.
    The index is the \a index that should be removed.
*/
void TabBar::tabCloseRequested(int index) {
    emitSignal(_SIGNAL(tabCloseRequested(int)), index);
}
/*!
    Called when a tab is clicked.
*/
void TabBar::onTabClicked() {
    Button *button = dynamic_cast<Button *>(sender());
    if(button) {
        int index = indexOf(button);
        if(index != -1) {
            setCurrentIndex(index);
        }
    }
}
/*!
    Called when a tab close button clicked.
*/
void TabBar::onCloseButtonClicked() {
    Button *closeButton = dynamic_cast<Button *>(sender());
    if(closeButton) {
        Actor *parentActor = dynamic_cast<Actor *>(closeButton->actor()->parent());
        if(parentActor) {
            Button *tab = parentActor->getComponent<Button>();
            if(tab) {
                int index = indexOf(tab);
                if(index > -1) {
                    tabCloseRequested(index);
                    // We need to stop updates for next widgets on this turn
                    m_blockUpdates = true;
                }
            }
        }
    }
}
/*!
    \internal
*/
void TabBar::onBackClicked() {
    if(m_visualIndex > 0) {
        --m_visualIndex;
        recalcOffset();
    }
}
/*!
    \internal
*/
void TabBar::onFrontClicked() {
    if(m_visualIndex < m_tabs.size() - 1) {
        ++m_visualIndex;
        recalcOffset();
    }
}
