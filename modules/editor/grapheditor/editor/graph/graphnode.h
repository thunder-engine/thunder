#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include <QObject>
#include <QVariant>
#include <QPoint>

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
    explicit NodePort(GraphNode *node, bool out, uint32_t type, int32_t pos, QString name, QVariant var = QVariant()) :
        m_out(out),
        m_type(type),
        m_pos(pos),
        m_name(name),
        m_var(var),
        m_node(node) {

    }

    bool m_out;

    uint32_t m_type;

    int32_t m_pos;

    QString m_name;

    QVariant m_var;

    GraphNode *m_node;
};

class NODEGRAPH_EXPORT GraphNode : public QObject {
public:
    GraphNode() :
        m_root(false),
        m_graph(nullptr) {

    }

    bool m_root;

    QString m_type;

    QPoint m_pos;

    QList<NodePort *> m_ports;

    AbstractNodeGraph *m_graph;
};

#endif // GRAPHNODE_H
