/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "abstractscrollarea.h"

#include "frame.h"
#include "scrollbar.h"
#include "canvas.h"

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
            m_content->setParent(this, 0, true);
            m_content->setAnchors(Vector2(0, 1), Vector2(0, 1));
            m_content->setPivot(Vector2(0, 1));
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
            m_vScroll->setSize(Vector2(16.0f, 0.0f));
            m_vScroll->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f));
            m_vScroll->setPivot(Vector2(1.0f, 0.5f));
            m_vScroll->setSize(Vector2(16.0f, 0.0f));
            m_vScroll->setMargin(Vector4(0.0f, 0.0f, 16.0f, 0.0f));

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
            m_hScroll->setSize(Vector2(0.0f, 16.0f));
            m_hScroll->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f));
            m_hScroll->setPivot(Vector2(0.5f, 0.0f));
            m_hScroll->setSize(Vector2(0.0f, 16.0f));
            m_hScroll->setMargin(Vector4(0.0f, 16.0f, 0.0f, 0.0f));

            connect(m_hScroll, _SIGNAL(valueChanged(int)), this, _SLOT(onHScrollChanged(int)));

            setSubWidget(m_hScroll);
        }
    }
}

void AbstractScrollArea::composeComponent() {
    Frame::composeComponent();

    setVerticalScrollBar(Widget::createWidget<ScrollBar>("vScroll", this));
    setHorizontalScrollBar(Widget::createWidget<ScrollBar>("hScroll", this));
}

void AbstractScrollArea::boundChanged(const Vector2 &size) {
    Frame::boundChanged(size);

    updateScrollRange();
}

void AbstractScrollArea::drawSub() {
    Canvas *canvas = Frame::canvas();

    Vector4 clip = clipRegion();
    if(m_vScroll && m_vScroll->isVisible()) {
        clip.z -= m_vScroll->size().x * m_vScroll->worldScale().x;
    }
    if(m_hScroll && m_hScroll->isVisible()) {
        const float height = m_hScroll->size().y * m_hScroll->worldScale().y;
        clip.y += height;
        clip.w -= height;
    }
    canvas->setClipRegion(clip);
    if(m_content && m_content->isVisible()) {
        m_content->draw();
    }
    canvas->disableClip();

    if(m_vScroll && m_vScroll->isVisible()) {
        m_vScroll->draw();
    }
    if(m_hScroll && m_hScroll->isVisible()) {
        m_hScroll->draw();
    }
    for(Widget *child : childWidgets()) {
        if(child && child != m_content && child != m_vScroll && child != m_hScroll &&
           child->isSubWidget() && child->isVisible()) {
            child->draw();
        }
    }
}

void AbstractScrollArea::onVScrollChanged(int value) {
    if(m_content) {
        Vector3 v(m_content->position());
        m_content->setPosition(Vector3(v.x, static_cast<float>(value), v.z));
        repaint();
    }
}

void AbstractScrollArea::onHScrollChanged(int value) {
    if(m_content) {
        Vector3 v(m_content->position());
        m_content->setPosition(Vector3(static_cast<float>(-value), v.y, v.z));
        repaint();
    }
}

void AbstractScrollArea::updateScrollRange() {
    if(m_content) {
        Widget *crect = m_content;
        Vector2 contentSize(crect->sizeHint());

        Widget *rect = this;
        Vector2 size(rect->size());
        Vector4 padding(rect->padding());
        const Vector4 border(rect->border());

        float vScrollWidth = m_vScroll ? m_vScroll->size().x : 0.0f;
        float hScrollHeight = m_hScroll ? m_hScroll->size().y : 0.0f;

        Vector2 viewport(MAX(0.0f, size.x - padding.w - padding.y - border.w - border.y),
                         MAX(0.0f, size.y - padding.x - padding.z - border.x - border.z));
        bool needVScroll = false;
        bool needHScroll = false;
        for(int i = 0; i < 3; ++i) {
            Vector2 available(viewport);
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

        Vector2 available(viewport);
        if(needVScroll) {
            available.x -= vScrollWidth;
        }
        if(needHScroll) {
            available.y -= hScrollHeight;
        }
        available.x = MAX(0.0f, available.x);
        available.y = MAX(0.0f, available.y);

        const float rightInset = padding.y + border.y;
        const float bottomInset = padding.z + border.z;

        if(m_vScroll) {
            m_vScroll->setAnchors(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f));
            m_vScroll->setPivot(Vector2(1.0f, 1.0f));
            m_vScroll->setSize(Vector2(vScrollWidth, available.y));
            m_vScroll->setMargin(Vector4(bottomInset + (needHScroll ? hScrollHeight : 0.0f), 0.0f, 0.0f, 0.0f));
            m_vScroll->setPosition(Vector3(-rightInset, 0, 0.0f));
            m_vScroll->setEnabled(needVScroll);
            m_vScroll->setVisible(needVScroll);
            m_vScroll->setMaximum(MAX(0, contentSize.y - available.y));
            m_vScroll->setPageStep(available.y);
            if(m_vScroll->value() > m_vScroll->maximum()) {
                m_vScroll->setValue(m_vScroll->maximum());
            }
        }
        if(m_hScroll) {
            m_hScroll->setAnchors(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f));
            m_hScroll->setPivot(Vector2(1.0f, 1.0f));
            m_hScroll->setSize(Vector2(available.x, hScrollHeight));
            m_hScroll->setPosition(Vector3(-rightInset-vScrollWidth, -available.y, 0.0f));
            m_hScroll->setEnabled(needHScroll);
            m_hScroll->setVisible(needHScroll);
            m_hScroll->setMaximum(MAX(0, contentSize.x - available.x));
            m_hScroll->setPageStep(available.x);
            if(m_hScroll->value() > m_hScroll->maximum()) {
                m_hScroll->setValue(m_hScroll->maximum());
            }
        }
    } else {
        if(m_vScroll) {
            m_vScroll->setEnabled(false);
            m_vScroll->setVisible(false);
        }

        if(m_hScroll) {
            m_hScroll->setEnabled(false);
            m_hScroll->setVisible(false);
        }
    }
}
