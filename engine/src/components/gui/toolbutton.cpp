#include "components/gui/toolbutton.h"

#include "components/actor.h"
#include "components/gui/recttransform.h"
#include "components/gui/label.h"
#include "components/gui/frame.h"
#include <components/gui/menu.h>

#include "resources/sprite.h"

#include "timer.h"
#include "input.h"

namespace {
    const char *gBackground = "Frame";
    const char *gImage = "Image";
    const char *gMenu = "Menu";

    const float gCorner = 4.0f;
};

ToolButton::ToolButton() :
        AbstractButton(),
        m_menu(nullptr) {

    connect(this, _SIGNAL(pressed()), this, _SLOT(showMenu()));
}
/*!
    Returns the associated menu, or nullptr if no menu has been defined.
*/
Menu *ToolButton::menu() const {
    return m_menu;
}
/*!
    Associates the given \a menu with this tool button.
    Ownership of the menu is not transferred to the tool button.
*/
void ToolButton::setMenu(Menu *menu) {
    disconnect(m_menu, nullptr, this, nullptr);

    m_menu = menu;
    if(m_menu) {
        connect(m_menu, _SIGNAL(aboutToHide()), this, _SLOT(hideMenu()));
        connect(m_menu, _SIGNAL(triggered(int)), this, _SLOT(onTriggered(int)));
        RectTransform *r = m_menu->rectTransform();
        if(r) {
            r->setAnchors(Vector2(0.0f), Vector2(0.0f));
        }
    }
}
/*!
    Shows the associated popup menu.
    If there is no such menu, this function does nothing.
*/
void ToolButton::showMenu() {
    if(m_menu) {
        Frame *back = background();
        if(back) {
            back->setCorners(Vector4(0, 0, gCorner, gCorner));
        }
        RectTransform *rect = m_menu->rectTransform();
        if(rect) {
            rect->setSize(Vector2(rectTransform()->size().x, rect->size().y));
        }
        m_menu->show(Vector2());
        Vector4 corners = m_menu->corners();
        corners.z = corners.w = 0.0f;
        m_menu->setCorners(corners);
    }
}
/*!
    Hides the associated popup menu.
*/
void ToolButton::hideMenu() {
    Frame *back = background();
    if(back) {
        back->setCorners(Vector4(gCorner));
    }
    if(m_menu) {
        m_menu->hide();
    }
}
/*!
    \internal
*/
void ToolButton::composeComponent() {
    AbstractButton::composeComponent();

    Frame *back = background();
    if(back) {
        back->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
    }
    // Add icon
    Actor *icon = Engine::composeActor(gImage, gImage, actor());
    Image *image = static_cast<Image *>(icon->component(gImage));

    image->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
    image->setItem("Arrow");
    RectTransform *t = image->rectTransform();
    if(t) {
        t->setSize(Vector2(16.0f, 8.0f));
        t->setAnchors(Vector2(1.0f, 0.5f), Vector2(1.0f, 0.5f));
        t->setPivot(Vector2(1.0f, 0.5f));
    }

    Actor *menu = Engine::composeActor(gMenu, gMenu, ToolButton::actor());
    setMenu(static_cast<Menu *>(menu->component(gMenu)));

    m_menu->addSection("Test 1");
    m_menu->addSection("Test 2");
    m_menu->addSection("Test 3");
}
/*!
    \internal
*/
void ToolButton::onTriggered(int index) {
    if(m_menu) {
        m_currentItem = m_menu->itemText(index);
        setText(m_currentItem);
    }
}
