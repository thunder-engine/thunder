#include "graphcontroller.h"

#include "graphview.h"
#include "graphwidgets/linksrender.h"
#include "graphwidgets/nodewidget.h"
#include "graphwidgets/groupwidget.h"
#include "graphwidgets/portwidget.h"

#include <input.h>
#include <components/gui/recttransform.h>

GraphController::GraphController(GraphView *view) :
        m_focusedWidget(nullptr),
        m_graph(nullptr),
        m_view(view),
        m_drag(false) {

}

NodeWidget *GraphController::focusNode() {
    return m_focusedWidget;
}

void GraphController::setFocusNode(NodeWidget *widget) {
    m_focusedWidget = widget;
}

AbstractNodeGraph *GraphController::graph() {
    return m_graph;
}

void GraphController::setGraph(AbstractNodeGraph *graph) {
    m_graph = graph;
}

const QList<QObject *> &GraphController::selectedItems() const {
    return m_selectedItems;
}

void GraphController::composeLinks() {
    m_view->composeLinks();
}

void GraphController::update() {
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
        emit setCursor(QCursor(shape));
    } else {
        emit unsetCursor();
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        if(m_focusedWidget == nullptr &&
           shape == Qt::ArrowCursor && m_view->linksRender()->creationLink() == nullptr) {
            m_view->rubberBand()->setEnabled(true);
            m_view->rubberBand()->raise();
            RectTransform *rect = m_view->rubberBand()->rectTransform();

            m_rubberOrigin = Vector2(pos.x, pos.y);
            rect->setPosition(Vector3(m_rubberOrigin, 0.0f));
            rect->setSize(Vector2());
        }
    } else if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        m_view->showMenu();
    }

    if(m_view->rubberBand()->isEnabled()) {
        QRect r(QPoint(MIN(m_rubberOrigin.x, pos.x), MIN(m_rubberOrigin.y, pos.y)),
                QPoint(MAX(m_rubberOrigin.x, pos.x), MAX(m_rubberOrigin.y, pos.y)));

        RectTransform *transform = m_view->rubberBand()->rectTransform();
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

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT) || m_view->linksRender()->creationLink() != nullptr) {
            m_view->rubberBand()->setEnabled(false);
            if(!list.empty()) {
                m_selectedItems = list;
                emit m_view->itemsSelected(m_selectedItems);
            } else {
                for(auto it : qAsConst(m_selectedItems)) {
                    GraphNode *node = static_cast<GraphNode *>(it);
                    reinterpret_cast<NodeWidget *>(node->widget())->setSelected(false);
                }
                m_selectedItems.clear();
                m_softSelectedItems.clear();
                emit m_view->itemsSelected({m_graph->rootNode()});
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
                        emit m_view->itemsSelected(m_selectedItems);
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

                float snap = m_view->gridCell();
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
                m_view->composeLinks();
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
                m_view->composeLinks();
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
                    emit m_view->itemsSelected(m_selectedItems);
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
            emit m_view->itemsSelected({m_graph->rootNode()});
            m_graph->deleteNodes(selection);
            m_selectedItems.clear();
        }
    }
}

bool GraphController::isSelected(NodeWidget *widget) const {
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

MoveNodes::MoveNodes(const list<NodeWidget *> &selection, GraphController *ctrl, const QString &name, QUndoCommand *parent) :
        m_controller(ctrl),
        UndoGraph(ctrl->graph(), name, parent) {

    m_indices.reserve(selection.size());
    m_points.reserve(selection.size());
    for(auto &it : selection) {
        m_indices.push_back(m_graph->node(it->node()));

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
    static_cast<GraphController *>(m_controller)->composeLinks();
    m_points = positions;
}
