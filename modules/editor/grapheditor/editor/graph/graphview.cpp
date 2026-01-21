#include "graphview.h"

#include <QMenu>

#include "graphnode.h"
#include "graphcontroller.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/groupwidget.h"
#include "graphwidgets/portwidget.h"
#include "graphwidgets/linksrender.h"

#include "actions/createnode.h"
#include "actions/createlink.h"
#include "actions/pastenodes.h"
#include "actions/deletenodes.h"
#include "actions/deletelinks.h"
#include "actions/changenodeproperty.h"

#include <components/actor.h>
#include <components/scene.h>
#include <components/world.h>
#include <components/camera.h>
#include <components/recttransform.h>

#include <pipelinecontext.h>
#include <pipelinetask.h>
#include <systems/rendersystem.h>

#include <editor/editorplatform.h>

#include <url.h>

namespace {
    const char *gLinksRender("LinksRender");
    const char *gFrame("Frame");
};

class GraphViewProxy : public Object {
    A_OBJECT(GraphViewProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(GraphViewProxy::onPortPressed),
        A_SLOT(GraphViewProxy::onPortReleased),
        A_SLOT(GraphViewProxy::onGraphUpdated),
        A_SLOT(GraphViewProxy::onGraphLoaded)
    )

public:
    GraphViewProxy() :
        m_view(nullptr) {

    }

