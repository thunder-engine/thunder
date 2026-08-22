#ifndef ABSTRACTSCROLLAREA_H
#define ABSTRACTSCROLLAREA_H

#include "frame.h"
#include "scrollbar.h"

class UIKIT_EXPORT AbstractScrollArea : public Frame {
    A_OBJECT(AbstractScrollArea, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(ScrollBar *, verticalScrollBar, AbstractScrollArea::verticalScrollBar, AbstractScrollArea::setVerticalScrollBar, "editor=Component"),
        A_PROPERTYEX(ScrollBar *, horizontalScrollBar, AbstractScrollArea::horizontalScrollBar, AbstractScrollArea::setHorizontalScrollBar, "editor=Component")
    )
    A_METHODS(
        A_SLOT(AbstractScrollArea::onVScrollChanged),
        A_SLOT(AbstractScrollArea::onHScrollChanged)
    )
    A_NOENUMS()

public:
    AbstractScrollArea();
    ~AbstractScrollArea();

    Widget *content() const;
    void setContent(Widget *content);

    ScrollBar *verticalScrollBar() const;
    void setVerticalScrollBar(ScrollBar *bar);

    ScrollBar *horizontalScrollBar() const;
    void setHorizontalScrollBar(ScrollBar *bar);

protected:
    void composeComponent() override;
    void boundChanged(const Vector2 &size) override;

    void drawSub() override;

    virtual void onVScrollChanged(int value);
    virtual void onHScrollChanged(int value);

    void updateScrollRange();

protected:
    Widget *m_content;

    ScrollBar *m_vScroll;
    ScrollBar *m_hScroll;

};

#endif // ABSTRACTSCROLLAREA_H
