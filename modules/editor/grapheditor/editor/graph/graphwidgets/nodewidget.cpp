#include "nodewidget.h"

#include "portwidget.h"

#include "../graphnode.h"
#include "../abstractnodegraph.h"
#include "../graphview.h"
#include "../graphcontroller.h"

#include <components/actor.h>
#include <components/textrender.h>
#include <components/spriterender.h>

#include <components/recttransform.h>
#include <components/layout.h>
#include <components/label.h>
#include <components/image.h>
#include <components/toolbutton.h>

#include <resources/material.h>
#include <resources/font.h>

#include <input.h>

const float row = 20.0f;

namespace {
    const char *gPortWidget("PortWidget");
    const char *gImage("Image");
    const char *gFrame("Frame");
    const char *gToolButton("ToolButton");
};

NodeWidget::NodeWidget() :
        m_node(nullptr),
        m_label(nullptr),
        m_title(nullptr),
        m_preview(nullptr),
        m_previewBtn(nullptr),
        m_view(nullptr),
        m_callLayout(nullptr),
        m_hovered(false) {

}

void NodeWidget::setView(GraphView *view) {
    m_view = view;
}

void NodeWidget::setGraphNode(GraphNode *node) {
    m_node = node;
    if(m_label) {
        string title = !m_node->objectName().isEmpty() ? qPrintable(m_node->objectName()) : m_node->typeName();
        m_label->setText(title);
    }

    if(m_title) {
        m_title->setColor(m_node->color());
        if(m_node->isState()) {
            rectTransform()->layout()->setMargins(0.0f, 10.0f, 10.0f, 0.0f);

            RectTransform *rect = m_title->rectTransform();
            rect->setMargin(Vector4(10.0f));
        }
    }

    // Call ports
    for(NodePort &port : m_node->ports()) {
        if(port.m_call) {
            composePort(port);
        }
    }

    // Out ports
    for(NodePort &port : m_node->ports()) {
        if(port.m_out && !port.m_call) {
            composePort(port);
        }
    }

    // Add properties
    if(!m_node->isState()) {

    }

    // In ports
    for(NodePort &port : m_node->ports()) {
        if(!port.m_out && !port.m_call) {
            composePort(port);
        }
    }

    RectTransform *rect = rectTransform();
    Layout *layout = rect->layout();

    // Add preview if exist
    AbstractNodeGraph *graph = m_node->graph();
    if(graph) {
        Texture *preview = graph->preview(m_node);
        if(preview) {
            Actor *actor = Engine::composeActor(gImage, "Preview", NodeWidget::actor());
            m_preview = static_cast<Image *>(actor->component(gImage));
            if(m_preview) {
                m_preview->setTexture(preview);
                m_preview->setDrawMode(SpriteRender::Simple);

                RectTransform *r = m_preview->rectTransform();
                r->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
                r->setSize(Vector2(preview->width(), preview->height()));
                r->setPivot(Vector2(0.5f, 1.0f));
                if(layout) {
                    layout->addTransform(r);
                }
            }
            actor->setEnabled(false);
        } else if(m_previewBtn) {
            m_previewBtn->actor()->setEnabled(false);
        }
    }

    Vector2 size = m_node->defaultSize();
    if(!m_node->ports().empty()) {
        size.y = (m_node->ports().size() + 2.0f) * row;
        if(layout) {
            size.y = layout->sizeHint().y;
            layout->update();
        }
    }

    rect->setSize(size);
}

