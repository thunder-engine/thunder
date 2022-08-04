#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include <QObject>
#include <QVariant>
#include <QPoint>

#include <engine.h>

class ENGINE_EXPORT NodePort {
public:
    explicit NodePort(bool out, uint32_t type, int32_t pos, QString name, QVariant var = QVariant()) :
        m_out(out),
        m_type(type),
        m_pos(pos),
        m_name(name),
        m_var(var) {

    }

    bool m_out;

    uint32_t m_type;

    int32_t m_pos;

    QString m_name;

    QVariant m_var;
};

class ENGINE_EXPORT GraphNode : public QObject {
public:
    GraphNode() :
        root(false) {

    }

    bool root;

    QString type;

    QPoint pos;

    QList<NodePort *> ports;
};

#endif // GRAPHNODE_H
