#include "graphcontroller.h"

#include "graphview.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/groupwidget.h"
#include "graphwidgets/portwidget.h"

#include "actions/movenodes.h"
#include "actions/selectnodes.h"
#include "actions/deletenodes.h"
#include "actions/deletelinks.h"

#include <input.h>
#include <components/camera.h>
#include <components/recttransform.h>

#include <editor/viewport/handletools.h>

#include <pugixml.hpp>
#include <sstream>

GraphController::GraphController(GraphView *view) :
        m_dragWidget(nullptr),
        m_graph(nullptr),
        m_view(view),
        m_zoom(1) {

    m_activeCamera->setOrthographic(true);
}

AbstractNodeGraph *GraphController::graph() {
    return m_graph;
}

void GraphController::setGraph(AbstractNodeGraph *graph) {
    m_graph = graph;

    m_dragWidget = nullptr;

    m_selectedNodes.clear();
    m_softSelected.clear();

    GraphNode *defaultNode = m_graph->defaultNode();
    if(defaultNode) {
        selectNodes({ m_graph->node(defaultNode) });
    }
}

Object::ObjectList GraphController::selected() {
    Object::ObjectList result;
    for(auto it : m_selectedNodes) {
        result.push_back(m_graph->node(it));
    }
    if(result.empty()) {
        for(auto it : m_selectedLinks) {
            result.push_back(m_graph->link(it));
        }
    }
    return result;
}

std::list<int32_t> GraphController::selectedLinks() const {
    return m_selectedLinks;
}

void GraphController::selectNodes(const std::list<int32_t> &nodes) {
    for(auto it : m_selectedNodes) {
        GraphNode *node = m_graph->node(it);
        if(node) {
            NodeWidget *widget = dynamic_cast<NodeWidget *>(node->widget());
            if(widget) {
                widget->setSelected(false);
            }
        }
    }
    m_selectedNodes = nodes;

    m_selectedLinks.clear();
    composeLinks();

    Object::ObjectList list;
    for(auto it : nodes) {
        GraphNode *node = m_graph->node(it);
        if(node) {
            NodeWidget *widget = dynamic_cast<NodeWidget *>(node->widget());
            if(widget) {
                widget->setSelected(true);
            }

            list.push_back(node);
        }
    }

    if(list.empty()) {
        emit m_view->objectsSelected({m_graph});
    } else {
        emit m_view->objectsSelected(list);
    }
}

void GraphController::selectLinks(const std::list<int32_t> &links) {
    for(auto it : m_selectedNodes) {
        GraphNode *node = m_graph->node(it);
        if(node) {
            NodeWidget *widget = dynamic_cast<NodeWidget *>(node->widget());
            if(widget) {
                widget->setSelected(false);
            }
        }
    }
    m_selectedNodes.clear();

    m_selectedLinks = links;
    composeLinks();

    Object::ObjectList list;
    for(auto it : links) {
        GraphLink *graphLink = m_graph->link(it);
        if(graphLink) {
            list.push_back(graphLink);
        }
    }

    emit m_view->objectsSelected(list);
}

void GraphController::composeLinks() {
    m_view->composeLinks();
}

void GraphController::copySelected() {
    pugi::xml_document doc;

    pugi::xml_node nodesElement = doc.append_child("nodes");
    for(auto it : m_selectedNodes) {
        GraphNode *node = m_graph->node(it);
        if(node->isRemovable()) {
            pugi::xml_node element = nodesElement.append_child("node");
            node->toXml(element);
        }
    }

    std::stringstream stream;
    doc.save(stream);
    m_copyData = stream.str();

    emit copied();
}

const std::string &GraphController::copyData() const {
    return m_copyData;
}

void GraphController::onSelectNodes(const std::list<int32_t> &nodes, bool additive) {
    if(nodes != m_selectedNodes || !m_selectedLinks.empty()) {
        std::list<int32_t> local = nodes;
        if(additive) {
            local.insert(local.end(), m_selectedNodes.begin(), m_selectedNodes.end());
        }

        m_view->undoRedo()->push(new SelectNodes(local, {}, this));
    }
}

