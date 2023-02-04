#ifndef TIME_H
#define TIME_H

#include "function.h"

class Time : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

    Q_PROPERTY(float scale READ scale WRITE setScale NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Time() :
            m_scale(1.0f) {

        m_ports.push_back(NodePort(this, true, QMetaType::Float, 0, "Output", m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, false, QMetaType::Float, 1, "Scale", m_portColors[QMetaType::Float]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {

            QString scale = QString::number(m_scale);
            const AbstractNodeGraph::Link *l = m_graph->findLink(this, &m_ports.back());
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);
                if(node) {
                    int32_t l_type = QMetaType::Float;
                    int32_t index = node->build(code, stack, *l, depth, l_type);
                    if(index >= 0) {
                        if(stack.isEmpty()) {
                            scale = convert("local" + QString::number(index), l_type, QMetaType::Float);
                        } else {
                            scale = convert(stack.pop(), type, QMetaType::Float);
                        }
                    }
                }
            }

            QString value = getVariable() + " * " + scale;

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(type, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    virtual QString getVariable() const {
        return "g.time";
    }

    float scale() const { return m_scale; }
    void setScale(float scale) { m_scale = scale; emit updated(); }

private:
    float m_scale;

};

class DeltaTime : public Time {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

public:
    Q_INVOKABLE DeltaTime() { }

    QString getVariable() const override {
        return "g.deltaTime";
    }
};

class CosTime : public Time {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

public:
    Q_INVOKABLE CosTime() { }

    QString getVariable() const override {
        return "cos(g.time)";
    }
};

class SinTime : public Time {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

public:
    Q_INVOKABLE SinTime() { }

    QString getVariable() const override {
        return "sin(g.time)";
    }
};

#endif // TIME_H
