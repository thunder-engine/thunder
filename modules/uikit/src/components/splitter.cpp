#include "components/splitter.h"

#include "components/layout.h"
#include "components/recttransform.h"

#include <input.h>

Splitter::Splitter() :
        m_savedPosition(0.0f),
        m_index(-1),
        m_orientation(Widget::Horizontal),
        m_handleWidth(5) {

}

int Splitter::handleWidth() const {
    return m_handleWidth;
}

void Splitter::setHandleWidth(int width) {
    m_handleWidth = width;
    Layout *layout = rectTransform()->layout();
    if(layout) {
        layout->setSpacing(width);
    }
}

int Splitter::orentation() const {
    return m_orientation;
}

void Splitter::setOrientation(int orientation) {
    m_orientation = orientation;
    Layout *layout = rectTransform()->layout();
    if(layout) {
        layout->setOrientation(m_orientation);

        if(m_orientation == Widget::Horizontal) {
            for(int i = 0; i < layout->count(); i++) {
                RectTransform *r = layout->transformAt(i);
                r->setVerticalPolicy(RectTransform::Expanding);
                if(i == layout->count() - 1) {
                    r->setHorizontalPolicy(RectTransform::Expanding);
                } else {
                    r->setHorizontalPolicy(RectTransform::Fixed);
                }
            }
        } else {
            for(int i = 0; i < layout->count(); i++) {
                RectTransform *r = layout->transformAt(i);
                r->setHorizontalPolicy(RectTransform::Expanding);
                if(i == layout->count() - 1) {
                    r->setVerticalPolicy(RectTransform::Expanding);
                } else {
                    r->setVerticalPolicy(RectTransform::Fixed);
                }
            }
        }
    }
}

void Splitter::addWidget(Widget *widget) {
    insertWidget(-1, widget);
}

int Splitter::count() {
    return rectTransform()->children().size();
}

int Splitter::indexOf(Widget *widget) {
    Layout *layout = rectTransform()->layout();
    if(layout && widget) {
        return layout->indexOf(widget->rectTransform());
    }

    return -1;
}

void Splitter::insertWidget(int index, Widget *widget) {
    RectTransform *widgetRect = widget->rectTransform();
    widgetRect->setParentTransform(rectTransform());
}

Widget *Splitter::replaceWidget(int index, Widget *widget) {
    Widget *result = Splitter::widget(index);
    if(result) {
        Layout *layout = rectTransform()->layout();
        if(layout) {
            layout->removeTransform(result->rectTransform());
        }

        if(widget) {
            insertWidget(index, widget);
        }
    }

    return result;
}

Widget *Splitter::widget(int index) {
    Layout *layout = rectTransform()->layout();
    if(layout) {
        RectTransform *rect = layout->transformAt(index);
        if(rect) {
            return rect->widget();
        }
    }

    return nullptr;
}

void Splitter::update(const Vector2 &pos) {
    Widget::update(pos);

    if(isHovered(pos)) {
        RectTransform *rect = rectTransform();
        Input::CursorShape shape = Input::CURSOR_ARROW;

        Layout *layout = rect->layout();
        if(layout) {
            if(m_index == -1) {
                int hoveredSlitter = -1;
                const float sense = layout->spacing() * 2;
                for(int i = 0; i < count()-1; i++) {
                    RectTransform *child = layout->transformAt(i);
                    const Matrix4 m(child->worldTransform());
                    const Vector2 size(child->size() * child->worldScale());
                    if(m_orientation == Widget::Horizontal) {
                        float d = std::abs((m[12] + size.x) - pos.x);
                        if(d < sense) {
                            hoveredSlitter = i;
                            shape = Input::CURSOR_HORSIZE;
                            break;
                        }
                    } else {
                        float d = std::abs((m[13] + size.y) - pos.y);
                        if(d < sense) {
                            hoveredSlitter = i;
                            shape = Input::CURSOR_VERSIZE;
                            break;
                        }
                    }
                }

                if(hoveredSlitter > -1 && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                    m_savedPosition = (m_orientation == Widget::Horizontal) ? pos.x : pos.y;
                    m_index = hoveredSlitter;
                }
            } else {
                shape = (m_orientation == Widget::Horizontal) ? Input::CURSOR_HORSIZE : Input::CURSOR_VERSIZE;

                if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                    float delta = ((m_orientation == Widget::Horizontal) ? pos.x : pos.y) - m_savedPosition;
                    if(delta != 0.0f) {
                        m_savedPosition = (m_orientation == Widget::Horizontal) ? pos.x : pos.y;

                        resizeWidget(m_index, delta);
                        resizeWidget(m_index + 1, -delta);
                    }
                }

                if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                    m_index = -1;
                }
            }
        }

        Input::mouseSetCursor(shape);
    }
}

void Splitter::resizeWidget(int index, float delta) {
    Widget *widget = Splitter::widget(index);
    if(widget) {
        RectTransform *rect = widget->rectTransform();
        Vector2 size(rect->size());
        if(m_orientation == Widget::Horizontal) {
            size.x = MAX(size.x + delta, 0.0f);
        } else {
            size.y = MAX(size.y + delta, 0.0f);
        }
        rect->setSize(size);
    }
}

void Splitter::childAdded(RectTransform *rect) {
    if(rect) {
        Layout *layout = rectTransform()->layout();
        if(m_orientation == Widget::Horizontal) {
            rect->setVerticalPolicy(RectTransform::Expanding);
            for(int i = 0; i < layout->count(); i++) {
                RectTransform *r = layout->transformAt(i);
                if(r == rect) {
                    rect->setHorizontalPolicy(RectTransform::Expanding);
                } else {
                    r->setHorizontalPolicy(RectTransform::Fixed);
                }
            }
        } else {
            rect->setHorizontalPolicy(RectTransform::Expanding);
            for(int i = 0; i < layout->count(); i++) {
                RectTransform *r = layout->transformAt(i);
                if(r == rect) {
                    rect->setVerticalPolicy(RectTransform::Expanding);
                } else {
                    r->setVerticalPolicy(RectTransform::Fixed);
                }
            }
        }
    }
}

void Splitter::composeComponent() {
    Frame::composeComponent();

    Layout *layout = new Layout;
    layout->setOrientation(m_orientation);
    layout->setSpacing(5);

    rectTransform()->setLayout(layout);
}