void GraphController::onSelectLinks(const std::list<int32_t> &links, bool additive) {
    if(links != m_selectedLinks || !m_selectedNodes.empty()) {
        std::list<int32_t> local = links;
        if(additive) {
            local.insert(local.end(), m_selectedLinks.begin(), m_selectedLinks.end());
        }

        m_view->undoRedo()->push(new SelectNodes({}, links, this));
    }
}

void GraphController::rubberBandBehavior(const Vector2 &pos) {
    QRect r(QPoint(MIN(m_originMousePos.x, pos.x), MIN(m_originMousePos.y, pos.y)),
            QPoint(MAX(m_originMousePos.x, pos.x), MAX(m_originMousePos.y, pos.y)));

    RectTransform *transform = m_view->rubberBand()->rectTransform();
    transform->setPosition(Vector3(r.x(), r.y(), 0.0f));
    transform->setSize(Vector2(r.width(), r.height()));

    std::list<int32_t> list;
    for(auto node : m_graph->nodes()) {
        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
        if(widget) {
            transform = widget->rectTransform();
            QRect n(QPoint(transform->position().x, transform->position().y - transform->size().y),
                    QSize(transform->size().x, transform->size().y));

            if(r.intersects(n)) {
                widget->setSelected(true);
                list.push_back(m_graph->node(node));
            } else {
                widget->setSelected(false);
            }
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT) || m_view->isCreationLink()) {
        m_view->rubberBand()->setEnabled(false);
        if(list.empty()) {
            GraphNode *defaultNode = m_graph->defaultNode();
            if(defaultNode) {
                list.push_back(m_graph->node(defaultNode));
            }
            m_softSelected.clear();
        }
        onSelectNodes(list);
    }
}

void GraphController::deleteNodes() {
    std::list<int32_t> selection;
    for(auto it : m_selectedNodes) {
        if(m_graph->node(it)->isRemovable()) {
            selection.push_back(it);
        }
    }
    if(!selection.empty()) {
        m_view->undoRedo()->push(new DeleteNodes(selection, this));
    }
}

void GraphController::deleteLinks() {
    m_view->undoRedo()->push(new DeleteLinks(m_selectedLinks, this));
}

void GraphController::dragSelection(const Vector2 &position) {
    Vector2 deltaPos(position - m_originMousePos);

    int snap = m_view->gridCell();

    auto selectedOrigin = m_selectedOrigins.begin();
    for(auto it : m_selectedNodes) {
        GraphNode *node = m_graph->node(it);
        RectTransform *rect = node->widget()->rectTransform();
        Vector2 newPos(*selectedOrigin + deltaPos);

        for(int n = 0; n < 2; n++) {
            newPos[n] = snap * int(newPos[n] / (float)snap);
        }

        rect->setPosition(Vector3(newPos, 0.0f));
        ++selectedOrigin;
    }

    auto softOrigin = m_softOrigins.begin();
    for(auto it : m_softSelected) {
        GraphNode *node = m_graph->node(it);
        RectTransform *rect = node->widget()->rectTransform();
        Vector2 newPos(*softOrigin + deltaPos);

        for(int n = 0; n < 2; n++) {
            newPos[n] = snap * int(newPos[n] / (float)snap);
        }

        rect->setPosition(Vector3(newPos, 0.0f));
        ++softOrigin;
    }
    m_view->composeLinks();
}

void GraphController::cancelDrag() {
    auto selectedOrigin = m_selectedOrigins.begin();
    for(auto it : m_selectedNodes) {
        GraphNode *node = m_graph->node(it);
        RectTransform *rect = node->widget()->rectTransform();
        rect->setPosition(Vector3(*selectedOrigin, 0.0f));
        ++selectedOrigin;
    }
    auto softOrigin = m_softOrigins.begin();
    for(auto it : m_softSelected) {
        GraphNode *node = m_graph->node(it);
        RectTransform *rect = node->widget()->rectTransform();
        rect->setPosition(Vector3(*softOrigin, 0.0f));
        ++softOrigin;
    }
    m_view->composeLinks();
    m_softSelected.clear();
    m_dragWidget = nullptr;
}

