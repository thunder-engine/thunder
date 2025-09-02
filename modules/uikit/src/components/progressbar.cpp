#include "components/progressbar.h"

#include "components/recttransform.h"
#include "components/image.h"

#include <components/actor.h>

namespace  {
    const char *gChunk = "chunk";
    const char *gBackground = "background";

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
        m_orientation(Horizontal),
        m_from(0.0f),
        m_to(1.0f),
        m_value(0.0f) {

}
int ProgressBar::orientation() const {
    return m_orientation;
}

void ProgressBar::setOrientation(int orientation) {
    if(m_orientation != orientation) {
        m_orientation = orientation;

        recalcProgress();
    }
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
    if(m_from != value) {
        m_from = value;

        recalcProgress();
    }
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
    if(m_to != value) {
        m_to = value;

        recalcProgress();
    }
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
    if(m_value != value) {
        m_value = value;

        recalcProgress();
    }
}
/*!
    Returns the frame representing the progress chunk.
*/
Frame *ProgressBar::chunk() const {
    return static_cast<Frame *>(subWidget(gChunk));
}
/*!
    Sets the \a frame representing the progress chunk.
*/
void ProgressBar::setChunk(Frame *frame) {
    setSubWidget(frame);

    if(frame) {
        connect(frame, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        frame->setColor(m_progressColor);
    }
}
/*!
    Returns the frame representing the background.
*/
Frame *ProgressBar::background() const {
    return static_cast<Frame *>(subWidget(gBackground));
}
/*!
    Sets the \a frame representing the background.
*/
void ProgressBar::setBackground(Frame *frame) {
    setSubWidget(frame);

    if(frame) {
        frame->setColor(m_backgroundColor);
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

    Frame *background = ProgressBar::background();
    if(background) {
        background->setColor(m_backgroundColor);
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

    Frame *chunk = ProgressBar::chunk();
    if(chunk) {
        chunk->setColor(m_progressColor);
    }
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

    Actor *progress = Engine::composeActor(gFrame, gChunk, background);
    Frame *progressFrame = static_cast<Frame *>(progress->component(gFrame));
    progressFrame->setColor(m_progressColor);
    progressFrame->setBorderColor(0.0f);

    RectTransform *progressRect = progressFrame->rectTransform();
    progressRect->setMinAnchors(Vector2(0.0f, 0.0f));
    progressRect->setSize(Vector2());

    setChunk(progressFrame);

    setValue(0.5f);

    rectTransform()->setSize(Vector2(100.0f, 30.0f));
}
/*!
    \internal
    Recalculates the progress based on the current values.
*/
void ProgressBar::recalcProgress() {
    Frame *progressFrame = ProgressBar::chunk();
    if(progressFrame) {
        float factor = CLAMP(m_value / (m_to - m_from), 0.0f, 1.0f);

        RectTransform *progressRect = progressFrame->rectTransform();
        if(m_orientation == Horizontal) {
            progressRect->setMaxAnchors(Vector2(factor, 1.0f));
        } else {
            progressRect->setMaxAnchors(Vector2(1.0f, factor));
        }
        progressRect->setSize(Vector2());
    }
}
