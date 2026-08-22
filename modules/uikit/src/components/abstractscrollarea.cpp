#include "components/abstractscrollarea.h"

#include <components/frame.h>
#include <components/scrollbar.h>
#include <components/recttransform.h>
#include <components/canvas.h>

AbstractScrollArea::AbstractScrollArea() :
        m_content(nullptr),
        m_vScroll(nullptr),
        m_hScroll(nullptr) {

}

AbstractScrollArea::~AbstractScrollArea() {

}

Widget *AbstractScrollArea::content() const {
    return m_content;
}

void AbstractScrollArea::setContent(Widget *content) {
    if(content != m_content) {
        m_content = content;

        if(m_content) {
            setSubWidget(m_content);

            RectTransform *contentRect = m_content->rectTransform();
            if(contentRect) {
                contentRect->setAnchors(Vector2(0, 1), Vector2(0, 1));
                contentRect->setPivot(Vector2(0, 1));
            }
        }
    }
}

ScrollBar *AbstractScrollArea::verticalScrollBar() const {
    return m_vScroll;
}

void AbstractScrollArea::setVerticalScrollBar(ScrollBar *bar) {
    if(bar != m_vScroll) {
        m_vScroll = bar;
        if(m_vScroll) {
            m_vScroll->setOrientation(Widget::Vertical);

            RectTransform *rect = m_vScroll->rectTransform();
            rect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f));
            rect->setPivot(Vector2(1.0f, 0.5f));
            rect->setSize(Vector2(16.0f, 0.0f));
            rect->setMargin(Vector4(0.0f, 0.0f, 16.0f, 0.0f));

            connect(m_vScroll, _SIGNAL(valueChanged(int)), this, _SLOT(onVScrollChanged(int)));

            setSubWidget(m_vScroll);
        }
    }
}

ScrollBar *AbstractScrollArea::horizontalScrollBar() const {
    return m_hScroll;
}

void AbstractScrollArea::setHorizontalScrollBar(ScrollBar *bar) {
    if(bar != m_hScroll) {
        m_hScroll = bar;
        if(m_hScroll) {
            m_hScroll->setOrientation(Widget::Horizontal);

            RectTransform *rect = m_hScroll->rectTransform();
            rect->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f));
            rect->setPivot(Vector2(0.5f, 0.0f));
            rect->setSize(Vector2(0.0f, 16.0f));
            rect->setMargin(Vector4(0.0f, 16.0f, 0.0f, 0.0f));

            connect(m_hScroll, _SIGNAL(valueChanged(int)), this, _SLOT(onHScrollChanged(int)));

            setSubWidget(m_hScroll);
        }
    }
}

void AbstractScrollArea::composeComponent() {
    Frame::composeComponent();

    // Create vertical scrollbar
    Actor *vActor = Engine::composeActor<ScrollBar>("vScroll", actor());
    setVerticalScrollBar(vActor->getComponent<ScrollBar>());

    // Create horizontal scrollbar
    Actor *hActor = Engine::composeActor<ScrollBar>("hScroll", actor());
    setHorizontalScrollBar(hActor->getComponent<ScrollBar>());
}

void AbstractScrollArea::boundChanged(const Vector2 &size) {
    Frame::boundChanged(size);

    updateScrollRange();
}

void AbstractScrollArea::drawSub() {
    Canvas *canvas = Frame::canvas();

    if(m_vScroll && m_vScroll->isEnabled()) {
        m_vScroll->draw();
    }
    if(m_hScroll && m_hScroll->isEnabled()) {
        m_hScroll->draw();
    }
    canvas->setClipRegion(rectTransform()->clipRegion());
    if(m_content && m_content->isEnabled()) {
        m_content->draw();
    }
    canvas->disableClip();
}

void AbstractScrollArea::onVScrollChanged(int value) {
    if(m_content) {
        RectTransform *rect = m_content->rectTransform();
        Vector3 v(rect->position());
        rect->setPosition(Vector3(v.x, static_cast<float>(value), v.z));
        repaint();
    }
}

void AbstractScrollArea::onHScrollChanged(int value) {
    if(m_content) {
        RectTransform *rect = m_content->rectTransform();
        Vector3 v(rect->position());
        rect->setPosition(Vector3(static_cast<float>(-value), v.y, v.z));
        repaint();
    }
}

void AbstractScrollArea::updateScrollRange() {
    if(m_content) {
        RectTransform *crect = m_content->rectTransform();
        Vector2 contentSize(crect->sizeHint());

        RectTransform *rect = rectTransform();
        Vector2 size(rect->size());
        Vector4 padding(rect->padding());

        float vScrollWidth = m_vScroll ? m_vScroll->rectTransform()->size().x : 0.0f;
        float hScrollHeight = m_hScroll ? m_hScroll->rectTransform()->size().y : 0.0f;

        bool needVScroll = false;
        bool needHScroll = false;
        for(int i = 0; i < 2; ++i) {
            Vector2 available(size);
            if(needVScroll) available.x -= vScrollWidth;
            if(needHScroll) available.y -= hScrollHeight;

            bool newNeedV = m_vScroll && contentSize.y > available.y;
            bool newNeedH = m_hScroll && contentSize.x > available.x;
            if(newNeedV == needVScroll && newNeedH == needHScroll) {
                break;
            }
            needVScroll = newNeedV;
            needHScroll = newNeedH;
        }

        Vector2 available(size);
        if(needVScroll) {
            available.x -= vScrollWidth;
            padding.y = vScrollWidth;
        }
        if(needHScroll) {
            available.y -= hScrollHeight;
            padding.z = hScrollHeight;
        }

        if(m_vScroll) {
            m_vScroll->setEnabled(needVScroll);
            m_vScroll->setMaximum(MAX(0, contentSize.y - available.y));
            m_vScroll->setPageStep(available.y);
            if(m_vScroll->value() > contentSize.y) {
                m_vScroll->setValue(contentSize.y);
            }
        }

        if(m_hScroll) {
            m_hScroll->setEnabled(needHScroll);
            m_hScroll->setMaximum(MAX(0, contentSize.x - available.x));
            m_hScroll->setPageStep(available.x);
            if(m_hScroll->value() > contentSize.x) {
                m_hScroll->setValue(contentSize.x);
            }
        }

        rect->setPadding(padding);
    } else {
        if(m_vScroll) {
            m_vScroll->setEnabled(false);
        }

        if(m_hScroll) {
            m_hScroll->setEnabled(false);
        }
    }
}
