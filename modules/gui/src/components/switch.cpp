#include "components/switch.h"

#include "components/image.h"
#include "components/recttransform.h"

#include <components/actor.h>

#include <resources/sprite.h>

#include <timer.h>

namespace  {
    const char *KNOB = "Knob";
}

class SwitchPrivate {
public:
    SwitchPrivate() :
        m_knobColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
        m_knobGraphic(nullptr),
        m_switchDuration(0.2f),
        m_currentFade(1.0f),
        m_isOn(false) {

    }

    Vector4 m_knobColor;
    Image *m_knobGraphic;
    float m_switchDuration;
    float m_currentFade;
    bool m_isOn;
};

Switch::Switch() :
    AbstractButton(),
    p_ptr(new SwitchPrivate) {

}

Switch::~Switch() {
    delete p_ptr;
    p_ptr = nullptr;

}

float Switch::switchDuration() const {
    return p_ptr->m_switchDuration;
}
void Switch::setSwitchDuration(float duration) {
    p_ptr->m_switchDuration = duration;
}

Image *Switch::knobGraphic() const {
    return p_ptr->m_knobGraphic;
}

void Switch::setKnobGraphic(Image *image) {
    if(p_ptr->m_knobGraphic != image) {
        disconnect(p_ptr->m_knobGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        p_ptr->m_knobGraphic = image;
        if(p_ptr->m_knobGraphic) {
            connect(p_ptr->m_knobGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            p_ptr->m_knobGraphic->setColor(p_ptr->m_knobColor);
        }
    }
}

Vector4 Switch::knobColor() const {
    return p_ptr->m_knobColor;
}
void Switch::setKnobColor(const Vector4 &color) {
    p_ptr->m_knobColor = color;
    if(p_ptr->m_knobGraphic) {
        p_ptr->m_knobGraphic->setColor(p_ptr->m_knobColor);
    }
}

bool Switch::isOn() const {
    return p_ptr->m_isOn;
}

void Switch::update() {
    AbstractButton::update();

    if(p_ptr->m_currentFade < 1.0f && p_ptr->m_knobGraphic) {
        p_ptr->m_currentFade += 1.0f / p_ptr->m_switchDuration * Timer::deltaTime();
        p_ptr->m_currentFade = CLAMP(p_ptr->m_currentFade, 0.0f, 1.0f);

        Actor *a = p_ptr->m_knobGraphic->actor();
        RectTransform *t = dynamic_cast<RectTransform *>(a->transform());
        RectTransform *p = dynamic_cast<RectTransform *>(actor()->transform());
        if(t && p) {
            Vector3 pos(t->position());
            if(p_ptr->m_isOn) {
                pos.x = MIX(0.0f, p->size().x * 0.5f, p_ptr->m_currentFade);
                t->setMinAnchors(Vector2(0.5f, 0.0f));
                t->setMaxAnchors(Vector2(1.0f, 1.0f));
            } else {
                pos.x = MIX(p->size().x * 0.5f, 0.0f, p_ptr->m_currentFade);
                t->setMinAnchors(Vector2(0.0f));
                t->setMaxAnchors(Vector2(0.5f, 1.0f));
            }
            t->setPosition(pos);
        }
    }
}

void Switch::onClicked() {
    AbstractButton::onClicked();

    p_ptr->m_isOn = !p_ptr->m_isOn;
    p_ptr->m_currentFade = 0.0f;
}
/*!
    \internal
*/
void Switch::loadUserData(const VariantMap &data) {
    AbstractButton::loadUserData(data);
    {
        auto it = data.find(KNOB);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setKnobGraphic(dynamic_cast<Image *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap Switch::saveUserData() const {
    VariantMap result = AbstractButton::saveUserData();
    {
        if(p_ptr->m_knobGraphic) {
            result[KNOB] = int(p_ptr->m_knobGraphic->uuid());
        }
    }
    return result;
}
/*!
    \internal
*/
void Switch::composeComponent() {
    AbstractButton::composeComponent();

    RectTransform *parent = dynamic_cast<RectTransform *>(actor()->transform());
    if(parent) {
        parent->setSize(Vector2(40.0f, 20.0f));
    }
    // Add knob
    Actor *knob = Engine::composeActor("Image", "Knob", actor());
    Image *image = static_cast<Image *>(knob->component("Image"));
    image->setParent(knob);
    image->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
    image->setItem("Rectangle");
    setKnobGraphic(image);

    RectTransform *t = dynamic_cast<RectTransform *>(knob->transform());
    if(t && parent) {
        Vector2 size = parent->size();
        size.x *= 0.5f;
        t->setMinAnchors(Vector2(0.0f));
        t->setSize(size);
    }
}
/*!
    \internal
*/
void Switch::onReferenceDestroyed() {
    AbstractButton::onReferenceDestroyed();

    Object *object = sender();
    if(p_ptr->m_knobGraphic == object) {
        p_ptr->m_knobGraphic = nullptr;
    }
}