    void setView(GraphView *view) {
        m_view = view;
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

    void onGraphUpdated() {
        if(m_view) {
            m_view->onGraphUpdated();
        }
    }

    void onGraphLoaded() {
        if(m_view) {
            m_view->onGraphLoaded();
        }
    }

private:
    GraphView *m_view;

};

GraphView::GraphView(QWidget *editor) :
        Viewport(editor),
        m_scene(nullptr),
        m_view(nullptr),
        m_createMenu(new QMenu(this)),
        m_proxy(new GraphViewProxy),
        m_linksRender(nullptr),
        m_rubberBand(nullptr),
        m_editor(nullptr),
        m_updateLinks(false) {

    m_controller = new GraphController(this);
    m_controller->doRotation(Vector3());
    m_controller->setGridAxis(CameraController::Axis::Z);
    m_controller->blockRotations(true);
    m_controller->setTransferSpeed(100.0f);

    static bool firtCall = true;
    if(firtCall) {
        NodeWidget::registerClassFactory(Engine::renderSystem());
        GroupWidget::registerClassFactory(Engine::renderSystem());
        PortWidget::registerClassFactory(Engine::renderSystem());
        LinksRender::registerClassFactory(Engine::renderSystem());
        firtCall = false;
    }

    m_proxy->setView(this);

    connect(static_cast<GraphController *>(m_controller), &GraphController::copied, this, &GraphView::copied);

    setLiveUpdate(true);
}

void GraphView::setEditor(AssetEditor *editor) {
    m_editor = editor;
}

void GraphView::setWorld(World *scene) {
    Viewport::setWorld(scene);

    m_scene = Engine::objectCreate<Scene>("Scene", m_world);
    m_view = Engine::composeActor<Widget>("View", m_scene);

    Actor *actor = Engine::composeActor<LinksRender>(gLinksRender, m_view);
    m_linksRender = actor->getComponent<LinksRender>();

    actor = Engine::composeActor<Frame>(gFrame, m_view);
    m_rubberBand = actor->getComponent<Frame>();
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

    Object::connect(graph, _SIGNAL(graphUpdated()), m_proxy, _SLOT(onGraphUpdated()));
    Object::connect(graph, _SIGNAL(graphLoaded()), m_proxy, _SLOT(onGraphLoaded()));

    StringList nodeList = graph->nodeList();

    nodeList.sort();

    // Create menu
    for(auto &it : nodeList) {
        QMenu *menu = m_createMenu;
        QStringList list = QString(it.data()).split("/", Qt::SkipEmptyParts);

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

    GraphController *ctrl = static_cast<GraphController *>(m_controller);

    PortWidget *widget = dynamic_cast<PortWidget *>(m_linksRender->creationLink());
    if(widget) {
        NodePort *p1 = widget->port();
        GraphNode *n1 = p1->m_node;
        GraphNode *n2 = node->node();

        if(n1 != n2) {
            if(p1->m_out) {
                m_editor->undoRedo()->push(new CreateLink(g->node(n1), n1->portPosition(p1), g->node(n2), port, ctrl));
                NodePort *p2 = n2->port(port);
                if(p2) {
                    PortWidget *w2 = static_cast<PortWidget *>(p2->m_widget);
                    if(w2) {
                        w2->portUpdate();
                    }
                }
            } else {
                m_editor->undoRedo()->push(new CreateLink(g->node(n2), port, g->node(n1), n1->portPosition(p1), ctrl));
                widget->portUpdate();
            }
        }
        cancelLinkCreation();
    } else {
        NodeWidget *widget = dynamic_cast<NodeWidget *>(m_linksRender->creationLink());
        if(widget) {
            GraphNode *n1 = widget->node();
            GraphNode *n2 = node->node();

            if(n1 != n2) {
                m_editor->undoRedo()->push(new CreateLink(g->node(n1), -1, g->node(n2), -1, ctrl));
            }

            cancelLinkCreation();
        }
    }
}

void GraphView::deleteLink(NodeWidget *node, int port) {
    GraphNode *n1 = node->node();
    NodePort *p1 = n1->port(port);

    AbstractNodeGraph *g = graph();

    std::list<PortWidget *> widgets;
    if(p1) {
        widgets = {static_cast<PortWidget *>(p1->m_widget)};
        for(auto it : g->findLinks(p1)) {
            if(it->oport == p1 && it->iport) {
                widgets.push_back(static_cast<PortWidget *>(it->iport->m_widget));
            }
        }
    }

    std::list<int32_t> links;
    if(port == -1) {
        for(auto l : g->findLinks(n1)) {
            int32_t index = g->link(l);
            if(index > -1) {
                links.push_back(index);
            }
        }
    } else {
        for(auto l : g->findLinks(p1)) {
            int32_t index = g->link(l);
            if(index > -1) {
                links.push_back(index);
            }
        }
    }

    m_editor->undoRedo()->push(new DeleteLinks(links, static_cast<GraphController *>(m_controller)));

    for(auto it : widgets) {
        it->portUpdate();
    }
}

bool GraphView::isCreationLink() const {
    return m_linksRender->creationLink() != nullptr;
}

void GraphView::cancelLinkCreation() {
    m_linksRender->setCreationLink(nullptr);
}

void GraphView::composeLinks() {
    Object::ObjectList list;
    for(auto it : static_cast<GraphController *>(m_controller)->selectedLinks()) {
        GraphLink *link = graph()->link(it);
        if(link) {
            list.push_back(link);
        }
    }

    m_linksRender->setSelectedLinks(list);

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

                Object::connect(widget, _SIGNAL(portPressed(int)), m_proxy, _SLOT(onPortPressed(int)));
                Object::connect(widget, _SIGNAL(portReleased(int)), m_proxy, _SLOT(onPortReleased(int)));
            }

            RectTransform *rect = widget->rectTransform();
            rect->setPosition(Vector3(0.0f, 0.0f, 1.0f)); // To make widget dirty
            rect->setPosition(Vector3(node->position(), 0.0f));
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

    // Remove deleted nodes
    for(auto it : children) {
        delete it;
    }

    composeLinks();
}

void GraphView::onGraphLoaded() {
    GraphController *ctrl = static_cast<GraphController *>(m_controller);
    ctrl->setGraph(ctrl->graph());
}

void GraphView::onDraw() {
    Viewport::onDraw();

    if(m_updateLinks) {
        m_updateLinks = false;
        composeLinks();
    }
}

void GraphView::resizeEvent(QResizeEvent *event) {
    Viewport::resizeEvent(event);

    m_updateLinks = true;
}

void GraphView::reselect() {
    Object::ObjectList selected = m_controller->selected();
    if(selected.empty()) {
        emit objectsSelected({graph()});
    } else {
        emit objectsSelected(selected);
    }
}

void GraphView::showMenu() {
    onInProgressFlag(true);
    if(m_createMenu->exec(QCursor::pos()) == nullptr) {
        cancelLinkCreation();
    }
}

bool GraphView::isCopyActionAvailable() const {
    return !m_controller->selected().empty();
}

bool GraphView::isPasteActionAvailable() const {
    return !static_cast<GraphController *>(m_controller)->copyData().empty();
}

UndoStack *GraphView::undoRedo() const {
    return m_editor->undoRedo();
}

void GraphView::onCutAction() {
    onCopyAction();

    AbstractNodeGraph *g = graph();
    GraphController *controller = static_cast<GraphController *>(m_controller);

    std::list<int32_t> selection;
    for(auto it : controller->selected()) {
        GraphNode *node = static_cast<GraphNode *>(it);
        if(node->isRemovable()) {
            selection.push_back(g->node(node));
        }
    }
    m_editor->undoRedo()->push(new DeleteNodes(selection, controller, Engine::translate("Cut Nodes")));
}

void GraphView::onCopyAction() {
    static_cast<GraphController *>(m_controller)->copySelected();
}

void GraphView::onPasteAction() {
    Vector4 pos(Input::mousePosition());

    RectTransform *t = static_cast<RectTransform *>(m_view->transform());
    Vector3 localPos(t->worldTransform().inverse() * Vector3(pos.x, pos.y, 0.0f));

    localPos.x -= t->size().x * 0.5f;
    localPos.y -= t->size().y * 0.5f;

    GraphController *controller = static_cast<GraphController *>(m_controller);

    const std::string &data = controller->copyData();

    m_editor->undoRedo()->push(new PasteNodes(data, localPos.x, localPos.y, controller));
}

void GraphView::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    TString name(Engine::translate("Change %1").arg(objects.front()->name()));

