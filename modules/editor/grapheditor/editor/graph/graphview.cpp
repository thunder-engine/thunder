#include "graphview.h"

#include <QMenu>
#include <QWindow>

#include "abstractnodegraph.h"
#include "graphnode.h"
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

#include <editor/viewport/cameractrl.h>
#include <editor/editorplatform.h>

namespace {
    const char *gLinksRender("LinksRender");
    const char *gFrame("Frame");
    const char *gNodeWidget("NodeWidget");
    const char *gGroupWidget("GroupWidget");
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
        m_scene(nullptr),
        m_createMenu(new QMenu(this)),
        m_graph(nullptr),
        m_objectObserver(new ObjectObserver),
        m_linksRender(nullptr),
        m_rubberBand(nullptr),
        m_focusedWidget(nullptr),
        m_drag(false) {

    m_controller = new CameraCtrl;
    m_controller->frontSide();
    m_controller->blockRotations(true);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
        camera->setOrthoSize(650.0f);
    }

    installEventFilter(this);
    m_rhiWindow->installEventFilter(this);

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

    m_scene = dynamic_cast<Scene *>(Engine::objectCreate("Scene", "Scene", m_sceneGraph));
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
    m_focusedWidget = widget;
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
        Actor *actor = Engine::composeActor(gLinksRender, gLinksRender, m_scene);
        m_linksRender = static_cast<LinksRender *>(actor->component(gLinksRender));
        m_linksRender->setGraph(m_graph);
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

    for(auto node : m_graph->nodes()) {
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

    // Remove deleted nodes
    for(auto it : children) {
        if(it == m_focusedWidget) {
            m_focusedWidget = nullptr;
        }
        delete it;
    }

    composeLinks();
}

void GraphView::reselect() {
    emit itemsSelected(m_selectedItems);
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

        EditorPlatform::instance().setMousePosition(QPoint(mouseWorld.x, mouseWorld.y));
    }

    Viewport::onDraw();

    Vector4 pos = Input::mousePosition();

    Qt::CursorShape shape = Qt::ArrowCursor;
    for(auto node : m_graph->nodes()) {
        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
        GroupWidget *group = dynamic_cast<GroupWidget *>(widget);
        if(group) {
            Qt::CursorShape s = static_cast<Qt::CursorShape>(group->cursorShape());
            if(s != Qt::ArrowCursor) {
                shape = s;
            }
        }
    }

