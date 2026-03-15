#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "frame.h"

class UIKIT_EXPORT ProgressBar : public Frame {
    A_OBJECT(ProgressBar, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Orientation, orientation, ProgressBar::orientation, ProgressBar::setOrientation, "enum=Orientation"),
        A_PROPERTY(float, from, ProgressBar::from, ProgressBar::setFrom),
        A_PROPERTY(float, to, ProgressBar::to, ProgressBar::setTo),
        A_PROPERTY(float, value, ProgressBar::value, ProgressBar::setValue),
        A_PROPERTYEX(Vector4, progressColor, ProgressBar::progressColor, ProgressBar::setProgressColor, "editor=Color"),
        A_PROPERTYEX(Frame *, chunk, ProgressBar::chunk, ProgressBar::setChunk, "editor=Component")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ProgressBar();

    int orientation() const;
    void setOrientation(int orientation);

    float from() const;
    void setFrom(float value);

    float to() const;
    void setTo(float value);

    float value() const;
    void setValue(float value);

    Vector4 progressColor() const;
    void setProgressColor(const Vector4 color);

    Frame *chunk() const;
    void setChunk(Frame *frame);

private:
    void composeComponent() override;

    void recalcProgress();

private:
    Vector4 m_backgroundColor;
    Vector4 m_progressColor;

    int m_orientation;

    float m_from;
    float m_to;
    float m_value;

};

#endif // PROGRESSBAR_H
