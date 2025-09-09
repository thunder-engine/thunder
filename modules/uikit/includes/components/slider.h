#ifndef SLIDER_H
#define SLIDER_H

#include "abstractslider.h"

class ProgressBar;

class UIKIT_EXPORT Slider : public AbstractSlider {
    A_OBJECT(Slider, AbstractSlider, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Slider();

    ProgressBar *background() const;
    void setBackground(ProgressBar *background);

    void setValue(int value) override;
    void setMinimum(int value) override;
    void setMaximum(int value) override;

    void setOrientation(int value) override;

private:
    void update() override;

    void composeComponent() override;

private:
    Vector4 m_normalColor;

    Vector4 m_highlightedColor;

};

#endif // SLIDER_H
