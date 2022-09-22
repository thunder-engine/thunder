#include "graphview.h"

#include <QMenu>
#include <QWindow>

#include "abstractnodegraph.h"
#include "graphnode.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/portwidget.h"

#include <components/actor.h>
#include <components/scenegraph.h>
#include <components/camera.h>

#include <systems/rendersystem.h>

#include <editor/viewport/cameractrl.h>
#include <editor/editorplatform.h>

GraphView::GraphView(QWidget *parent) :
    Viewport(parent),
    m_createMenu(new QMenu(this)),
    m_graph(nullptr),
    m_selectedItem(nullptr),
    m_node(-1),
    m_port(-1),
    m_out(false) {

    setContextMenuPolicy(Qt::CustomContextMenu);

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
        firtCall = false;
    }
}

void GraphView::setGraph(AbstractNodeGraph *graph, bool state) {
    m_graph = graph;

    connect(m_graph, &AbstractNodeGraph::graphUpdated, this, &GraphView::onGraphUpdated);

    //connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));
    //connect(item, SIGNAL(showContextMenu(int,int,bool)), this, SLOT(onShowContextMenu(int,int,bool)));

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

void GraphView::onGraphUpdated() {
    // Clean scene graph
    Object::ObjectList children = m_sceneGraph->getChildren();

    for(auto node : m_graph->nodes()) {
        bool create = true;
        for(auto it : children) {
            if(node->objectName() == it->name().c_str()) {
                children.remove(it);
                create = false;
            }
        }

        if(create) {
            Actor *nodeActor = Engine::composeActor("NodeWidget", qPrintable(node->objectName()), m_sceneGraph);
            if(nodeActor) {
                NodeWidget *widget = dynamic_cast<NodeWidget *>(nodeActor->component("NodeWidget"));
                if(widget) {
                    widget->setGraphNode(node);
                    return;
                }
            }
        }
    }

    // Remove deleted nodes
    for(auto it : children) {
        it->deleteLater();
    }
}

void GraphView::reselect() {
    emit itemSelected(m_selectedItem);
}

void GraphView::onComponentSelected() {
    m_createMenu->hide();

    QAction *action = static_cast<QAction *>(sender());
/*
    QQuickItem *scheme = rootObject()->findChild<QQuickItem *>("Scheme");
    if(scheme) {
        int x = scheme->property("x").toInt();
        int y = scheme->property("y").toInt();
        float scale = scheme->property("scale").toFloat();

        QQuickItem *canvas = rootObject()->findChild<QQuickItem *>("Canvas");
        if(canvas) {
            int mouseX = canvas->property("mouseX").toInt();
            int mouseY = canvas->property("mouseY").toInt();
            x = (float)(mouseX - x) * scale;
            y = (float)(mouseY - y) * scale;

            if(m_node > -1 && m_port > -1) {
                m_graph->createAndLink(action->objectName(), x, y, m_node, m_port, m_out);
            } else {
                m_graph->createNode(action->objectName(), x, y);
            }
        }
    }
*/
}

void GraphView::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        GraphNode *node = m_graph->node(list.front().toInt());
        if(node) {
            m_selectedItem = node;
            emit itemSelected(m_selectedItem);
        }
    }
}

void GraphView::onDraw() {
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

    EditorPlatform::instance().update();

    if(m_sceneGraph) {
        m_sceneGraph->setToBeUpdated(false);
    }
}

void GraphView::onShowContextMenu(int node, int port, bool out) {
    m_node = node;
    m_port = port;
    m_out = out;
    m_createMenu->exec(QCursor::pos());
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
            return true;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMouseButtons(ev->button(), RELEASE);
            return true;
        }
        default: break;
    }
    return Viewport::eventFilter(object, event);
}