    m_editor->undoRedo()->push(new ChangeNodeProperty(objects, property, value,
                                                      static_cast<GraphController *>(m_controller), name));
}

void GraphView::onComponentSelected() {
    AbstractNodeGraph *g = graph();

    RectTransform *t = static_cast<RectTransform *>(m_view->transform());

    std::string type = sender()->objectName().toStdString();

    Vector4 pos(Input::mousePosition());
    Vector3 localPos(t->worldTransform().inverse() * Vector3(pos.x, pos.y, 0.0f));

    localPos.x -= t->size().x * 0.5f;
    localPos.y -= t->size().y * 0.5f;

    GraphController *controller = static_cast<GraphController *>(m_controller);

    Widget *widget = m_linksRender->creationLink();
    if(widget) {
        PortWidget *portWidget = dynamic_cast<PortWidget *>(widget);
        if(portWidget) {
            NodePort *p1 = portWidget->port();
            GraphNode *n1 = p1->m_node;

            m_editor->undoRedo()->push(new CreateNode(type, localPos.x, localPos.y, controller, g->node(n1), n1->portPosition(p1), p1->m_out));
        } else {
            NodeWidget *nodeWidget = dynamic_cast<NodeWidget *>(widget);
            if(nodeWidget) {
                m_editor->undoRedo()->push(new CreateNode(type, localPos.x, localPos.y, controller, g->node(nodeWidget->node()), -1, true));
            }
        }
        cancelLinkCreation();
    } else {
        m_editor->undoRedo()->push(new CreateNode(type, localPos.x, localPos.y, controller));
    }

    m_createMenu->hide();
    onInProgressFlag(false);
}
