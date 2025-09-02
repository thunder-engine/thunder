#include "components/menu.h"

#include "components/recttransform.h"
#include "components/label.h"
#include "components/layout.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <input.h>

#include <stdint.h>

namespace {
    const char *gSelected = "selected";

    const char *gFrame = "Frame";
    const char *gLabel = "Label";

    const float gCorner = 4.0f;
    const float gRowHeight = 20.0f;
};

/*!
    \class Menu
    \brief The Menu class represents a graphical user interface menu with options and actions.
    \inmodule Gui

    The Menu class represents a graphical user interface (GUI) menu that contains a list of options or actions that users can select.
    It provides a way to organize and display commands or choices in a structured manner, typically within a dropdown or context menu format.
    Menus are essential in GUI design for providing users with accessible options and actions within an application.
*/

Menu::Menu() :
        m_visible(false) {

}
/*!
    Adds a section to the menu with the specified \a text.
*/
void Menu::addSection(const TString &text) {
    Actor *actor = Engine::composeActor(gLabel, text, Menu::actor());
    Label *label = static_cast<Label *>(actor->component(gLabel));
    if(label) {
        label->setText(text);
        label->setAlign(Alignment::Middle | Alignment::Left);

        RectTransform *labelRect = label->rectTransform();
        if(labelRect) {
            labelRect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
            labelRect->setSize(Vector2(gRowHeight));
            labelRect->setMargin(Vector4(gRowHeight, 0.0f, 0.0f, 0.0f));
            labelRect->setPivot(Vector2(0.0f, 1.0f));
        }

        addWidget(label);
    }
}
/*!
    Adds a \a widget to the menu.
*/
void Menu::addWidget(Widget *widget) {
    Layout *layout = rectTransform()->layout();
    if(layout) {
        layout->addTransform(widget->rectTransform());
    }
    m_actions.push_back(widget);
}
/*!
    Returns the selection frame for the menu;
*/
Frame *Menu::selected() const {
    return static_cast<Frame *>(subWidget(gSelected));
}
/*!
    Sets the selection \a frame for the menu;
*/
void Menu::setSelected(Frame *frame) {
    setSubWidget(frame);
}
/*!
    Returns the title of the menu.
*/
TString Menu::title() const {
    return m_title;
}
/*!
    Sets the \a title of the menu.
*/
void Menu::setTitle(const TString &title) {
    m_title = title;
}
/*!
    Displays the menu at the specified \a position.
*/
void Menu::show(const Vector2 &position) {
    emitSignal(_SIGNAL(aboutToShow()));
    actor()->setEnabled(true);
    rectTransform()->setPosition(Vector3(position, 0.0f));
    m_visible = true;
}
/*!
    Hides the menu.
*/
void Menu::hide() {
    if(m_visible) {
        m_visible = false;
        emitSignal(_SIGNAL(aboutToHide()));
    }
    actor()->setEnabled(false);
}
/*!
    Returns the text of the item at the specified \a index.
*/
TString Menu::itemText(int index) {
    auto it = std::next(m_actions.begin(), index);
    Label *label = dynamic_cast<Label *>(*it);
    if(label) {
        return label->text();
    }

    return TString();
}
/*!
    \internal
    Updates the menu. Handles input and triggers actions based on user interactions.
*/
void Menu::update() {
    if(m_visible) {
        Vector4 pos = Input::mousePosition();
        if(Input::touchCount() > 0) {
            pos = Input::touchPosition(0);
        }

        bool hover = rectTransform()->isHovered(pos.x, pos.y);
        if(!hover && Input::isMouseButtonUp(0)) {
            hide();
        } else {
            int index = 0;
            for(auto it : m_actions) {
                hover = it->rectTransform()->isHovered(pos.x, pos.y);
                if(hover) {
                    float y = it->rectTransform()->position().y;

                    Frame *select = selected();
                    if(select) {
                        RectTransform *r = select->rectTransform();
                        if(r) {
                            r->setPosition(Vector3(0.0f, y, 0.0f));
                            r->setSize(Vector2(0.0f, it->rectTransform()->size().y));
                        }
                    }
                    if(Input::isMouseButtonDown(0)) {
                        emitSignal(_SIGNAL(triggered(int)), index);
                        hide();
                    }
                    break;
                }
                ++index;
            }
        }
    }

    Widget::update();
}
/*!
    \internal
    Composes the menu components and sets initial properties.
*/
void Menu::composeComponent() {
    Frame::composeComponent();

    setColor(Vector4(0.376f, 0.376f, 0.376f, 1.0f));
    setCorners(Vector4(gCorner));
    rectTransform()->setPivot(Vector2(0.0f, 1.0f));

    Layout *layout = new Layout;
    Vector4 c = corners();
    RectTransform *r = rectTransform();
    if(r) {
        r->setLayout(layout);
    }

    Actor *actor = Engine::composeActor(gFrame, gSelected, Menu::actor());
    Frame *select = static_cast<Frame *>(actor->component(gFrame));
    select->setColor(Vector4(0.01f, 0.6f, 0.89f, 1.0f));
    select->setCorners(0.0f);
    select->setBorderColor(0.0f);

    r = select->rectTransform();
    if(r) {
        r->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
        r->setPivot(Vector2(0.0f, 1.0f));
    }

    setSelected(select);

    hide();
}
