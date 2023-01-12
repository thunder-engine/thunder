#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include <QObject>
#include <QVariant>

#include <amath.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef NODEGRAPH_LIBRARY
        #define NODEGRAPH_EXPORT __declspec(dllexport)
    #else
        #define NODEGRAPH_EXPORT __declspec(dllimport)
    #endif
#else
    #define NODEGRAPH_EXPORT
#endif

class AbstractNodeGraph;
class GraphNode;

class NODEGRAPH_EXPORT NodePort {
public:
    explicit NodePort(GraphNode *node, bool out, uint32_t type, int32_t pos, std::string name, const Vector4 &color, QVariant var = QVariant()) :
            m_out(out),
            m_type(type),
            m_pos(pos),
            m_name(name),
            m_var(var),
            m_node(node),
            m_userData(nullptr),
            m_color(color) {

    }

    GraphNode *m_node;

    bool m_out;

    uint32_t m_type;

    int32_t m_pos;

    std::string m_name;

    Vector4 m_color;

    std::string m_hints;

    QVariant m_var = QVariant();

    void *m_userData = nullptr;

};

class NODEGRAPH_EXPORT GraphNode : public QObject {
public:
    GraphNode();

    AbstractNodeGraph *graph() const;
    void setGraph(AbstractNodeGraph *graph);

    NodePort *port(int position);

    int portPosition(NodePort *port);

    std::string type() const;
    void setType(const std::string &type);

    virtual Vector2 defaultSize() const;
    virtual Vector4 color() const;

    Vector2 position() const;

    void setPosition(const Vector2 &position);

    void *widget() const;
    void setWidget(void *widget);

    virtual bool isState() const;

    std::vector<NodePort> &ports();

protected:
    std::string m_type;

    Vector2 m_pos;

    void *m_userData;

    std::vector<NodePort> m_ports;

    AbstractNodeGraph *m_graph;

};

#endif // GRAPHNODE_H
