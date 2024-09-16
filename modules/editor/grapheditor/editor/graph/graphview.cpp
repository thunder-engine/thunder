#include "graphview.h"

#include <QMenu>
#include <QWindow>

#include "graphnode.h"
#include "graphcontroller.h"
#include "nodegroup.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/groupwidget.h"
#include "graphwidgets/portwidget.h"
#include "graphwidgets/linksrender.h"

#include <components/actor.h>
#include <components/scene.h>
#include <components/world.h>
#include <components/camera.h>
#include <components/recttransform.h>

#include <systems/rendersystem.h>

#include <editor/viewport/cameracontroller.h>
#include <editor/editorplatform.h>

namespace {
    const char *gLinksRender("LinksRender");
    const char *gFrame("Frame");
};

class ObjectObserver : public Object {
    A_REGISTER(ObjectObserver, Object, Editor)

    A_METHODS(
        A_SLOT(ObjectObserver::onNodePressed),
        A_SLOT(ObjectObserver::onPortPressed),
        A_SLOT(ObjectObserver::onPortReleased)
    )

public:
    ObjectObserver() :
        m_view(nullptr) {

    }

    void setView(GraphView *view) {
        m_view = view;
    }

    void onNodePressed() {
        if(m_view) {
            GraphController *ctrl = dynamic_cast<GraphController *>(m_view->controllder());
            if(ctrl) {
                ctrl->setFocusNode(dynamic_cast<NodeWidget *>(sender()));
            }
        }
    }

    void onPortPressed(int port) {
        if(m_view) {
            NodeWidget *widget = dynamic_cast<NodeWidget *>(sender());
            if(widget && Input::isKey(Input::KEY_LEFT_ALT)) {
                m_view->deleteLink(widget, port);
            } else { // Link creation mode
                m_view->createLink(widget, port);
            }
        }
    }

    void onPortReleased(int port) {
        if(m_view) {
            NodeWidget *widget = dynamic_cast<NodeWidget *>(sender());
            if(widget) {
                m_view->buildLink(widget, port);
            }
        }
    }

private:
    GraphView *m_view;

};

GraphView::GraphView(QWidget *parent) :
        Viewport(parent),
        m_scene(nullptr),
        m_view(nullptr),
        m_createMenu(new QMenu(this)),
        m_objectObserver(new ObjectObserver),
        m_linksRender(nullptr),
        m_rubberBand(nullptr) {

    m_controller = new GraphController(this);
    m_controller->frontSide();
    m_controller->blockRotations(true);
    m_controller->setZoomLimits(Vector2(400, 2000));

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
        camera->setOrthoSize(500.0f);
    }

    static bool firtCall = true;
    if(firtCall) {
        NodeWidget::registerClassFactory(Engine::renderSystem());
        GroupWidget::registerClassFactory(Engine::renderSystem());
        PortWidget::registerClassFactory(Engine::renderSystem());
        LinksRender::registerClassFactory(Engine::renderSystem());
        firtCall = false;
    }

    m_objectObserver->setView(this);

    setLiveUpdate(true);
}

void GraphView::setWorld(World *scene) {
    Viewport::setWorld(scene);

    m_scene = Engine::objectCreate<Scene>("Scene", m_world);
    m_view = Engine::composeActor("Widget", "View", m_scene);

    Actor *actor = Engine::composeActor(gLinksRender, gLinksRender, m_view);
    m_linksRender = static_cast<LinksRender *>(actor->component(gLinksRender));

    actor = Engine::composeActor(gFrame, gFrame, m_view);
    m_rubberBand = static_cast<Frame *>(actor->component(gFrame));
    m_rubberBand->setColor(Vector4(0.376f, 0.376f, 0.376f, 0.3f));
    m_rubberBand->setBorderColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f));

    RectTransform *rect = m_rubberBand->rectTransform();
    rect->setPivot(Vector2(0.0f));

    m_rubberBand->setEnabled(false);
}

AbstractNodeGraph *GraphView::graph() const {
    return static_cast<GraphController *>(m_controller)->graph();
}

void GraphView::setGraph(AbstractNodeGraph *graph) {
    static_cast<GraphController *>(m_controller)->setGraph(graph);

    connect(graph, &AbstractNodeGraph::graphUpdated, this, &GraphView::onGraphUpdated);

    // Create menu
    for(auto &it : graph->nodeList()) {
        QMenu *menu = m_createMenu;
        QStringList list = it.split("/", Qt::SkipEmptyParts);

        for(int i = 0; i < list.size(); i++) {
            QString part = list.at(i);
            QAction *action = nullptr;
            for(QAction *act : menu->actions()) {
                if(part == act->objectName()) {
                    action = act;
                    menu = act->menu();
                    break;
                }
            }
            if(action == nullptr) {
                action = menu->addAction(part);
                action->setObjectName(qPrintable(part));
                if(i < (list.size() - 1)) {
                    menu = new QMenu;
                    action->setMenu(menu);
                } else {
                    connect(action, &QAction::triggered, this, &GraphView::onComponentSelected);
                }
            }
        }
    }
}

Actor &GraphView::view() const {
    return *m_view;
}

Frame *GraphView::rubberBand() {
    return m_rubberBand;
}

void GraphView::createLink(NodeWidget *node, int port) {
    Widget *widget = nullptr;
    if(node) {
        GraphNode *n = node->node();

        widget = n->portWidget(port);
        if(widget == nullptr) {
            widget = node;
        }
    }

    m_linksRender->setCreationLink(widget);
}