    if(shape != Qt::ArrowCursor) {
        onCursorSet(QCursor(shape));
    } else {
        onCursorUnset();
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        if(m_focusedWidget == nullptr &&
           shape == Qt::ArrowCursor &&
           m_linksRender->creationLink() == nullptr) {
            m_rubberBand->setEnabled(true);
            m_rubberBand->raise();
            RectTransform *rect = m_rubberBand->rectTransform();

            m_rubberOrigin = Vector2(pos.x, pos.y);
            rect->setPosition(Vector3(m_rubberOrigin, 0.0f));
            rect->setSize(Vector2());
        }
    } else if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(m_linksRender->creationLink()) {
            if(m_createMenu->exec(QCursor::pos()) == nullptr) {
                m_linksRender->setCreationLink(nullptr);
            }
        }
    }

    if(m_rubberBand->isEnabled()) {
        QRect r(QPoint(MIN(m_rubberOrigin.x, pos.x), MIN(m_rubberOrigin.y, pos.y)),
                QPoint(MAX(m_rubberOrigin.x, pos.x), MAX(m_rubberOrigin.y, pos.y)));

        RectTransform *transform = m_rubberBand->rectTransform();
        transform->setPosition(Vector3(r.x(), r.y(), 0.0f));
        transform->setSize(Vector2(r.width(), r.height()));

        QList<QObject *> list;
        for(auto node : m_graph->nodes()) {
            NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
            transform = widget->rectTransform();
            QRect n(QPoint(transform->position().x, transform->position().y),
                    QSize(transform->size().x, transform->size().y));

            if(r.intersects(n)) {
                widget->setSelected(true);
                list.push_back(node);
            } else {
                widget->setSelected(false);
            }
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
            m_rubberBand->setEnabled(false);
            if(!list.empty()) {
                m_selectedItems = list;
                emit itemsSelected(m_selectedItems);
            } else {
                for(auto it : qAsConst(m_selectedItems)) {
                    GraphNode *node = static_cast<GraphNode *>(it);
                    reinterpret_cast<NodeWidget *>(node->widget())->setSelected(false);
                }
                m_selectedItems.clear();
                m_softSelectedItems.clear();
                emit itemsSelected({m_graph->rootNode()});
            }
        }
    }

    if(m_focusedWidget) {
        RectTransform *title = m_focusedWidget->title()->rectTransform();

        if(title->isHovered(pos.x, pos.y)) {
            if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                m_originMousePos = Vector3(pos.x, pos.y, 0.0f);
            }

            if(!m_drag && Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                for(auto node : m_graph->nodes()) {
                    NodeWidget *widget = reinterpret_cast<NodeWidget *>(node->widget());
                    if(node->widget() == m_focusedWidget) {
                        widget->setSelected(true);
                        m_selectedItems = {node};
                        m_softSelectedItems.clear();
                        emit itemsSelected(m_selectedItems);
                    } else {
                        widget->setSelected(false);
                    }
                }
            }
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
            std::list<NodeWidget *> list;
            for(auto it : qAsConst(m_selectedItems)) {
                GraphNode *node = static_cast<GraphNode *>(it);
                NodeWidget *widget = reinterpret_cast<NodeWidget *>(node->widget());
                list.push_back(widget);
            }
            for(auto it : qAsConst(m_softSelectedItems)) {
                GraphNode *node = static_cast<GraphNode *>(it);
                NodeWidget *widget = reinterpret_cast<NodeWidget *>(node->widget());
                list.push_back(widget);
            }
            UndoManager::instance()->push(new MoveNodes(list, this));

            m_focusedWidget = nullptr;
            m_drag = false;
        }

        if(m_drag) {
            if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                Vector3 newPos = m_originNodePos + Vector3(pos.x, pos.y, 0.0f) - m_originMousePos;

                float snap = gridCell();
                for(int n = 0; n < 3; n++) {
                    newPos[n] = snap * int(newPos[n] / snap);
                }

                RectTransform *rect = reinterpret_cast<NodeWidget *>(m_focusedWidget)->rectTransform();
                Vector3 deltaPos = (newPos - Vector3(0.0f, rect->size().y, 0.0f)) - rect->position();

                for(auto it : qAsConst(m_selectedItems)) {
                    GraphNode *node = static_cast<GraphNode *>(it);
                    RectTransform *rect = reinterpret_cast<NodeWidget *>(node->widget())->rectTransform();
                    rect->setPosition(rect->position() + deltaPos);
                }
                for(auto it : qAsConst(m_softSelectedItems)) {
                    GraphNode *node = static_cast<GraphNode *>(it);
                    RectTransform *rect = reinterpret_cast<NodeWidget *>(node->widget())->rectTransform();
                    rect->setPosition(rect->position() + deltaPos);
                }
                composeLinks();
            }

            if(Input::isMouseButtonDown(Input::MOUSE_RIGHT)) { // Cancel drag
                RectTransform *rect = reinterpret_cast<NodeWidget *>(m_focusedWidget)->rectTransform();
                Vector3 deltaPos = (m_originNodePos - Vector3(0.0f, rect->size().y, 0.0f)) - rect->position();

                for(auto it : qAsConst(m_selectedItems)) {
                    GraphNode *node = static_cast<GraphNode *>(it);
                    RectTransform *rect = reinterpret_cast<NodeWidget *>(node->widget())->rectTransform();
                    rect->setPosition(rect->position() + deltaPos);
                }
                for(auto it : qAsConst(m_softSelectedItems)) {
                    GraphNode *node = static_cast<GraphNode *>(it);
                    RectTransform *rect = reinterpret_cast<NodeWidget *>(node->widget())->rectTransform();
                    rect->setPosition(rect->position() + deltaPos);
                }
                composeLinks();
                m_softSelectedItems.clear();
                m_focusedWidget = nullptr;
                m_drag = false;
            }
        } else {
            if(m_focusedWidget && (Vector3(pos.x, pos.y, 0.0f) - m_originMousePos).length() > 5.0f) { // Drag sensor = 5.0f
                if(m_selectedItems.isEmpty() || !isSelected(m_focusedWidget)) { // Select on drag
                    for(auto it : qAsConst(m_selectedItems)) {
                        GraphNode *node = static_cast<GraphNode *>(it);
                        reinterpret_cast<NodeWidget *>(node->widget())->setSelected(false);
                    }
                    m_selectedItems = {m_focusedWidget->node()};
                    emit itemsSelected(m_selectedItems);
                    m_focusedWidget->setSelected(true);
                }

                m_softSelectedItems.clear();

                GroupWidget *group = dynamic_cast<GroupWidget *>(m_focusedWidget);
                if(group) {
                    RectTransform *rect = group->rectTransform();
                    QRect r(QPoint(rect->position().x, rect->position().y),
                            QSize(rect->size().x, rect->size().y));

                    for(auto node : m_graph->nodes()) {
                        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
                        if(group == widget) {
                            continue;
                        }
                        rect = widget->rectTransform();
                        QRect n(QPoint(rect->position().x, rect->position().y),
                                QSize(rect->size().x, rect->size().y));

                        if(r.contains(n) && !m_selectedItems.contains(widget->node())) {
                            m_softSelectedItems.push_back(widget->node());
                        }
                    }
                }

                RectTransform *rect = m_focusedWidget->rectTransform();
                m_originNodePos = rect->position() + Vector3(0.0f, rect->size().y, 0.0f);
                m_drag = true;
            }
        }
    }

    if(!m_selectedItems.isEmpty() && Input::isKeyDown(Input::KEY_DELETE)) {
        vector<int32_t> selection;
        for(auto it : qAsConst(m_selectedItems)) {
            if(it != m_graph->rootNode()) {
                selection.push_back(m_graph->node(static_cast<GraphNode *>(it)));
            }
        }
        if(!selection.empty()) {
            emit itemsSelected({m_graph->rootNode()});
            m_graph->deleteNodes(selection);
            m_selectedItems.clear();
        }
    }

    EditorPlatform::instance().update();

    if(m_sceneGraph) {
        m_sceneGraph->setToBeUpdated(false);
    }
}

bool GraphView::isSelected(NodeWidget *widget) const {
    bool result = false;
    if(m_focusedWidget) {
        for(auto it : qAsConst(m_selectedItems)) {
            GraphNode *node = static_cast<GraphNode *>(it);
            if(node->widget() == m_focusedWidget) {
                result = true;
                break;
            }
        }
    }
    return result;
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
