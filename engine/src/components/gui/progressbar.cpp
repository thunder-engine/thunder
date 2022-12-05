#include "components/gui/progressbar.h"
#include "components/gui/recttransform.h"

#include "components/actor.h"

namespace  {
    const char *gProgress = "Progress";
    const char *gFrame = "Frame";
}

ProgressBar::ProgressBar() :
    Frame(),
    m_backgroundColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
    m_progressColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_from(0.0f),
    m_to(1.0f),
    m_value(0.0f),
    m_progress(nullptr) {

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
    setColor(m_backgroundColor);
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
    Image::loadUserData(data);
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
    VariantMap result = Image::saveUserData();
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

    setCorners(Vector4(3.0f));
    setColor(m_backgroundColor);

    Actor *progress = Engine::composeActor(gFrame, "Progress", actor());
    Frame *frame = static_cast<Frame *>(progress->component(gFrame));
    frame->setCorners(corners());
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

    if(m_progress == object) {
        m_progress = nullptr;
        return;
    }
}
/*!
    \internal
*/
void ProgressBar::recalcProgress() {
    if(m_progress) {
        m_progress->rectTransform()->setMaxAnchors(Vector2(CLAMP((m_from - m_value) / (m_from - m_to), 0.0f, 1.0f), 1.0f));
    }
}
