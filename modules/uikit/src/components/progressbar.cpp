#include "components/progressbar.h"

#include "components/recttransform.h"
#include "components/image.h"

#include <components/actor.h>

namespace  {
    const char *gProgress = "Progress";
    const char *gBackground = "Background";
    const char *gFrame = "Frame";
}

/*!
    \class ProgressBar
    \brief The ProgressBar class represents a graphical user interface element that displays progress visually.
    \inmodule Gui

    The ProgressBar class is designed to provide a graphical representation of progress with customizable appearance and range.
    It supports features such as setting the minimum and maximum values, adjusting the progress value, and specifying visual elements for background and progress indicator.
*/

ProgressBar::ProgressBar() :
        m_backgroundColor(0.5f, 0.5f, 0.5f, 1.0f),
        m_progressColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_progress(nullptr),
        m_background(nullptr),
        m_from(0.0f),
        m_to(1.0f),
        m_value(0.0f) {

}
/*!
    Returns the minimum value of the progress range.
*/
float ProgressBar::from() const {
    return m_from;
}
/*!
    Sets the minimum \a value of the progress range.
*/
void ProgressBar::setFrom(float value) {
    m_from = value;

    recalcProgress();
}
/*!
    Returns the maximum value of the progress range.
*/
float ProgressBar::to() const {
    return m_to;
}
/*!
    Sets the maximum \a value of the progress range.
*/
void ProgressBar::setTo(float value) {
    m_to = value;

    recalcProgress();
}
/*!
    Returns the current progress value.
*/
float ProgressBar::value() const {
    return m_value;
}
/*!
    Sets the current progress \a value.
*/
void ProgressBar::setValue(float value) {
    m_value = value;

    recalcProgress();
}
/*!
    Returns the frame representing the progress bar.
*/
Frame *ProgressBar::progress() const {
    return m_progress;
}
/*!
    Sets the \a frame representing the progress.
*/
void ProgressBar::setProgress(Frame *frame) {
    if(m_progress != frame) {
        disconnect(m_progress, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_progress = frame;
        if(m_progress) {
            connect(m_progress, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_progress->setColor(m_progressColor);

            recalcProgress();
        }
    }
}
/*!
    Returns the frame representing the background.
*/
Frame *ProgressBar::background() const {
    return m_background;
}
/*!
    Sets the \a frame representing the background.
*/
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
/*!
    Returns the background color of the progress bar.
*/
Vector4 ProgressBar::backgroundColor() const {
    return m_backgroundColor;
}
/*!
    Sets the background \a color of the progress bar.
*/
void ProgressBar::setBackgroundColor(const Vector4 color) {
    m_backgroundColor = color;
    if(m_background) {
        m_background->setColor(m_backgroundColor);
    }
}
/*!
    Returns the color of the progress indicator.
*/
Vector4 ProgressBar::progressColor() const {
    return m_progressColor;
}
/*!
    Sets the \a color of the progress indicator.
*/
void ProgressBar::setProgressColor(const Vector4 color) {
    m_progressColor = color;
    if(m_progress) {
        m_progress->setColor(m_progressColor);
    }
}
/*!
    \internal
    Loads user \a data for the progress bar.
*/
void ProgressBar::loadUserData(const VariantMap &data) {
    Widget::loadUserData(data);

    auto it = data.find(gProgress);
    if(it != data.end()) {
        uint32_t uuid = uint32_t((*it).second.toInt());
        Object *object = Engine::findObject(uuid);
        setProgress(dynamic_cast<Frame *>(object));
    }

    it = data.find(gBackground);
    if(it != data.end()) {
        uint32_t uuid = uint32_t((*it).second.toInt());
        Object *object = Engine::findObject(uuid);
        setBackground(dynamic_cast<Frame *>(object));
    }
}
/*!
    \internal
     Saves user data for the progress bar.
*/
VariantMap ProgressBar::saveUserData() const {
    VariantMap result = Widget::saveUserData();

    if(m_progress) {
        result[gProgress] = int(m_progress->uuid());
    }

    if(m_background) {
        result[gBackground] = int(m_background->uuid());
    }

    return result;
}
/*!
    \internal
    Composes the components of the progress bar and sets initial properties.
*/
void ProgressBar::composeComponent() {
    Widget::composeComponent();

    Actor *background = Engine::composeActor(gFrame, gBackground, actor());
    Frame *backgroundFrame = static_cast<Frame *>(background->component(gFrame));
    backgroundFrame->setColor(m_backgroundColor);
    backgroundFrame->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));

    setBackground(backgroundFrame);

    Actor *progress = Engine::composeActor(gFrame, gProgress, background);
    Frame *progressFrame = static_cast<Frame *>(progress->component(gFrame));
    progressFrame->setColor(m_progressColor);
    progressFrame->setBorderColor(0.0f);
    progressFrame->rectTransform()->setMinAnchors(Vector2(0.0f, 0.0f));

    setProgress(progressFrame);

    setValue(0.5f);

    rectTransform()->setSize(Vector2(100.0f, 30.0f));
}
/*!
    \internal
    Slot for handling destruction of referenced objects.
*/
void ProgressBar::onReferenceDestroyed() {
    Object *object = sender();

    if(m_progress == object) {
        m_progress = nullptr;
    } else if(m_background == object) {
        m_background = nullptr;
    }
}
/*!
    \internal
    Recalculates the progress based on the current values.
*/
void ProgressBar::recalcProgress() {
    if(m_progress) {
        m_progress->rectTransform()->setMaxAnchors(Vector2(CLAMP((m_from - m_value) / (m_from - m_to), 0.0f, 1.0f), 1.0f));
    }
}
