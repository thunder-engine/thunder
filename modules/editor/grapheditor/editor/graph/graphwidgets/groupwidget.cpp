#include "groupwidget.h"

#include "../nodegroup.h"
#include "../graphcontroller.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <components/gui/label.h>
#include <components/gui/recttransform.h>

#include <resources/material.h>
#include <resources/font.h>

#include <input.h>

namespace {
    const float row = 40.0f;
    const float sensor = 5.0f;
    const float width = 100.0f;
};

GroupWidget::GroupWidget() :
    m_shape(Qt::ArrowCursor),
    m_resize(0) {

}

Vector2 GroupWidget::size() const {
    NodeGroup *node = static_cast<NodeGroup *>(m_node);

    return Vector2(node->width(), node->height());
}
void GroupWidget::setSize(const Vector2 &size) {
    NodeGroup *node = static_cast<NodeGroup *>(m_node);
    node->setWidth(size.x);
    node->setHeight(size.y);
    RectTransform *rect = rectTransform();
    rect->setSize(size);
}

int GroupWidget::cursorShape() const {
    return m_shape;
}

void GroupWidget::update() {
    Widget::update();

    Vector4 newColor = m_node->color();
    if(m_title->color() != newColor) {
        setColor(Vector4(newColor.x, newColor.y, newColor.z, 0.5f));
        m_title->setColor(newColor);
    }

    m_shape = Qt::ArrowCursor;

    Vector3 cursor = GraphController::worldPosition();
    if(m_title) {
        bool hover = m_title->rectTransform()->isHovered(cursor.x, cursor.y);
        if(hover && Input::isMouseButtonDown(0)) {
            emitSignal(_SIGNAL(pressed()));
        } else {
            RectTransform *r = rectTransform();
            Vector3 pos = r->worldPosition();
            Vector2 size = r->size();

            Vector2 lb(pos.x, pos.y);
            Vector2 lt(pos.x, pos.y + size.y);
            Vector2 rb(pos.x + size.x, pos.y);
            Vector2 rt(pos.x + size.x, pos.y + size.y);

            Vector2 c(cursor.x, cursor.y);

            bool isMouse = Input::isMouseButton(Input::MOUSE_LEFT);

            int resize = 0;
            if(Mathf::distanceToSegment(lb, lt, c) < sensor) {
                m_shape = Qt::SizeHorCursor;
                resize |= POINT_L;
            }
            if(Mathf::distanceToSegment(rb, rt, c) < sensor) {
                m_shape = Qt::SizeHorCursor;
                resize |= POINT_R;
            }
            if(Mathf::distanceToSegment(lt, rt, c) < sensor) {
                m_shape = Qt::SizeVerCursor;
                resize |= POINT_T;
            }
            if(Mathf::distanceToSegment(lb, rb, c) < sensor) {
                m_shape = Qt::SizeVerCursor;
                resize |= POINT_B;
            }

            if((resize & POINT_L && resize & POINT_T) ||
               (resize & POINT_R && resize & POINT_B)) {
                m_shape = Qt::SizeFDiagCursor;
            }
            if((resize & POINT_L && resize & POINT_B) ||
               (resize & POINT_R && resize & POINT_T)) {
                m_shape = Qt::SizeBDiagCursor;
            }

            if(m_resize != 0) {
                if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                    m_resize = 0;
                } else if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                    if(m_resize & POINT_L) {
                        size.x += pos.x - c.x;
                        if(size.x > width)  {
                            pos.x = c.x;
                        } else {
                            size.x = width;
                        }
                    }
                    if(m_resize & POINT_R) {
                        size.x = MAX(c.x - pos.x, width);
                    }
                    if(m_resize & POINT_T) {
                        size.y = MAX(c.y - pos.y, width);
                    }
                    if(m_resize & POINT_B) {
                        size.y += pos.y - c.y;
                        if(size.y > width) {
                            pos.y = c.y;
                        } else {
                            size.y = width;
                        }
                    }

                    r->setPosition(pos);
                    setSize(size);
                }
            } else if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                m_resize = resize;
            }
        }
    }
}

void GroupWidget::composeComponent() {
    Frame::composeComponent();

    setColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));

    Actor *title = Engine::composeActor("Frame", "Title", actor());
    if(title) {
        m_title = static_cast<Frame *>(title->component("Frame"));
        if(m_title) {
            RectTransform *rect = m_title->rectTransform();
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
            rect->setSize(Vector2(0, row));
            rect->setPivot(Vector2(0.0f, 1.0f));

            Vector4 corn(corners());
            corn.x = corn.y = 0.0f;
            corn.w = corn.z;
            m_title->setCorners(corn);
            m_title->setBorderColor(Vector4());

            m_label = static_cast<Label *>(Engine::objectCreate("Label", "", title));
            if(m_label) {
                m_label->lower();
                m_label->setFontSize(32);
                m_label->setAlign(Alignment::Middle | Alignment::Center);
                m_label->setColor(Vector4(0.66f, 0.66f, 0.66f, 1.0f));
                m_label->setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
            }
            m_title->lower();
        }
    }

    lower();
}
