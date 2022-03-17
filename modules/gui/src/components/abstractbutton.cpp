#include "components/abstractbutton.h"

#include "components/recttransform.h"
#include "components/image.h"

#include <components/actor.h>

#include <resources/sprite.h>

#include <input.h>
#include <timer.h>
#include <log.h>

namespace  {
const char *TARGET = "TargetGraphic";
}

class AbstractButtonPrivate {
public:
    AbstractButtonPrivate() :
        m_Hovered(false),
        m_fadeDuration(0.2f),
        m_currentFade(1.0f),
        m_targetGraphic(nullptr),
        m_normalColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
        m_highlightedColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f)),
        m_pressedColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f)) {

    }

    bool m_Hovered;
    float m_fadeDuration;
    float m_currentFade;
    Image *m_targetGraphic;
    Vector4 m_normalColor;
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;
};

AbstractButton::AbstractButton() :
    Widget(),
    p_ptr(new AbstractButtonPrivate) {

}

AbstractButton::~AbstractButton() {
    delete p_ptr;
    p_ptr = nullptr;
}

float AbstractButton::fadeDuration() const {
    return p_ptr->m_fadeDuration;
}

void AbstractButton::setFadeDuration(float duration) {
    p_ptr->m_fadeDuration = duration;
}

Vector4 AbstractButton::highlightedColor() const {
    return p_ptr->m_highlightedColor;
}

void AbstractButton::setHighlightedColor(const Vector4 color) {
    p_ptr->m_highlightedColor = color;
}

Vector4 AbstractButton::normalColor() const {
    return p_ptr->m_normalColor;
}

void AbstractButton::setNormalColor(const Vector4 color) {
    p_ptr->m_normalColor = color;
    if(p_ptr->m_targetGraphic) {
        p_ptr->m_targetGraphic->setColor(p_ptr->m_normalColor);
    }
}

Vector4 AbstractButton::pressedColor() const {
    return p_ptr->m_pressedColor;
}

void AbstractButton::setPressedColor(const Vector4 color) {
    p_ptr->m_pressedColor = color;
}

Image *AbstractButton::targetGraphic() const {
    return p_ptr->m_targetGraphic;
}

void AbstractButton::setTargetGraphic(Image *image) {
    if(p_ptr->m_targetGraphic != image) {
        disconnect(p_ptr->m_targetGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        p_ptr->m_targetGraphic = image;
        if(p_ptr->m_targetGraphic) {
            connect(p_ptr->m_targetGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            p_ptr->m_targetGraphic->setColor(p_ptr->m_normalColor);
        }
    }
}
/*!
    \internal
*/
void AbstractButton::onReferenceDestroyed() {
    Object *object = sender();
    if(p_ptr->m_targetGraphic == object) {
        p_ptr->m_targetGraphic = nullptr;
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

    Actor *a = actor();
    if(a) {
        RectTransform *t = dynamic_cast<RectTransform *>(a->transform());
        if(t) {
            Vector4 color(p_ptr->m_normalColor);

            bool hover = t->isHovered(pos.x, pos.y);
            if(p_ptr->m_Hovered != hover) {
                p_ptr->m_currentFade = 0.0f;
                p_ptr->m_Hovered = hover;
            }

            if(p_ptr->m_Hovered) {
                color = p_ptr->m_highlightedColor;
                if(Input::isMouseButtonDown(0) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
                    p_ptr->m_currentFade = 0.0f;
                    onClicked();
                }

                if(Input::isMouseButtonUp(0)) {
                    p_ptr->m_currentFade = 0.0f;
                }

                if(Input::isMouseButton(0) || Input::touchCount() > 0) {
                    color = p_ptr->m_pressedColor;
                }
            }

            if(p_ptr->m_targetGraphic) {
                if(p_ptr->m_currentFade < 1.0f) {
                    p_ptr->m_currentFade += 1.0f / p_ptr->m_fadeDuration * Timer::deltaTime();
                    p_ptr->m_currentFade = CLAMP(p_ptr->m_currentFade, 0.0f, 1.0f);

                    p_ptr->m_targetGraphic->setColor(MIX(p_ptr->m_targetGraphic->color(), color, p_ptr->m_currentFade));
                }
            }
        }
    }

    Widget::update();
}

void AbstractButton::onClicked() {
    emitSignal(_SIGNAL(clicked()));
}

/*!
    \internal
*/
void AbstractButton::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(TARGET);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setTargetGraphic(dynamic_cast<Image *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap AbstractButton::saveUserData() const {
    VariantMap result = Widget::saveUserData();
    {
        if(p_ptr->m_targetGraphic) {
            result[TARGET] = int(p_ptr->m_targetGraphic->uuid());
        }
    }
    return result;
}

void AbstractButton::composeComponent() {
    Widget::composeComponent();

    Image *image = Engine::objectCreate<Image>("Image", actor());
    image->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
    image->setItem("Rectangle");
    setTargetGraphic(image);
}