void NodeWidget::setSelected(bool flag) {
    if(flag) {
       setBorderColor(Vector4(1.0f));
    } else {
       setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

Frame *NodeWidget::title() const {
    return m_title;
}

GraphNode *NodeWidget::node() const {
    return m_node;
}

void NodeWidget::update() {
    Widget::update();

    Vector3 pos = GraphController::worldPosition();
    if(m_previewBtn && m_previewBtn->actor()->isEnabled()) {
        RectTransform *rect = m_previewBtn->rectTransform();
        bool hover = rect->isHovered(pos.x, pos.y);
        if(hover && Input::isMouseButtonDown(0)) {
            Actor *p = m_preview->actor();
            if(p->isEnabled()) {
                rect->setRotation(Vector3(0.0f, 0.0f, 90.0f));
            } else {
                rect->setRotation(Vector3(0.0f, 0.0f, 0.0f));
            }
            p->setEnabled(!p->isEnabled());

            if(m_node) {
                m_node->graph()->setPreviewVisible(m_node, p->isEnabled());

                RectTransform *rect = rectTransform();
                Layout *layout = rect->layout();

                Vector2 size = rect->size();
                Vector3 p = rect->position() + Vector3(size, 0.0f);

                size.y = (m_node->ports().size() + 2.0f) * row;
                if(layout) {
                    size.y = layout->sizeHint().y;
                    layout->invalidate();
                    layout->update();
                }
                rect->setSize(size);
                rect->setPosition(p - Vector3(size, 0.0f));
            }
            m_view->composeLinks();
            return;
        }
    }
    if(m_title) {
        bool hover = m_title->rectTransform()->isHovered(pos.x, pos.y);
        if(hover && Input::isMouseButtonDown(0)) {
            emitSignal(_SIGNAL(pressed()));
        } else {
            if(m_node && m_node->isState()) { // For state machine
                bool hover = rectTransform()->isHovered(pos.x, pos.y);
                if(m_hovered != hover) {
                    m_hovered = hover;

                    Vector4 color(m_frameColor);
                    if(m_hovered) {
                        color.x = CLAMP(color.x + 0.25f, 0.0f, 1.0f);
                        color.y = CLAMP(color.y + 0.25f, 0.0f, 1.0f);
                        color.z = CLAMP(color.z + 0.25f, 0.0f, 1.0f);
                    } else {
                        color.x = CLAMP(color.x - 0.25f, 0.0f, 1.0f);
                        color.y = CLAMP(color.y - 0.25f, 0.0f, 1.0f);
                        color.z = CLAMP(color.z - 0.25f, 0.0f, 1.0f);
                    }
                    setColor(color);
                }

                if(m_hovered) {
                    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                        emitSignal(_SIGNAL(portPressed(int)), -1);
                    }
                    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                        emitSignal(_SIGNAL(portReleased(int)), -1);
                    }
                }
            }
        }
    }
}

void NodeWidget::composeComponent() {
    Frame::composeComponent();

    setColor(Vector4(0.376f, 0.376f, 0.376f, 1.0f));
    setCorners(8);

    Layout *layout = new Layout;
    layout->setSpacing(2.0f);
    layout->setMargins(0.0f, 0.0f, 0.0f, corners().x);
    rectTransform()->setLayout(layout);

    Actor *title = Engine::composeActor(gFrame, "Title", actor());
    if(title) {
        m_title = static_cast<Frame *>(title->component(gFrame));
        if(m_title) {
            RectTransform *rect = m_title->rectTransform();
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
            rect->setSize(Vector2(0, row));
            rect->setPivot(Vector2(0.0f, 1.0f));

            layout->addTransform(rect);

            Vector4 corn(corners());
            corn.x = corn.y = 0.0f;
            corn.w = corn.z;
            m_title->setCorners(corn);
            m_title->setBorderColor(Vector4());

            m_label = Engine::objectCreate<Label>("", title);
            if(m_label) {
                m_label->setFontSize(14);
                m_label->setAlign(Alignment::Middle | Alignment::Center);
                m_label->setColor(Vector4(1.0f));
                m_label->setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
            }

            Actor *icon = Engine::composeActor(gImage, gImage, title);
            m_previewBtn = static_cast<Image *>(icon->component(gImage));

            m_previewBtn->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
            m_previewBtn->setItem("Arrow");
            RectTransform *t = m_previewBtn->rectTransform();
            if(t) {
                t->setSize(Vector2(16.0f, 8.0f));
                t->setMargin(Vector4(0.0f, 10.0f, 0.0f, 10.0f));
                t->setAnchors(Vector2(1.0f, 0.5f), Vector2(1.0f, 0.5f));
                t->setPivot(Vector2(1.0f, 0.5f));
                t->setRotation(Vector3(0.0f, 0.0f, 90.0f));
            }
        }
    }
}

void NodeWidget::composePort(NodePort &port) {
    Actor *portActor = Engine::composeActor(gPortWidget, port.m_name.c_str(), actor());
    if(portActor) {
        PortWidget *portWidget = static_cast<PortWidget *>(portActor->component(gPortWidget));
        if(portWidget) {
            RectTransform *r = portWidget->rectTransform();
            r->setSize(Vector2(0, row));
            portWidget->setNodePort(&port);
            Layout *layout = rectTransform()->layout();
            if(layout) {
                if(port.m_call) {
                    if(m_callLayout) {
                        if(port.m_out) {
                            m_callLayout->insertTransform(-1, r);
                        } else {
                            m_callLayout->insertTransform(0, r);
                        }
                    } else {
                        m_callLayout = new Layout;
                        m_callLayout->setDirection(Layout::Horizontal);
                        m_callLayout->addTransform(r);
                        layout->addLayout(m_callLayout);
                    }
                } else {
                    layout->addTransform(r);
                }
            }
            connect(portWidget, _SIGNAL(pressed(int)), this, _SIGNAL(portPressed(int)));
            connect(portWidget, _SIGNAL(released(int)), this, _SIGNAL(portReleased(int)));
        }
    }
}
