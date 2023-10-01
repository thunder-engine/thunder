#include "components/gui/menu.h"

#include "components/actor.h"
#include "components/textrender.h"

#include "components/gui/recttransform.h"
#include "components/gui/label.h"
#include "components/gui/layout.h"

#include "input.h"

#include <stdint.h>

namespace {
const char *gFrame = "Frame";
const char *gLabel = "Label";

const float gCorner = 4.0f;
const float gRowHeight = 20.0f;
};

Menu::Menu() :
        m_select(nullptr),
        m_visible(false) {

}

void Menu::addSection(const string &text) {
    Actor *actor = Engine::composeActor(gLabel, text, Menu::actor());
    Label *label = static_cast<Label *>(actor->component(gLabel));
    if(label) {
        label->setText(text);
        label->setAlign(Alignment::Middle | Alignment::Left);

        RectTransform *labelRect = label->rectTransform();
        if(labelRect) {
            labelRect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
            labelRect->setSize(Vector2(gRowHeight));
            labelRect->setOffsetMax(Vector2(gRowHeight, 0.0f));
            labelRect->setPivot(Vector2(0.0f, 1.0f));
        }
        addWidget(label);
    }
}

void Menu::addWidget(Widget *widget) {
    Layout *layout = rectTransform()->layout();
    if(layout) {
        layout->addWidget(widget);
        layout->update();

        Vector2 size = layout->sizeHint();
        rectTransform()->setSize(size);
    }
    m_actions.push_back(widget);
}

string Menu::title() const {
    return m_title;
}
void Menu::setTitle(const string &title) {
    m_title = title;
}

void Menu::show(const Vector2 &position) {
    emitSignal(_SIGNAL(aboutToShow()));
    actor()->setEnabled(true);
    rectTransform()->setPosition(Vector3(position, 0.0f));
    m_visible = true;
}

void Menu::hide() {
    if(m_visible) {
        m_visible = false;
        emitSignal(_SIGNAL(aboutToHide()));
    }
    actor()->setEnabled(false);
}

string Menu::itemText(int index) {
    auto it = std::next(m_actions.begin(), index);
    Label *label = dynamic_cast<Label *>(*it);
    if(label) {
        return label->text();
    }

    return string();
}

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
                    RectTransform *r = m_select->rectTransform();
                    if(r) {
                        r->setPosition(Vector3(0.0f, y, 0.0f));
                        r->setSize(Vector2(0.0f, it->rectTransform()->size().y));
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

void Menu::composeComponent() {
    Frame::composeComponent();

    setColor(Vector4(0.376f, 0.376f, 0.376f, 1.0f));
    setCorners(Vector4(gCorner));
    rectTransform()->setPivot(Vector2(0.0f, 1.0f));

    Layout *layout = new Layout;
    Vector4 c = corners();
    layout->setMargins(c.x, 0.0f, c.z, c.w);
    RectTransform *r = rectTransform();
    if(r) {
        r->setLayout(layout);
    }

    Actor *actor = Engine::composeActor(gFrame, gFrame, Menu::actor());
    m_select = static_cast<Frame *>(actor->component(gFrame));
    m_select->setColor(Vector4(0.01f, 0.6f, 0.89f, 1.0f));
    m_select->setCorners(0.0f);
    m_select->setBorderColor(0.0f);
    r = m_select->rectTransform();
    if(r) {
        r->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
        r->setPivot(Vector2(0.0f, 1.0f));
    }

    hide();
}
