#include "graphcontroller.h"

#include "graphview.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/groupwidget.h"

#include "actions/movenodes.h"
#include "actions/selectnodes.h"
#include "actions/deletenodes.h"

#include <input.h>
#include <components/camera.h>
#include <components/recttransform.h>

#include <pugixml.hpp>
#include <sstream>

GraphController::GraphController(GraphView *view) :
        m_graph(nullptr),
        m_view(view),
        m_zoom(1),
        m_dragWidget(nullptr) {

    m_activeCamera->setOrthographic(true);
}

AbstractNodeGraph *GraphController::graph() {
    return m_graph;
}

void GraphController::setGraph(AbstractNodeGraph *graph) {
    m_graph = graph;

    m_dragWidget = nullptr;

    m_selected.clear();
    m_softSelected.clear();

    GraphNode *defaultNode = m_graph->defaultNode();
    if(defaultNode) {
        selectNodes({ m_graph->node(defaultNode) });
    }
}

Object::ObjectList GraphController::selected() {
    Object::ObjectList result;
    for(auto it : m_selected) {
        result.push_back(m_graph->node(it));
    }
    return result;
}

void GraphController::selectNodes(const std::list<int32_t> &nodes) {
    for(auto it : m_selected) {
        GraphNode *node = m_graph->node(it);
        if(node) {
            NodeWidget *widget = dynamic_cast<NodeWidget *>(node->widget());
            if(widget) {
                widget->setSelected(false);
            }
        }
    }

    m_selected = nodes;

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

    emit m_view->objectsSelected(list);
}

void GraphController::composeLinks() {
    m_view->composeLinks();
}

