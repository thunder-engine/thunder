#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "frame.h"

class UIKIT_EXPORT ProgressBar : public Widget {
    A_OBJECT(ProgressBar, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(float, from, ProgressBar::from, ProgressBar::setFrom),
        A_PROPERTY(float, to, ProgressBar::to, ProgressBar::setTo),
        A_PROPERTY(float, value, ProgressBar::value, ProgressBar::setValue),
        A_PROPERTYEX(Vector4, backgroundColor, ProgressBar::backgroundColor, ProgressBar::setBackgroundColor, "editor=Color"),
        A_PROPERTYEX(Vector4, progressColor, ProgressBar::progressColor, ProgressBar::setProgressColor, "editor=Color"),
        A_PROPERTYEX(Frame *, background, ProgressBar::background, ProgressBar::setBackground, "editor=Component"),
        A_PROPERTYEX(Frame *, chunk, ProgressBar::chunk, ProgressBar::setChunk, "editor=Component")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ProgressBar();

    float from() const;
    void setFrom(float value);

    float to() const;
    void setTo(float value);

    float value() const;
    void setValue(float value);

    Vector4 backgroundColor() const;
    void setBackgroundColor(const Vector4 color);

    Vector4 progressColor() const;
    void setProgressColor(const Vector4 color);

    Frame *chunk() const;
    void setChunk(Frame *frame);

    Frame *background() const;
    void setBackground(Frame *frame);

private:
    void composeComponent() override;

    void recalcProgress();

private:
    Vector4 m_backgroundColor;
    Vector4 m_progressColor;

    float m_from;
    float m_to;
    float m_value;

};

#endif // PROGRESSBAR_H
