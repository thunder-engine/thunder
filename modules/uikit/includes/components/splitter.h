#ifndef SPLITTER_H
#define SPLITTER_H

#include "frame.h"

class UIKIT_EXPORT Splitter : public Frame {
    A_OBJECT(Splitter, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(int, orentation, Splitter::orentation, Splitter::setOrientation, "enum=Orientation"),
        A_PROPERTY(int, handleWidth, Splitter::handleWidth, Splitter::setHandleWidth)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Splitter();

    int handleWidth() const;
    void setHandleWidth(int width);

    int orentation() const;
    void setOrientation(int orientation);

    void addWidget(Widget *widget);

    int count();

    int indexOf(Widget *widget);

    void insertWidget(int index, Widget *widget);

    Widget *replaceWidget(int index, Widget *widget);

    Widget *widget(int index);

protected:
    void update(const Vector2 &pos) override;

    void resizeWidget(int index, float delta);

    void childAdded(RectTransform *rect) override;

    void composeComponent() override;

protected:
    float m_savedPosition;

    int m_index;

    int m_orientation;

    int m_handleWidth;

};

#endif // SPLITTER_H
