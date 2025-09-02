#ifndef ABSTRACTSLIDER_H
#define ABSTRACTSLIDER_H

#include "widget.h"

class Frame;

class UIKIT_EXPORT AbstractSlider : public Widget {
    A_OBJECT(AbstractSlider, Widget, General)

    A_PROPERTIES(
        A_PROPERTYEX(int, orientation, AbstractSlider::orientation, AbstractSlider::setOrientation, "enum=Orientation"),
        A_PROPERTY(int, value, AbstractSlider::value, AbstractSlider::setValue),
        A_PROPERTY(int, minimum, AbstractSlider::minimum, AbstractSlider::setMinimum),
        A_PROPERTY(int, maximum, AbstractSlider::maximum, AbstractSlider::setMaximum),
        A_PROPERTYEX(Widget *, knob, AbstractSlider::knob, AbstractSlider::setKnob, "editor=Component")
    )
    A_METHODS(
        A_SIGNAL(AbstractSlider::pressed),
        A_SIGNAL(AbstractSlider::valueChanged)
    )
    A_ENUMS(
        A_ENUM(Orientation,
            A_VALUE(Horizontal),
            A_VALUE(Vertical)
        )
    )

public:
    enum Orientation {
        Horizontal,
        Vertical
    };

public:
    AbstractSlider();

    int orientation() const;
    virtual void setOrientation(int orientation);

    int value() const;
    virtual void setValue(int value);

    int minimum() const;
    virtual void setMinimum(int minimum);

    int maximum() const;
    virtual void setMaximum(int maximum);

    Widget *knob() const;
    void setKnob(Widget *handle);

public: // signals
    void pressed();

    void valueChanged(int value);

protected:
    void update() override;

protected:
    int m_value;
    int m_minimum;
    int m_maximum;

    float m_currentFade;

    int m_orientation;

    bool m_hovered;

};

#endif // ABSTRACTSLIDER_H
