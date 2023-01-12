#include "graphnode.h"

GraphNode::GraphNode() :
        m_userData(nullptr),
        m_graph(nullptr) {

}

AbstractNodeGraph *GraphNode::graph() const {
    return m_graph;
}

void GraphNode::setGraph(AbstractNodeGraph *graph) {
    m_graph = graph;
}

NodePort *GraphNode::port(int position) {
    for(auto &it : m_ports) {
        if(it.m_pos == position) {
            return &it;
        }
    }
    return nullptr;
}

int GraphNode::portPosition(NodePort *port) {
    return port->m_pos;
}

std::string GraphNode::type() const {
    return m_type;
}

void GraphNode::setType(const std::string &type) {
    m_type = type;
}

Vector2 GraphNode::defaultSize() const {
    return Vector2(200.0f, 30.0f);
}

Vector4 GraphNode::color() const {
    return Vector4(1.0f);
}

Vector2 GraphNode::position() const {
    return m_pos;
}

void GraphNode::setPosition(const Vector2 &position) {
    m_pos = position;
}

void *GraphNode::widget() const {
    return m_userData;
}
void GraphNode::setWidget(void *widget) {
    m_userData = widget;
}

bool GraphNode::isState() const {
    return false;
}

std::vector<NodePort> &GraphNode::ports() {
    return m_ports;
}
