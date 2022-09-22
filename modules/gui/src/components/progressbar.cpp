#include "components/progressbar.h"
#include "components/frame.h"
#include "components/recttransform.h"

#include <components/actor.h>

namespace  {
    const char *gBackground = "Background";
    const char *gProgress = "Progress";
    const char *gFrame = "Frame";
}

ProgressBar::ProgressBar() :
    Widget(),
    m_backgroundColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
    m_progressColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_from(0.0f),
    m_to(1.0f),
    m_value(0.0f),
    m_background(nullptr),
    m_progress(nullptr) {

}

ProgressBar::~ProgressBar() {

}

float ProgressBar::from() const {
    return m_from;
}
void ProgressBar::setFrom(float value) {
    m_from = value;

    recalcProgress();
}

float ProgressBar::to() const {
    return m_to;
}
void ProgressBar::setTo(float value) {
    m_to = value;

    recalcProgress();
}

float ProgressBar::value() const {
    return m_value;
}
void ProgressBar::setValue(float value) {
    m_value = value;

    recalcProgress();
}

Frame *ProgressBar::background() const {
    return m_background;
}
void ProgressBar::setBackground(Frame *frame) {
    if(m_background != frame) {
        disconnect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_background = frame;
        if(m_background) {
            connect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_background->setColor(m_backgroundColor);
        }
    }
}

Frame *ProgressBar::progress() const {
    return m_progress;
}
void ProgressBar::setProgress(Frame *image) {
    if(m_progress != image) {
        disconnect(m_progress, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_progress = image;
        if(m_progress) {
            connect(m_progress, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_progress->setColor(m_progressColor);

            recalcProgress();
        }
    }
}

Vector4 ProgressBar::backgroundColor() const {
    return m_backgroundColor;
}
void ProgressBar::setBackgroundColor(const Vector4 color) {
    m_backgroundColor = color;
    if(m_background) {
        m_background->setColor(m_backgroundColor);
    }
}

Vector4 ProgressBar::progressColor() const {
    return m_progressColor;
}
void ProgressBar::setProgressColor(const Vector4 color) {
    m_progressColor = color;
    if(m_progress) {
        m_progress->setColor(m_progressColor);
    }
}
/*!
    \internal
*/
void ProgressBar::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gBackground);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setBackground(dynamic_cast<Frame *>(object));
        }
    }
    {
        auto it = data.find(gProgress);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setProgress(dynamic_cast<Frame *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap ProgressBar::saveUserData() const {
    VariantMap result = Widget::saveUserData();
    {
        if(m_background) {
            result[gBackground] = int(m_background->uuid());
        }
    }
    {
        if(m_progress) {
            result[gProgress] = int(m_progress->uuid());
        }
    }
    return result;
}
/*!
    \internal
*/
void ProgressBar::composeComponent() {
    Widget::composeComponent();

    Frame *frame = Engine::objectCreate<Frame>(gFrame, actor());
    frame->setCorners(Vector4(3.0f));
    setBackground(frame);

    Actor *progress = Engine::composeActor(gFrame, "Progress", actor());
    frame = static_cast<Frame *>(progress->component(gFrame));
    frame->setCorners(Vector4(3.0f));
    frame->rectTransform()->setMinAnchors(Vector2(0.0f, 0.0f));
    setProgress(frame);

    setValue(0.5f);

    rectTransform()->setSize(Vector2(100.0f, 30.0f));
}
/*!
    \internal
*/
void ProgressBar::onReferenceDestroyed() {
    Object *object = sender();
    if(m_background == object) {
        m_background = nullptr;
        return;
    }

    if(m_progress == object) {
        m_progress = nullptr;
        return;
    }
}

void ProgressBar::recalcProgress() {
    if(m_progress) {
        m_progress->rectTransform()->setMaxAnchors(Vector2(CLAMP((m_from - m_value) / (m_from - m_to), 0.0f, 1.0f), 1.0f));
    }
}
