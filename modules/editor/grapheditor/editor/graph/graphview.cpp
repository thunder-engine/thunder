#include "graphview.h"

#include <QMenu>
#include <QWindow>

#include "abstractnodegraph.h"
#include "graphnode.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/portwidget.h"
#include "graphwidgets/linksrender.h"

#include <components/actor.h>
#include <components/scenegraph.h>
#include <components/camera.h>
#include <components/gui/recttransform.h>

#include <systems/rendersystem.h>

#include <editor/viewport/cameractrl.h>
#include <editor/editorplatform.h>

namespace {
    const char *gLinksRender("LinksRender");
    const char *gNodeWidget("NodeWidget");
};

class ObjectObserver : public Object {
    A_REGISTER(ObjectObserver, Object, "General")

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
            m_view->focusNode(dynamic_cast<NodeWidget *>(sender()));
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
        m_createMenu(new QMenu(this)),
        m_graph(nullptr),
        m_selectedItem(nullptr),
        m_objectObserver(new ObjectObserver),
        m_linksRender(nullptr),
        m_focusedNode(nullptr),
        m_drag(false) {

    m_controller = new CameraCtrl;
    m_controller->frontSide();
    m_controller->blockRotations(true);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
        camera->setOrthoSize(650.0f);
    }

    m_rhiWindow->installEventFilter(this);

    static bool firtCall = true;
    if(firtCall) {
        NodeWidget::registerClassFactory(Engine::renderSystem());
        PortWidget::registerClassFactory(Engine::renderSystem());
        LinksRender::registerClassFactory(Engine::renderSystem());
        firtCall = false;
    }

    ObjectObserver::registerClassFactory(Engine::renderSystem());
    m_objectObserver->setView(this);
}

AbstractNodeGraph *GraphView::graph() const {
    return m_graph;
}

