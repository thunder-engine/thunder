#include "components/progressbar.h"
#include "components/image.h"
#include "components/recttransform.h"

#include <components/actor.h>

#include <resources/sprite.h>

namespace  {
    const char *BACKGROUND = "BackgroundGraphic";
    const char *PROGRESS = "ProgressGraphic";
    const char *IMAGE = "Image";
    const char *PATH = ".embedded/ui.png";
}

class ProgressBarPrivate {
public:
    ProgressBarPrivate() :
        m_From(0.0f),
        m_To(1.0f),
        m_Value(0.0f),
        m_backgroundGraphic(nullptr),
        m_progressGraphic(nullptr),
        m_backgroundColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
        m_progressColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)) {

    }

    void recalcProgress() {
        if(m_progressGraphic && m_backgroundGraphic) {
            RectTransform *parent = static_cast<RectTransform *>(m_backgroundGraphic->actor()->transform());
            RectTransform *progress = static_cast<RectTransform *>(m_progressGraphic->actor()->transform());
            Vector2 size = parent->size();
            float f = CLAMP((m_From - m_Value) / (m_From - m_To), 0.0f, 1.0f);
            size.x *= f;
            progress->setSize(size);
            progress->setMaxAnchors(Vector2(f, 1.0f));
        }
    }

    float m_From;
    float m_To;
    float m_Value;

    Image *m_backgroundGraphic;
    Image *m_progressGraphic;

    Vector4 m_backgroundColor;
    Vector4 m_progressColor;
};

ProgressBar::ProgressBar() :
    Widget(),
    p_ptr(new ProgressBarPrivate) {

}

ProgressBar::~ProgressBar() {
    delete p_ptr;
}

float ProgressBar::from() const {
    return p_ptr->m_From;
}
void ProgressBar::setFrom(float value) {
    p_ptr->m_From = value;

    p_ptr->recalcProgress();
}

float ProgressBar::to() const {
    return p_ptr->m_To;
}
void ProgressBar::setTo(float value) {
    p_ptr->m_To = value;

    p_ptr->recalcProgress();
}

float ProgressBar::value() const {
    return p_ptr->m_Value;
}
void ProgressBar::setValue(float value) {
    p_ptr->m_Value = value;

    p_ptr->recalcProgress();
}

Image *ProgressBar::backgroundGraphic() const {
    return p_ptr->m_backgroundGraphic;
}
void ProgressBar::setBackgroundGraphic(Image *image) {
    p_ptr->m_backgroundGraphic = image;
    if(p_ptr->m_backgroundGraphic) {
        p_ptr->m_backgroundGraphic->setColor(p_ptr->m_backgroundColor);
    }
}

Image *ProgressBar::progressGraphic() const {
    return p_ptr->m_progressGraphic;
}
void ProgressBar::setProgressGraphic(Image *image) {
    p_ptr->m_progressGraphic = image;
    if(p_ptr->m_progressGraphic) {
        p_ptr->m_progressGraphic->setColor(p_ptr->m_progressColor);

        p_ptr->recalcProgress();
    }
}

Vector4 ProgressBar::backgroundColor() const {
    return p_ptr->m_backgroundColor;
}
void ProgressBar::setBackgroundColor(const Vector4 &color) {
    p_ptr->m_backgroundColor = color;
    if(p_ptr->m_backgroundGraphic) {
        p_ptr->m_backgroundGraphic->setColor(p_ptr->m_backgroundColor);
    }
}

Vector4 ProgressBar::progressColor() const {
    return p_ptr->m_progressColor;
}
void ProgressBar::setProgressColor(const Vector4 &color) {
    p_ptr->m_progressColor = color;
    if(p_ptr->m_progressGraphic) {
        p_ptr->m_progressGraphic->setColor(p_ptr->m_progressColor);
    }
}
/*!
    \internal
*/
void ProgressBar::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(BACKGROUND);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setBackgroundGraphic(dynamic_cast<Image *>(object));
        }
    }
    {
        auto it = data.find(PROGRESS);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setProgressGraphic(dynamic_cast<Image *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap ProgressBar::saveUserData() const {
    VariantMap result = Widget::saveUserData();
    {
        if(p_ptr->m_backgroundGraphic) {
            result[BACKGROUND] = int(p_ptr->m_backgroundGraphic->uuid());
        }
    }
    {
        if(p_ptr->m_progressGraphic) {
            result[PROGRESS] = int(p_ptr->m_progressGraphic->uuid());
        }
    }
    return result;
}

void ProgressBar::composeComponent() {
    Widget::composeComponent();

    {
        Image *image = Engine::objectCreate<Image>(IMAGE, actor());
        image->setSprite(Engine::loadResource<Sprite>(PATH));
        image->setItem("Rectangle");
        setBackgroundGraphic(image);
    }
    {
        Actor *progress = Engine::composeActor(IMAGE, "Progress", actor());
        Image *image = static_cast<Image *>(progress->component(IMAGE));
        image->setSprite(Engine::loadResource<Sprite>(PATH));
        image->setItem("Rectangle");
        setProgressGraphic(image);

        RectTransform *rect = dynamic_cast<RectTransform *>(progress->transform());
        if(rect) {
            rect->setMinAnchors(Vector2(0.0f, 0.0f));
        }
    }

    RectTransform *parent = dynamic_cast<RectTransform *>(actor()->transform());
    if(parent) {
        parent->setSize(Vector2(100.0f, 20.0f));
    }
}
