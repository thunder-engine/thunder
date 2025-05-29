#include "components/toolbutton.h"

#include "components/recttransform.h"
#include "components/label.h"
#include <components/image.h>
#include "components/frame.h"
#include <components/menu.h>

#include <components/actor.h>

#include <resources/sprite.h>

#include <timer.h>
#include <input.h>

namespace {
    const char *gImage = "Image";
    const char *gMenu = "Menu";

    const char *gMenuFrame = "menu-frame";

    const float gCorner = 4.0f;
};

/*!
    \class ToolButton
    \brief The ToolButton class is a UI component that represents a button with an associated popup menu.
    \inmodule Gui

    The ToolButton class provides a UI component with a button-like appearance and an associated popup menu.
    It extends the functionality of AbstractButton to handle menu-related features.
    Users can show and hide the menu associated with the ToolButton, and the class also provides functionality to update the button text based on menu selections.
*/

ToolButton::ToolButton() :
        AbstractButton() {

    connect(this, _SIGNAL(pressed()), this, _SLOT(showMenu()));
}
/*!
    Returns the associated menu, or nullptr if no menu has been defined.
*/
Menu *ToolButton::menu() const {
    return static_cast<Menu *>(subWidget(gMenuFrame));
}
/*!
    Associates the given \a menu with this tool button.
    Ownership of the menu is not transferred to the tool button.
*/
void ToolButton::setMenu(Menu *menu) {
    setSubWidget(gMenuFrame, menu);

    if(menu) {
        connect(menu, _SIGNAL(aboutToHide()), this, _SLOT(hideMenu()));
        connect(menu, _SIGNAL(triggered(int)), this, _SLOT(onTriggered(int)));
        RectTransform *r = menu->rectTransform();
        if(r) {
            r->setAnchors(Vector2(0.0f), Vector2(0.0f));
        }
    }
}
/*!
    Shows the associated popup menu.
    Does nothing if no menu is associated.
*/
void ToolButton::showMenu() {
    Menu *menu = ToolButton::menu();
    if(menu) {
        Frame *back = background();
        if(back) {
            back->setCorners(Vector4(0, 0, gCorner, gCorner));
        }
        RectTransform *rect = menu->rectTransform();
        if(rect) {
            rect->setSize(Vector2(rectTransform()->size().x, rect->size().y));
        }
        menu->show(Vector2());
        Vector4 corners = menu->corners();
        corners.z = corners.w = 0.0f;
        menu->setCorners(corners);
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
    Menu *menu = ToolButton::menu();
    if(menu) {
        menu->hide();
    }
}
/*!
    \internal
    Overrides the composeComponent method to create the tool button component.
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

    Actor *actor = Engine::composeActor(gMenu, gMenu, ToolButton::actor());
    Menu *menu = static_cast<Menu *>(actor->component(gMenu));

    setMenu(menu);

    menu->addSection("Menu Item 1");
    menu->addSection("Menu Item 2");
    menu->addSection("Menu Item 3");
}
/*!
    \internal
    Slot function to handle the triggered signal from the associated menu. Updates the current item text.
*/
void ToolButton::onTriggered(int index) {
    Menu *menu = ToolButton::menu();
    if(menu) {
        m_currentItem = menu->itemText(index);
        setText(m_currentItem);
    }
}