void GraphController::beginDrag(NodeWidget *hovered) {
    m_softSelected.clear();

    GroupWidget *group = dynamic_cast<GroupWidget *>(hovered);
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

            if(r.contains(n) && std::find(m_selectedNodes.begin(), m_selectedNodes.end(), m_graph->node(widget->node())) == m_selectedNodes.end()) {
                m_softSelected.push_back(m_graph->node(widget->node()));
            }
        }
    }

    m_selectedOrigins.clear();
    for(auto it : m_selectedNodes) {
        GraphNode *node = m_graph->node(it);
        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
        RectTransform *rect = widget->rectTransform();
        if(rect) {
            m_selectedOrigins.push_back(rect->position());
        }
    }

    m_softOrigins.clear();
    for(auto it : m_softSelected) {
        GraphNode *node = m_graph->node(it);
        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
        RectTransform *rect = widget->rectTransform();
        if(rect) {
            m_softOrigins.push_back(rect->position());
        }
    }

    m_dragWidget = hovered;
}

void GraphController::endDrag() {
    std::list<NodeWidget *> list;
    for(auto it : m_selectedNodes) {
        list.push_back(static_cast<NodeWidget *>(m_graph->node(it)->widget()));
    }
    for(auto it : m_softSelected) {
        list.push_back(static_cast<NodeWidget *>(m_graph->node(it)->widget()));
    }
    m_view->undoRedo()->push(new MoveNodes(list, this));

    m_dragWidget = nullptr;
}

void GraphController::update() {
    if(Input::isMouseButtonUp(Input::MOUSE_RIGHT) && !m_cameraInMove) {
        m_view->showMenu();
    }

    CameraController::update();

    Vector4 mousePosition(Input::mousePosition());

    RectTransform *parentRect = static_cast<RectTransform *>(m_view->view().transform());
    Vector2 parentSize(parentRect->size());
    Vector2 parentScale(parentRect->scale());
    Vector2 localPos(parentRect->worldTransform().inverse() * Vector3(mousePosition.x, mousePosition.y, 0.0f));
    Vector2 pos(localPos.x - (parentSize.x / parentScale.x) * 0.5f,
                localPos.y - (parentSize.y / parentScale.y) * 0.5f);

    NodeWidget *hovered = hoveredNode(mousePosition.x, mousePosition.y);

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        if(hovered == nullptr && !m_view->isCreationLink()) {
            m_originMousePos = pos;
        }
    }

    Frame *rubberBand = m_view->rubberBand();

    if(rubberBand->isEnabled()) {
        rubberBandBehavior(pos);
    } else if(!m_view->isCreationLink()) {
        if(hovered && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
            m_originMousePos = pos;
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
            if(m_dragWidget) {
                endDrag();
                m_originMousePos = pos;
            } else if(hovered) {
                onSelectNodes({ m_graph->node(hovered->node()) }, Input::isKey(Input::KEY_LEFT_CONTROL));
                m_softSelected.clear();
            } else {
                int link = hoveredLink(localPos.x, localPos.y);
                if(link > -1) {
                    onSelectLinks({link});
                } else {
                    onSelectNodes({});
                }
            }
        }

        if(m_dragWidget) {
            if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                dragSelection(pos);
            }

            if(Input::isMouseButtonDown(Input::MOUSE_RIGHT)) { // Cancel drag
                cancelDrag();
            }
        } else if(Input::isMouseButton(Input::MOUSE_LEFT) && (pos - m_originMousePos).length() > 5.0f) { // Drag sensor = 5.0f
            if(hovered) {
                if(m_selectedNodes.empty() || !hovered->isSelected()) { // Select on drag
                    onSelectNodes({ m_graph->node(hovered->node()) });
                }

                beginDrag(hovered);
            } else {
                rubberBand->setEnabled(true);
                rubberBand->raise();

                RectTransform *rect = rubberBand->rectTransform();
                rect->setPosition(Vector3(m_originMousePos, 0.0f));
                rect->setSize(Vector2(0.0f));
            }
        }

        if(Input::isKeyDown(Input::KEY_DELETE)) {
            if(!m_selectedNodes.empty()) {
                deleteNodes();
            }

            if(!m_selectedLinks.empty()) {
                deleteLinks();
            }
        }
    } else {
        if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
            m_view->cancelLinkCreation();
        }
    }
}

