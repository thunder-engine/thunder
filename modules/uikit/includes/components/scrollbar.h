#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "abstractslider.h"

class UIKIT_EXPORT ScrollBar : public AbstractSlider {
    A_OBJECT(ScrollBar, AbstractSlider, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SLOT(ScrollBar::stepBack),
        A_SLOT(ScrollBar::stepFront)
    )
    A_NOENUMS()

public:
    ScrollBar();

    void setValue(int value) override;
    void setMinimum(int value) override;
    void setMaximum(int value) override;

    void setOrientation(int value) override;

    int pageStep() const;
    void setPageStep(int page);

    int singleStep() const;
    void setSingleStep(int step);

    void stepBack();
    void stepFront();

    Widget *backArrow() const;
    void setBackArrow(Widget *arrow);

    Widget *frontArrow() const;
    void setFrontArrow(Widget *arrow);

private:
    void recalcKnob();

    void boundChanged(const Vector2 &size) override;

    void update() override;

    void composeComponent() override;

private:
    int m_pageStep;

    int m_singleStep;

};

#endif // SCROLLBAR_H
