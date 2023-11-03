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
#include <components/gui/recttransform.h>

#include <systems/rendersystem.h>

#include <editor/viewport/cameracontroller.h>
#include <editor/editorplatform.h>

namespace {
    const char *gLinksRender("LinksRender");
    const char *gFrame("Frame");
    const char *gNodeWidget("NodeWidget");
    const char *gGroupWidget("GroupWidget");
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

    ObjectObserver::registerClassFactory(Engine::renderSystem());
    m_objectObserver->setView(this);
}

void GraphView::setWorld(World *scene) {
    Viewport::setWorld(scene);

    m_scene = Engine::objectCreate<Scene>("Scene", m_world);
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
        QStringList list = it.split("/", QString::SkipEmptyParts);

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

Frame *GraphView::rubberBand() {
    return m_rubberBand;
}

void GraphView::createLink(NodeWidget *node, int port) {
    Widget *widget = nullptr;
    if(node) {
        GraphNode *n = node->node();
        if(n->isState()) {
            widget = reinterpret_cast<NodeWidget *>(n->widget());
        } else {
            NodePort *p = n->port(port);
            widget = reinterpret_cast<PortWidget *>(p->m_userData);
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
                PortWidget *w2 = reinterpret_cast<PortWidget *>(p2->m_userData);
                if(w2) {
                    w2->portUpdate();
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

    std::list<PortWidget *> widgets = {reinterpret_cast<PortWidget *>(p1->m_userData)};
    for(auto it : g->findLinks(p1)) {
        if(it->oport == p1 && it->iport) {
            widgets.push_back(reinterpret_cast<PortWidget *>(it->iport->m_userData));
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

    if(m_linksRender == nullptr) {
        Actor *actor = Engine::composeActor(gLinksRender, gLinksRender, m_scene);
        m_linksRender = static_cast<LinksRender *>(actor->component(gLinksRender));
        m_linksRender->setGraph(g);
    }

    if(m_rubberBand == nullptr) {
        Actor *actor = Engine::composeActor(gFrame, gFrame, m_scene);
        m_rubberBand = static_cast<Frame *>(actor->component(gFrame));
        m_rubberBand->setColor(Vector4(0.376f, 0.376f, 0.376f, 0.3f));
        m_rubberBand->setBorderColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f));
        m_rubberBand->setCorners(Vector4());

        RectTransform *rect = m_rubberBand->rectTransform();
        rect->setPivot(Vector2());

        m_rubberBand->setEnabled(false);
    }

    // Clean scene graph
    Object::ObjectList children = m_scene->getChildren();
    auto it = children.begin();
    while(it != children.end()) {
        if(*it == m_linksRender->actor() || *it == m_rubberBand->actor()) {
            it = children.erase(it);
            continue;
        }
        ++it;
    }

    for(auto node : g->nodes()) {
        if(node == nullptr) {
            continue;
        }
        bool create = true;
        for(auto it : children) {
            Widget *widget = reinterpret_cast<Widget *>(node->widget());
            if(widget && widget->actor() == it) {
                children.remove(it);
                create = false;
                break;
            }
        }

        if(create) {
            NodeGroup *group = dynamic_cast<NodeGroup *>(node);
            NodeWidget *widget = nullptr;
            if(group) {
                Actor *nodeActor = Engine::composeActor(gGroupWidget, qPrintable(node->objectName()), m_scene);
                if(nodeActor) {
                    widget = dynamic_cast<GroupWidget *>(nodeActor->component(gGroupWidget));
                }
            } else {
                Actor *nodeActor = Engine::composeActor(gNodeWidget, qPrintable(node->objectName()), m_scene);
                if(nodeActor) {
                    widget = dynamic_cast<NodeWidget *>(nodeActor->component(gNodeWidget));
                }
            }

            if(widget) {
                node->setWidget(widget);

                widget->setView(this);
                widget->setGraphNode(node);
                widget->setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

                RectTransform *rect = widget->rectTransform();
                rect->setPosition(Vector3(node->position().x, -node->position().y - rect->size().y, 0.0f));

                Object::connect(widget, _SIGNAL(pressed()), m_objectObserver, _SLOT(onNodePressed()));
                Object::connect(widget, _SIGNAL(portPressed(int)), m_objectObserver, _SLOT(onPortPressed(int)));
                Object::connect(widget, _SIGNAL(portReleased(int)), m_objectObserver, _SLOT(onPortReleased(int)));
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
    emit itemsSelected(static_cast<GraphController *>(m_controller)->selectedItems());
}

void GraphView::showMenu() {
    if(m_createMenu->exec(QCursor::pos()) == nullptr) {
        m_linksRender->setCreationLink(nullptr);
    }
}

void GraphView::onComponentSelected() {
    AbstractNodeGraph *g = graph();

    QAction *action = static_cast<QAction *>(sender());

    Vector3 pos(GraphController::worldPosition());

    Widget *widget = m_linksRender->creationLink();
    if(widget) {
        PortWidget *portWidget = dynamic_cast<PortWidget *>(widget);
        if(portWidget) {
            NodePort *p1 = portWidget->port();
            GraphNode *n1 = p1->m_node;

            g->createAndLink(action->objectName(), pos.x, -pos.y, g->node(n1), n1->portPosition(p1), p1->m_out);
        } else {
            NodeWidget *nodeWidget = dynamic_cast<NodeWidget *>(widget);
            if(nodeWidget) {
                g->createAndLink(action->objectName(), pos.x, -pos.y, g->node(nodeWidget->node()), -1, true);
            }
        }
        m_linksRender->setCreationLink(nullptr);
    } else {
        g->createNode(action->objectName(), pos.x, -pos.y);
    }

    m_createMenu->hide();
}

void GraphView::onDraw() {
    if(m_createMenu->isVisible()) {
        return;
    }

    if(m_world) {
        m_world->setToBeUpdated(true);
    }

    Viewport::onDraw();

    if(m_world) {
        m_world->setToBeUpdated(false);
    }
}