void GraphView::buildLink(NodeWidget *node, int port) {
    AbstractNodeGraph *g = graph();

    PortWidget *widget = dynamic_cast<PortWidget *>(m_linksRender->creationLink());
    if(widget) {
        NodePort *p1 = widget->port();
        GraphNode *n1 = p1->m_node;
        GraphNode *n2 = node->node();

        if(n1 != n2) {
            if(p1->m_out) {
                g->createLink(g->node(n1), n1->portPosition(p1), g->node(n2), port);
                NodePort *p2 = n2->port(port);
                if(p2) {
                    PortWidget *w2 = reinterpret_cast<PortWidget *>(p2->m_userData);
                    if(w2) {
                        w2->portUpdate();
                    }
                }
            } else {
                g->createLink(g->node(n2), port, g->node(n1), n1->portPosition(p1));
                widget->portUpdate();
            }
        }
        m_linksRender->setCreationLink(nullptr);
    } else {
        NodeWidget *widget = dynamic_cast<NodeWidget *>(m_linksRender->creationLink());
        if(widget) {
            GraphNode *n1 = widget->node();
            GraphNode *n2 = node->node();

            g->createLink(g->node(n1), -1, g->node(n2), -1);

            m_linksRender->setCreationLink(nullptr);
        }
    }
}

void GraphView::deleteLink(NodeWidget *node, int port) {
    GraphNode *n1 = node->node();
    NodePort *p1 = n1->port(port);

    AbstractNodeGraph *g = graph();

    std::list<PortWidget *> widgets;
    if(p1) {
        widgets = {reinterpret_cast<PortWidget *>(p1->m_userData)};
        for(auto it : g->findLinks(p1)) {
            if(it->oport == p1 && it->iport) {
                widgets.push_back(reinterpret_cast<PortWidget *>(it->iport->m_userData));
            }
        }
    }

    g->deleteLinksByPort(g->node(n1), port);

    for(auto it : widgets) {
        it->portUpdate();
    }
}

bool GraphView::isCreationLink() const {
    return m_linksRender->creationLink() != nullptr;
}

void GraphView::composeLinks() {
    m_linksRender->composeLinks();
}

void GraphView::onGraphUpdated() {
    AbstractNodeGraph *g = graph();

    m_linksRender->setGraph(g);

    // Clean scene graph
    Object::ObjectList children = m_view->getChildren();
    auto it = children.begin();
    while(it != children.end()) {
        Actor *actor = dynamic_cast<Actor *>(*it);

        if(actor == nullptr || actor == m_linksRender->actor() || actor == m_rubberBand->actor()) {
            it = children.erase(it);
            continue;
        }

        ++it;
    }

    for(auto node : g->nodes()) {
        if(node == nullptr) {
            continue;
        }

        Widget *widget = reinterpret_cast<Widget *>(node->widget());
        if(widget) {
            Actor *actor = widget->actor();
            if(actor->parent() == nullptr) {
                actor->setParent(m_view);
                actor->transform()->setPosition(Vector3(node->position(), 0.0f));
                actor->transform()->setScale(Vector3(1.0f));

                Object::connect(widget, _SIGNAL(pressed()), m_objectObserver, _SLOT(onNodePressed()));
                Object::connect(widget, _SIGNAL(portPressed(int)), m_objectObserver, _SLOT(onPortPressed(int)));
                Object::connect(widget, _SIGNAL(portReleased(int)), m_objectObserver, _SLOT(onPortReleased(int)));
            }
        }

        for(auto it : children) {
             if(widget) {
                Actor *actor = widget->actor();
                if(actor == it) {
                    children.remove(it);
                    break;
                }
            }
        }
    }

    GraphController *ctrl = static_cast<GraphController *>(m_controller);

    // Remove deleted nodes
    for(auto it : children) {
        if(ctrl && it == ctrl->focusNode()) {
            ctrl->setFocusNode(nullptr);
        }
        delete it;
    }

    composeLinks();
}

void GraphView::reselect() {
    auto list = static_cast<GraphController *>(m_controller)->selectedItems();
    emit itemsSelected(list);
}

void GraphView::showMenu() {
    if(m_createMenu->exec(QCursor::pos()) == nullptr) {
        m_linksRender->setCreationLink(nullptr);
    }
}

void GraphView::onComponentSelected() {
    AbstractNodeGraph *g = graph();

    QAction *action = static_cast<QAction *>(sender());

    Vector4 pos(Input::mousePosition());

    RectTransform *t = static_cast<RectTransform *>(m_view->transform());

    Vector3 localPos(t->worldTransform().inverse() * Vector3(pos.x, pos.y, 0.0f));

    localPos.x -= t->size().x * 0.5f;
    localPos.y -= t->size().y * 0.5f;

    Widget *widget = m_linksRender->creationLink();
    if(widget) {
        PortWidget *portWidget = dynamic_cast<PortWidget *>(widget);
        if(portWidget) {
            NodePort *p1 = portWidget->port();
            GraphNode *n1 = p1->m_node;

            g->createAndLink(action->objectName(), localPos.x, localPos.y, g->node(n1), n1->portPosition(p1), p1->m_out);
        } else {
            NodeWidget *nodeWidget = dynamic_cast<NodeWidget *>(widget);
            if(nodeWidget) {
                g->createAndLink(action->objectName(), localPos.x, localPos.y, g->node(nodeWidget->node()), -1, true);
            }
        }
        m_linksRender->setCreationLink(nullptr);
    } else {
        g->createNode(action->objectName(), localPos.x, localPos.y);
    }

    m_createMenu->hide();
}

void GraphView::onDraw() {
    if(m_createMenu->isVisible()) {
        return;
    }

    Viewport::onDraw();
}