void GraphController::copySelected() {
    pugi::xml_document doc;

    pugi::xml_node nodesElement = doc.append_child("nodes");
    for(auto it : m_selected) {
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
    if(nodes != m_selected) {
        std::list<int32_t> local = nodes;
        if(additive) {
            local.insert(local.end(), m_selected.begin(), m_selected.end());
        }

        m_view->undoRedo()->push(new SelectNodes(local, this));
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

void GraphController::deleteNode() {
    std::list<int32_t> selection;
    for(auto it : m_selected) {
        if(m_graph->node(it)->isRemovable()) {
            selection.push_back(it);
        }
    }
    if(!selection.empty()) {
        GraphNode *defaultNode = m_graph->defaultNode();
        std::list<int32_t> nodeList;
        Object::ObjectList list;
        if(defaultNode) {
            nodeList.push_back(m_graph->node(defaultNode));
            list.push_back(defaultNode);
        }

        // The order of calls is correct
        emit m_view->objectsSelected(list);
        m_view->undoRedo()->push(new DeleteNodes(selection, this));
        selectNodes(nodeList);
    }
}

void GraphController::cancelDrag() {
    auto selectedOrigin = m_selectedOrigins.begin();
    for(auto it : m_selected) {
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

void GraphController::beginDrag() {
    m_selectedOrigins.clear();
    for(auto it : m_selected) {
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
}

void GraphController::update() {
    if((Input::isMouseButtonUp(Input::MOUSE_RIGHT) && !m_cameraInMove) ||
       (Input::isMouseButtonUp(Input::MOUSE_LEFT) && m_view->isCreationLink())) {

        m_view->showMenu();
    }

    CameraController::update();

    Vector4 mousePosition(Input::mousePosition());
    RectTransform *parentRect = static_cast<RectTransform *>(m_view->view().transform());
    Vector2 parentSize(parentRect->size());
    Vector3 parentScale(parentRect->scale());
    Vector3 localPos(parentRect->worldTransform().inverse() * Vector3(mousePosition.x, mousePosition.y, 0.0f));

    Vector2 pos(localPos.x - (parentSize.x / parentScale.x) * 0.5f,
                localPos.y - (parentSize.y / parentScale.y) * 0.5f);

    NodeWidget *hovered = nullptr;
    Qt::CursorShape shape = Qt::ArrowCursor;
    for(auto node : m_graph->nodes()) {
        NodeWidget *widget = static_cast<NodeWidget *>(node->widget());
        RectTransform *rect = widget->rectTransform();
        if(rect->isHovered(mousePosition.x, mousePosition.y)) {
            hovered = widget;
        }
        GroupWidget *group = dynamic_cast<GroupWidget *>(widget);
        if(group) {
            Qt::CursorShape s = static_cast<Qt::CursorShape>(group->cursorShape());
            if(s != Qt::ArrowCursor) {
                shape = s;
            }
        }
    }

    if(shape != Qt::ArrowCursor) {
        emit setCursor(QCursor(shape));
    } else {
        emit unsetCursor();
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        if(hovered == nullptr && !m_view->isCreationLink()) {
            m_originMousePos = pos;
        }
    }

    if(m_view->rubberBand()->isEnabled()) {
        rubberBandBehavior(pos);
    } else if(!m_view->isCreationLink()) {
        if(hovered && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
            m_originMousePos = pos;
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
            if(m_dragWidget) {
                std::list<NodeWidget *> list;
                for(auto it : m_selected) {
                    list.push_back(static_cast<NodeWidget *>(m_graph->node(it)->widget()));
                }
                for(auto it : m_softSelected) {
                    list.push_back(static_cast<NodeWidget *>(m_graph->node(it)->widget()));
                }
                m_view->undoRedo()->push(new MoveNodes(list, this));

                m_originMousePos = pos;
                m_dragWidget = nullptr;
            } else if(hovered) {
                onSelectNodes({ m_graph->node(hovered->node()) }, Input::isKey(Input::KEY_LEFT_CONTROL));
                m_softSelected.clear();
            } else {
                std::list<int32_t> list;
                GraphNode *defaultNode = m_graph->defaultNode();
                if(defaultNode) {
                    list.push_back(m_graph->node(defaultNode));
                }
                onSelectNodes(list);
            }
        }

        if(m_dragWidget) {
            RectTransform *dragRect = m_dragWidget->rectTransform();

            if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                Vector2 deltaPos = pos - m_originMousePos;

                int snap = m_view->gridCell();

                auto selectedOrigin = m_selectedOrigins.begin();
                for(auto it : m_selected) {
                    GraphNode *node = m_graph->node(it);
                    RectTransform *rect = m_graph->node(it)->widget()->rectTransform();
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

            if(Input::isMouseButtonDown(Input::MOUSE_RIGHT)) { // Cancel drag
                cancelDrag();
            }
        } else if(Input::isMouseButton(Input::MOUSE_LEFT) && (pos - m_originMousePos).length() > 5.0f) { // Drag sensor = 5.0f
            if(hovered) {
                if(m_selected.empty() || !hovered->isSelected()) { // Select on drag
                    onSelectNodes({ m_graph->node(hovered->node()) });
                }

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

                        if(r.contains(n) && std::find(m_selected.begin(), m_selected.end(), m_graph->node(widget->node())) == m_selected.end()) {
                            m_softSelected.push_back(m_graph->node(widget->node()));
                        }
                    }
                }

                beginDrag();
                m_dragWidget = hovered;
            } else {
                m_view->rubberBand()->setEnabled(true);
                m_view->rubberBand()->raise();

                RectTransform *rect = m_view->rubberBand()->rectTransform();
                rect->setPosition(Vector3(m_originMousePos, 0.0f));
                rect->setSize(Vector2(0.0f));
            }
        }

        if(!m_selected.empty() && Input::isKeyDown(Input::KEY_DELETE)) {
            deleteNode();
        }
    }
}

void GraphController::cameraMove(const Vector3 &delta) {
    CameraController::cameraMove(delta);

    Transform *t = m_view->view().transform();
    t->setPosition(t->position() + Vector3(m_delta, 0.0f));
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