void GraphView::setGraph(AbstractNodeGraph *graph, bool state) {
    m_graph = graph;

    connect(m_graph, &AbstractNodeGraph::graphUpdated, this, &GraphView::onGraphUpdated);

    // Create menu
    for(auto &it : m_graph->nodeList()) {
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

void GraphView::focusNode(NodeWidget *widget) {
    m_focusedNode = widget;
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
    PortWidget *widget = dynamic_cast<PortWidget *>(m_linksRender->creationLink());
    if(widget) {
        NodePort *p1 = widget->port();
        GraphNode *n1 = p1->m_node;
        GraphNode *n2 = node->node();

        if(n1 != n2) {
            if(p1->m_out) {
                m_graph->createLink(m_graph->node(n1), n1->portPosition(p1), m_graph->node(n2), port);
                NodePort *p2 = n2->port(port);
                PortWidget *w2 = reinterpret_cast<PortWidget *>(p2->m_userData);
                if(w2) {
                    w2->portUpdate();
                }
            } else {
                m_graph->createLink(m_graph->node(n2), port, m_graph->node(n1), n1->portPosition(p1));
                widget->portUpdate();
            }
        }
        m_linksRender->setCreationLink(nullptr);
    } else {
        NodeWidget *widget = dynamic_cast<NodeWidget *>(m_linksRender->creationLink());
        if(widget) {
            GraphNode *n1 = widget->node();
            GraphNode *n2 = node->node();

            m_graph->createLink(m_graph->node(n1), -1, m_graph->node(n2), -1);

            m_linksRender->setCreationLink(nullptr);
        }
    }
}

void GraphView::deleteLink(NodeWidget *node, int port) {
    GraphNode *n1 = node->node();
    NodePort *p1 = n1->port(port);

    std::list<PortWidget *> widgets = {reinterpret_cast<PortWidget *>(p1->m_userData)};
    for(auto it : m_graph->findLinks(p1)) {
        if(it->oport == p1) {
            widgets.push_back(reinterpret_cast<PortWidget *>(it->iport->m_userData));
        }
    }

    m_graph->deleteLinksByPort(m_graph->node(n1), port);

    for(auto it : widgets) {
        it->portUpdate();
    }
}

void GraphView::composeLinks() {
    m_linksRender->composeLinks();
}

void GraphView::onGraphUpdated() {
    if(m_linksRender == nullptr) {
        Actor *actor = Engine::composeActor(gLinksRender, gLinksRender, m_sceneGraph);
        m_linksRender = static_cast<LinksRender *>(actor->component(gLinksRender));
        m_linksRender->setGraph(m_graph);
    }

    // Clean scene graph
    Object::ObjectList children = m_sceneGraph->getChildren();
    for(auto it : children) {
        if(it == m_linksRender->actor()) {
            children.remove(it);
            break;
        }
    }

    for(auto node : m_graph->nodes()) {
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
            Actor *nodeActor = Engine::composeActor(gNodeWidget, qPrintable(node->objectName()), m_sceneGraph);
            if(nodeActor) {
                NodeWidget *widget = dynamic_cast<NodeWidget *>(nodeActor->component(gNodeWidget));
                if(widget) {
                    node->setWidget(widget);

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
    }

    // Remove deleted nodes
    for(auto it : children) {
        if(it == m_focusedNode) {
            m_focusedNode = nullptr;
        }
        delete it;
    }

    composeLinks();
}

void GraphView::reselect() {
    emit itemSelected(m_selectedItem);
}

void GraphView::onComponentSelected() {
    QAction *action = static_cast<QAction *>(sender());

    Vector4 pos = Input::mousePosition();

    Widget *widget = m_linksRender->creationLink();
    if(widget) {
        PortWidget *portWidget = dynamic_cast<PortWidget *>(widget);
        if(portWidget) {
            NodePort *p1 = portWidget->port();
            GraphNode *n1 = p1->m_node;

            m_graph->createAndLink(action->objectName(), pos.x, -pos.y, m_graph->node(n1), n1->portPosition(p1), p1->m_out);
        } else {
            NodeWidget *nodeWidget = dynamic_cast<NodeWidget *>(widget);
            if(nodeWidget) {
                m_graph->createAndLink(action->objectName(), pos.x, -pos.y, m_graph->node(nodeWidget->node()), -1, true);
            }
        }
        m_linksRender->setCreationLink(nullptr);
    } else {
        m_graph->createNode(action->objectName(), pos.x, -pos.y);
    }

    m_createMenu->hide();
}

void GraphView::onDraw() {
    if(m_createMenu->isVisible()) {
        return;
    }

    if(m_sceneGraph) {
        m_sceneGraph->setToBeUpdated(true);

        QPoint p(mapFromGlobal(QCursor::pos()));
        Vector3 mouseWorld = Camera::unproject(Vector3(static_cast<float>(p.x()) / width(),
                                               1.0f - static_cast<float>(p.y()) / height(), 0.0f),
                                               m_controller->camera()->viewMatrix(),
                                               m_controller->camera()->projectionMatrix());

        EditorPlatform::instance().setMousePosition(Vector2(mouseWorld.x, mouseWorld.y));
    }

    Viewport::onDraw();

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(m_linksRender->creationLink()) {
            if(m_createMenu->exec(QCursor::pos()) == nullptr) {
                m_linksRender->setCreationLink(nullptr);
            }
        }
    }

    if(m_focusedNode) {
        RectTransform *title = m_focusedNode->title()->rectTransform();
        Vector4 pos = Input::mousePosition();

        if(title->isHovered(pos.x, pos.y) && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
            m_originMousePos = Vector3(pos.x, pos.y, 0.0f);
            RectTransform *rect = m_focusedNode->rectTransform();
            m_originNodePos = rect->position() + Vector3(0.0f, rect->size().y, 0.0f);

            for(auto node : m_graph->nodes()) {
                if(node->widget() == m_focusedNode) {
                    reinterpret_cast<NodeWidget *>(node->widget())->setBorderColor(Vector4(1.0f));
                    m_selectedItem = node;
                    emit itemSelected(m_selectedItem);
                } else {
                    reinterpret_cast<NodeWidget *>(node->widget())->setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
                }
            }
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
            UndoManager::instance()->push(new MoveNodes({m_focusedNode}, this));

            m_focusedNode = nullptr;
            m_drag = false;
        }

        if(m_drag) {
            if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                Vector3 newPos = m_originNodePos + (Vector3(pos.x, pos.y, 0.0f) - m_originMousePos);

                float snap = gridCell();
                for(int i = 0; i < 3; i++) {
                    newPos[i] = snap * int(newPos[i] / snap);
                }
                RectTransform *rect = m_focusedNode->rectTransform();
                rect->setPosition(newPos - Vector3(0.0f, rect->size().y, 0.0f));
                composeLinks();
            }

            if(Input::isMouseButtonDown(Input::MOUSE_RIGHT)) { // Cancel drag
                RectTransform *rect = m_focusedNode->rectTransform();
                rect->setPosition(m_originNodePos + Vector3(0.0f, rect->size().y, 0.0f));
                composeLinks();
                m_focusedNode = nullptr;
                m_drag = false;
            }
        } else {
            if(m_focusedNode && (Vector3(pos.x, pos.y, 0.0f) - m_originMousePos).length() > 5.0f) { // Drag sensor = 5.0f
                m_drag = true;
            }
        }
    }

    if(m_selectedItem && m_selectedItem != m_graph->rootNode() && Input::isKeyDown(Input::KEY_DELETE)) {
        emit itemSelected(nullptr);
        m_graph->deleteNodes({m_graph->node(static_cast<GraphNode *>(m_selectedItem))});
        m_selectedItem = nullptr;
    }

    EditorPlatform::instance().update();

    if(m_sceneGraph) {
        m_sceneGraph->setToBeUpdated(false);
    }
}

bool GraphView::eventFilter(QObject *object, QEvent *event) {
    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), ev->text(), false, ev->isAutoRepeat());
            return true;
        }
        case QEvent::KeyRelease: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), "", true, ev->isAutoRepeat());
            return true;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMouseButtons(ev->button(), PRESS);
        } break;
        case QEvent::MouseButtonRelease: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMouseButtons(ev->button(), RELEASE);

            if(ev->button() == Qt::RightButton && !m_drag && !m_controller->cameraInMove()) {
                m_createMenu->exec(QCursor::pos());
            }
        } break;
        default: break;
    }
    return Viewport::eventFilter(object, event);
}

MoveNodes::MoveNodes(const list<NodeWidget *> &selection, GraphView *view, const QString &name, QUndoCommand *parent) :
        m_view(view),
        UndoGraph(view->graph(), name, parent) {

    m_indices.reserve(selection.size());
    m_points.reserve(selection.size());
    for(auto &it : selection) {
        m_indices.push_back(m_view->graph()->node(it->node()));

        RectTransform *rect = it->rectTransform();
        Vector3 pos(rect->position());
        m_points.push_back(Vector2(pos.x, -pos.y - rect->size().y));
    }
}
void MoveNodes::undo() {
    redo();
}
void MoveNodes::redo() {
    vector<Vector2> positions(m_indices.size());
    for(int i = 0; i < m_indices.size(); i++) {
        GraphNode *node = m_graph->node(m_indices.at(i));
        positions[i] = node->position();
        node->setPosition(m_points.at(i));

        // Update widget position
        RectTransform *rect = reinterpret_cast<NodeWidget *>(node->widget())->rectTransform();
        Vector3 pos(node->position().x, -node->position().y - rect->size().y, 0.0f);
        rect->setPosition(pos);
    }
    // Recalc links positions
    m_view->composeLinks();
    m_points = positions;
}
