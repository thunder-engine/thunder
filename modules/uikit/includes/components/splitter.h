#ifndef SPLITTER_H
#define SPLITTER_H

#include "frame.h"

class UIKIT_EXPORT Splitter : public Frame {
    A_OBJECT(Splitter, Frame, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Splitter();

    int handleWidth();
    void setHandleWidth(int width);

    int orentation();
    void setOrientation(int orientation);

    void addWidget(Widget *widget);

    int count();

    int indexOf(Widget *widget);

    void insertWidget(int index, Widget *widget);

    Widget *replaceWidget(int index, Widget *widget);

    Widget *widget(int index);

protected:
    void update() override;

    void resizeWidget(int index, int orientation, float delta);

    void childAdded(RectTransform *rect) const override;

    void composeComponent() override;

protected:
    float m_savedPosition;

    int m_index;

};

#endif // SPLITTER_H
