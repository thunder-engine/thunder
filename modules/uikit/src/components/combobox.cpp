/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "combobox.h"

#include "image.h"
#include "menu.h"

#include <resources/sprite.h>

#include <timer.h>
#include <input.h>

namespace {
    const char *gIcon("icon");

    const char *gMenuFrame("menu-frame");

    const float gCorner = 4.0f;
};

/*!
    \class ComboBox
    \brief The ComboBox class is a UI component that represents a button with an associated popup menu.
    \inmodule Gui

    The ComboBox class provides a UI component with a button-like appearance and an associated popup menu.
    It extends the functionality of AbstractButton to handle menu-related features.
    Users can show and hide the menu associated with the ComboBox, and the class also provides functionality to update the button text based on menu selections.
*/

ComboBox::ComboBox() :
        Button() {

    connect(this, _SIGNAL(pressed()), this, _SLOT(showMenu()));
}
/*!
    Returns the associated menu, or nullptr if no menu has been defined.
*/
Menu *ComboBox::menu() const {
    return static_cast<Menu *>(subWidget(gMenuFrame));
}
/*!
    Associates the given \a menu with this tool button.
    Ownership of the menu is not transferred to the tool button.
*/
void ComboBox::setMenu(Menu *menu) {
    setSubWidget(menu);

    if(menu) {
        connect(menu, _SIGNAL(aboutToHide()), this, _SLOT(hideMenu()));
        connect(menu, _SIGNAL(triggered(int)), this, _SLOT(onTriggered(int)));
        menu->setAnchors(Vector2(0.0f), Vector2(0.0f));
    }
}
/*!
    Shows the associated popup menu.
    Does nothing if no menu is associated.
*/
void ComboBox::showMenu() {
    Menu *menu = ComboBox::menu();
    if(menu) {
        setCorners(Vector4(0, 0, gCorner, gCorner));

        menu->setSize(Vector2(this->size().x, menu->size().y));
        menu->show(Vector2());
        Vector4 corners = menu->corners();
        corners.z = corners.w = 0.0f;
        menu->setCorners(corners);
    }
}
/*!
    Hides the associated popup menu.
*/
void ComboBox::hideMenu() {
    setCorners(Vector4(gCorner));

    Menu *menu = ComboBox::menu();
    if(menu) {
        menu->hide();
    }
}
/*!
    \internal
    Overrides the composeComponent method to create the tool button component.
*/
void ComboBox::composeComponent() {
    Button::composeComponent();

    // Add icon
    Image *icon = Widget::createWidget<Image>(gIcon, this);
    icon->setSprite(Engine::loadResource<Sprite>(".engine/ui.png/Arrow"));
    icon->setSize(Vector2(16.0f, 8.0f));
    icon->setAnchors(Vector2(1.0f, 0.5f), Vector2(1.0f, 0.5f));
    icon->setPivot(Vector2(1.0f, 0.5f));

    Menu *menu = Widget::createWidget<Menu>(gMenuFrame, this);
    setMenu(menu);

    menu->addAction("Menu Item 1");
    menu->addAction("Menu Item 2");
    menu->addAction("Menu Item 3");
}
/*!
    \internal
    Slot function to handle the triggered signal from the associated menu. Updates the current item text.
*/
void ComboBox::onTriggered(int index) {
    Menu *menu = ComboBox::menu();
    if(menu) {
        m_currentItem = menu->itemText(index);
        setText(m_currentItem);
    }
}
