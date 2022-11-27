#include "components/gui/floatinput.h"

#include "components/gui/layout.h"
#include "components/gui/recttransform.h"
#include "components/gui/button.h"
#include "components/gui/textinput.h"
#include "components/gui/label.h"
#include "components/gui/frame.h"

#include "components/actor.h"

#include "resources/sprite.h"

#include <sstream>

namespace {
    const char *gTextInput("TextInput");
    const char *gButton("Button");
};

FloatInput::FloatInput() :
    m_increaseBtn(nullptr),
    m_decreaseBtn(nullptr),
    m_input(nullptr),
    m_value(0.0f),
    m_singleStep(1.0f),
    m_minimum(0.0f),
    m_maximum(99.99f) {
}

float FloatInput::value() const {
    return m_value;
}
void FloatInput::setValue(float value) {
    m_value = CLAMP(value, m_minimum, m_maximum);

    std::ostringstream ss;
    ss << m_value;
    m_input->setText(ss.str());
}

float FloatInput::minimum() const {
    return m_minimum;
}
void FloatInput::setMinimum(float minimum) {
    m_minimum = minimum;
}

float FloatInput::maximum() const {
    return m_maximum;
}
void FloatInput::setMaximum(float maximum) {
    m_maximum = maximum;
}

float FloatInput::singleStep() const {
    return m_singleStep;
}
void FloatInput::setSingleStep(float step) {
    m_singleStep = step;
    if(m_increaseBtn) {
        m_increaseBtn->actor()->setEnabled(m_singleStep != 0.0f);
    }
    if(m_decreaseBtn) {
        m_decreaseBtn->actor()->setEnabled(m_singleStep != 0.0f);
    }
}

Vector4 FloatInput::corners() const {
    return m_cornerRadius;
}
void FloatInput::setCorners(Vector4 corners) {
    m_cornerRadius = corners;

    if(m_decreaseBtn) {
        Frame *frame = m_decreaseBtn->background();
        if(frame) {
            Vector4 corners = m_cornerRadius;
            corners.y = corners.z = 0.0f;
            frame->setCorners(corners);
        }
    }

    if(m_increaseBtn) {
        Frame *frame = m_increaseBtn->background();
        if(frame) {
            Vector4 corners = m_cornerRadius;
            corners.x = corners.w = 0.0f;
            frame->setCorners(corners);
        }
    }
}

void FloatInput::onIncrease() {
    setValue(m_value + m_singleStep);
}
void FloatInput::onDecrease() {
    setValue(m_value - m_singleStep);
}

void FloatInput::onEditingFinished() {
    string text = m_input->text();
    if(!text.empty()) {
        setValue(stof(text));
    } else {
        setValue(value());
    }
}

void FloatInput::composeComponent() {
    Widget::composeComponent();

    const float width = 20.0f;

    Actor *text = Engine::composeActor(gTextInput, gTextInput, actor());
    m_input = static_cast<TextInput *>(text->component(gTextInput));
    if(m_input) {
        connect(m_input, _SIGNAL(focusOut()), this, _SLOT(onEditingFinished()));
        connect(m_input, _SIGNAL(editingFinished()), this, _SLOT(onEditingFinished()));

        Frame *frame = m_input->background();
        if(frame) {
            frame->setCorners(Vector4());
        }

        setValue(m_value);
    }
    RectTransform *textRect = static_cast<RectTransform *>(text->transform());
    textRect->setSize(Vector2(0.0f));
    textRect->setAnchors(Vector2(0.0f), Vector2(1.0f));
    textRect->setOffsets(Vector2(width, 0.0f), Vector2(width, 0.0f));

    Sprite *arrow = Engine::loadResource<Sprite>(".embedded/ui.png");

    Actor *left = Engine::composeActor(gButton, "Decrease", actor());
    m_decreaseBtn = static_cast<Button *>(left->component(gButton));
    if(m_decreaseBtn) {
        connect(m_decreaseBtn, _SIGNAL(clicked()), this, _SLOT(onDecrease()));
        m_decreaseBtn->setText("");
        m_decreaseBtn->setIconSize(Vector2(16.0f, 8.0f));
        Image *icon = m_decreaseBtn->icon();
        if(icon) {
            icon->setSprite(arrow);
            icon->setItem("Arrow");
            RectTransform *rect = icon->rectTransform();
            if(rect) {
                rect->setRotation(Vector3(0.0f, 0.0f,-90.0f));
            }
        }
    }
    RectTransform *leftRect = static_cast<RectTransform *>(left->transform());
    leftRect->setSize(Vector2(width));
    leftRect->setAnchors(Vector2(0.0f), Vector2(0.0f));
    leftRect->setPivot(Vector2(0.0f));

    Actor *right = Engine::composeActor(gButton, "Increase", actor());
    m_increaseBtn = static_cast<Button *>(right->component(gButton));
    if(m_increaseBtn) {
        connect(m_increaseBtn, _SIGNAL(clicked()), this, _SLOT(onIncrease()));
        m_increaseBtn->setText("");
        m_increaseBtn->setIconSize(Vector2(16.0f, 8.0f));
        Image *icon = m_increaseBtn->icon();
        if(icon) {
            icon->setSprite(arrow);
            icon->setItem("Arrow");
            RectTransform *rect = icon->rectTransform();
            if(rect) {
                rect->setRotation(Vector3(0.0f, 0.0f, 90.0f));
            }
        }
    }
    RectTransform *rightRect = static_cast<RectTransform *>(right->transform());
    rightRect->setSize(Vector2(width, 0.0f));
    rightRect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0f));
    rightRect->setPivot(Vector2(1.0f, 1.0f));

    rectTransform()->setSize(Vector2(100.0f, 30.0f));

    setCorners(Vector4(4.0f));
}