NodeWidget *GraphController::hoveredNode(float mouseX, float mouseY) {
    NodeWidget *hovered = nullptr;

    for(auto node : m_graph->nodes()) {
        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
        if(widget) {
            RectTransform *rect = widget->header()->rectTransform();
            if(rect->isHovered(mouseX, mouseY)) {
                hovered = widget;
            }
        }
    }

    return hovered;
}

PortWidget *GraphController::hoveredPort(float mouseX, float mouseY) {
    for(auto node : m_graph->nodes()) {
        for(auto &port : node->ports()) {
            PortWidget *widget = static_cast<PortWidget *>(port.m_widget);
            if(widget) {
                RectTransform *rect = widget->rectTransform();
                if(rect->isHovered(mouseX, mouseY)) {
                    return widget;
                }
            }
        }
    }

    return nullptr;
}

int GraphController::hoveredLink(float mouseX, float mouseY) {
    RectTransform *parentTransform = static_cast<RectTransform *>(m_view->view().transform());
    Matrix4 worlToView(parentTransform->worldTransform().inverse());

    for(auto it : m_graph->links()) {
        Vector3Vector points;
        if(it->iport) {
            Vector3 b, e;
            PortWidget *widget = static_cast<PortWidget *>(it->oport->m_widget);
            if(widget) {
                RectTransform *rect = widget->knob()->rectTransform();
                b = worlToView * (rect->worldTransform() * Vector3(rect->size() * 0.5f, 0.0f));
            }

            widget = static_cast<PortWidget *>(it->iport->m_widget);
            if(widget) {
                RectTransform *rect = widget->knob()->rectTransform();
                e = worlToView * (rect->worldTransform() * Vector3(rect->size() * 0.5f, 0.0f));
            }

            points = Mathf::pointsCurve(b, e, Vector3(b.x + 40.0f, b.y, b.z), Vector3(e.x - 40.0f, e.y, e.z), 10);
        } else {
            RectTransform *rect = it->sender->widget()->rectTransform();
            points.push_back(worlToView * (rect->worldTransform() * Vector3(rect->size() * 0.5f, 0.0f)));

            rect = it->receiver->widget()->rectTransform();
            points.push_back(worlToView * (rect->worldTransform() * Vector3(rect->size() * 0.5f, 0.0f)));
        }

        float distance = HandleTools::distanceToPath(Matrix4(), points, Vector2(mouseX, mouseY), false);
        if(distance < 10.0f) {
            return m_graph->link(it);
        }
    }

    return -1;
}

void GraphController::cameraMove(const Vector3 &delta) {
    CameraController::cameraMove(delta);

    Transform *t = m_view->view().transform();
    t->setPosition(t->position() + Vector3(m_mouseDelta, 0.0f));
}

void GraphController::cameraZoom(float delta) {
    m_zoom += (delta > 0) ? -1 : 1;

    if(m_zoom >= 1 && m_zoom <= 10) {
        RectTransform *t = static_cast<RectTransform *>(m_view->view().transform());
        Vector3 world(m_activeCamera->unproject(t->position()));

        float scale = 1.1f - ((float)m_zoom / 10.0f);

        m_activeCamera->setOrthoSize(m_screenSize.y / scale);

        t->setScale(Vector3(scale, scale, 1.0f));
        t->setPosition(Vector3(m_activeCamera->project(world), 0.0f));
    } else {
        m_zoom = CLAMP(m_zoom, 1, 10);
    }
}

void GraphController::resize(int32_t width, int32_t height) {
    CameraController::resize(width, height);

    float scale = 1.1f - ((float)m_zoom / 10.0f);

    m_activeCamera->setOrthoSize(height / scale);
}
