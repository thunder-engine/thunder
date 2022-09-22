#include "components/abstractbutton.h"

#include "components/recttransform.h"
#include "components/frame.h"
#include "components/label.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <input.h>
#include <timer.h>
#include <log.h>

namespace  {
    const char *gTarget = "TargetGraphic";
    const char *gLabel = "Label";
    const char *gBackground = "Frame";
}

AbstractButton::AbstractButton() :
    Widget(),
    m_normalColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
    m_highlightedColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f)),
    m_pressedColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f)),
    m_label(nullptr),
    m_background(nullptr),
    m_fadeDuration(0.2f),
    m_currentFade(1.0f),
    m_hovered(false),
    m_mirrored(false),
    m_checkable(false),
    m_checked(false),
    m_exclusive(false) {

}

AbstractButton::~AbstractButton() {

}

string AbstractButton::text() const {
    return m_text;
}
void AbstractButton::setText(const string text) {
    m_text = text;
    if(m_label) {
        m_label->setText(text);
    }
}

Frame *AbstractButton::background() const {
    return m_background;
}
void AbstractButton::setBackground(Frame *frame) {
    if(m_background != frame) {
        disconnect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_background = frame;
        if(m_background) {
            connect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_background->setColor(m_normalColor);
        }
    }
}

Label *AbstractButton::label() const {
    return m_label;
}
void AbstractButton::setLabel(Label *label) {
    if(m_label != label) {
        disconnect(m_label, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_label = label;
        if(m_label) {
            connect(m_label, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        }
    }
}

float AbstractButton::fadeDuration() const {
    return m_fadeDuration;
}
void AbstractButton::setFadeDuration(float duration) {
    m_fadeDuration = duration;
}

Vector4 AbstractButton::highlightedColor() const {
    return m_highlightedColor;
}
void AbstractButton::setHighlightedColor(const Vector4 color) {
    m_highlightedColor = color;
}

Vector4 AbstractButton::normalColor() const {
    return m_normalColor;
}
void AbstractButton::setNormalColor(const Vector4 color) {
    m_normalColor = color;
    if(m_background) {
        m_background->setColor(m_normalColor);
    }
}

Vector4 AbstractButton::pressedColor() const {
    return m_pressedColor;
}
void AbstractButton::setPressedColor(const Vector4 color) {
    m_pressedColor = color;
}

bool AbstractButton::isCheckable() const {
    return m_checkable;
}
void AbstractButton::setCheckable(bool checkable) {
    m_checkable = checkable;
}

bool AbstractButton::isChecked() const {
    return m_checked;
}
void AbstractButton::setChecked(bool checked) {
    m_checked = checked;
    checkStateSet();
}

bool AbstractButton::isExclusive() const {
    return m_exclusive;
}
void AbstractButton::setExclusive(bool exclusive) {
    m_exclusive = exclusive;
}

bool AbstractButton::isMirrored() const {
    return m_mirrored;
}
void AbstractButton::setMirrored(bool flag) {
    m_mirrored = flag;
}

/*!
    \internal
*/
void AbstractButton::onReferenceDestroyed() {
    Object *object = sender();
    if(m_background == object) {
        m_background = nullptr;
    }

    if(m_label == object) {
        m_label = nullptr;
    }
}
/*!
    \internal
*/
void AbstractButton::update() {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }
    Vector4 color(m_normalColor);

    bool hover = rectTransform()->isHovered(pos.x, pos.y);
    if(m_hovered != hover) {
        m_currentFade = 0.0f;
        m_hovered = hover;
    }

    if(m_hovered) {
        color = m_highlightedColor;
        if(Input::isMouseButtonDown(0) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;
            onClicked();
        }

        if(Input::isMouseButtonUp(0)) {
            m_currentFade = 0.0f;
        }

        if(Input::isMouseButton(0) || Input::touchCount() > 0) {
            color = m_pressedColor;
        }
    }

    if(m_background) {
        if(m_currentFade < 1.0f) {
            m_currentFade += 1.0f / m_fadeDuration * Timer::deltaTime();
            m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

            m_background->setColor(MIX(m_background->color(), color, m_currentFade));
        }
    }

    Widget::update();
}

void AbstractButton::onClicked() {
    if(m_checkable) {
        setChecked(!m_checked);
    }
    emitSignal(_SIGNAL(clicked()));
}

void AbstractButton::checkStateSet() {
    if(m_exclusive && m_checked) {
        for(auto it : actor()->getChildren()) {
            Actor *actor = dynamic_cast<Actor *>(it);
            if(actor) {
                AbstractButton *btn = static_cast<AbstractButton *>(actor->component("AbstractButton"));
                if(btn && btn != this) {
                    btn->setChecked(false);
                }
            }
        }
    }
}

/*!
    \internal
*/
void AbstractButton::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gTarget);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setBackground(dynamic_cast<Frame *>(object));
        }
        it = data.find(gLabel);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setLabel(dynamic_cast<Label *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap AbstractButton::saveUserData() const {
    VariantMap result = Widget::saveUserData();
    {
        if(m_background) {
            result[gTarget] = int(m_background->uuid());
        }
        if(m_label) {
            result[gLabel] = int(m_label->uuid());
        }
    }
    return result;
}

void AbstractButton::composeComponent() {
    Widget::composeComponent();

    // Add background
    Actor *background = Engine::composeActor(gBackground, "Background", actor());
    Frame *frame = static_cast<Frame *>(background->component(gBackground));
    frame->setCorners(Vector4(3.0f));
    setBackground(frame);

    // Add label
    Actor *text = Engine::composeActor(gLabel, gLabel, actor());
    Label *label = static_cast<Label *>(text->component(gLabel));
    label->setAlign(label->align() | Alignment::Center);
    setLabel(label);
    setText("Text");
}
