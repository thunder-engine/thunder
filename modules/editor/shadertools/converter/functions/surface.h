#ifndef SURFACE_H
#define SURFACE_H

#include "function.h"

class Fresnel : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

    Q_PROPERTY(float Power READ power WRITE setPower NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Fresnel() :
        m_power(5.0f) {

        m_ports.push_back(NodePort(this, false, QMetaType::QVector3D, 1, "Normal", m_portColors[QMetaType::QVector3D]));
        m_ports.push_back(NodePort(this, false, QMetaType::QVector3D, 2, "View Dir", m_portColors[QMetaType::QVector3D]));
        m_ports.push_back(NodePort(this, false, QMetaType::Float, 3, "Power", m_portColors[QMetaType::Float]));

        m_ports.push_back(NodePort(this, true,  QMetaType::Float, 0, "Output", m_portColors[QMetaType::Float]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            const AbstractNodeGraph::Link *nl = m_graph->findLink(this, port(1));
            const AbstractNodeGraph::Link *vl = m_graph->findLink(this, port(2));
            const AbstractNodeGraph::Link *pl = m_graph->findLink(this, port(3));

            QString normal("_n");
            if(nl) {
                int32_t type = 0;
                ShaderFunction *node = static_cast<ShaderFunction *>(nl->sender);
                uint32_t index = node->build(code, stack, *nl, depth, type);

                if(stack.isEmpty()) {
                    normal = convert("local" + QString::number(index), type, QMetaType::QVector3D);
                } else {
                    normal = convert(stack.pop(), type, QMetaType::QVector3D);
                }
            }

            QString view("_view");
            if(vl) {
                int32_t type = 0;
                ShaderFunction *node = static_cast<ShaderFunction *>(vl->sender);
                uint32_t index = node->build(code, stack, *vl, depth, type);

                if(stack.isEmpty()) {
                    view = convert("local" + QString::number(index), type, QMetaType::QVector3D);
                } else {
                    view = convert(stack.pop(), type, QMetaType::QVector3D);
                }
            }

            QString power = QString::number(m_power);
            if(pl) {
                int32_t type = 0;
                ShaderFunction *node = static_cast<ShaderFunction *>(pl->sender);
                uint32_t index = node->build(code, stack, *pl, depth, type);

                if(stack.isEmpty()) {
                    power = convert("local" + QString::number(index), type, QMetaType::Float);
                } else {
                    power = convert(stack.pop(), type, QMetaType::Float);
                }
            }

            // f0 = 0.04
            code.append(QString("float local%1 = 0.04 + (1.0 - 0.04) * pow(1.0 - dot(%2, -%3), %4);\n")
                        .arg(QString::number(depth), normal, view, power));
        }

        return ShaderFunction::build(code, stack, link, depth, type);
    }

    float power() const {
        return m_power;
    }

    void setPower(float power) {
        m_power = power;
        emit updated();
    }

private:
    float m_power;

};

class WorldBitangent : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldBitangent() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Output", m_portColors[QMetaType::QVector3D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_b");
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class WorldNormal : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldNormal() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Output", m_portColors[QMetaType::QVector3D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_n");
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class WorldPosition : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldPosition() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Output", m_portColors[QMetaType::QVector3D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_vertex.xyz");
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class WorldTangent : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldTangent() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Output", m_portColors[QMetaType::QVector3D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_t");
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

#endif // SURFACE_H
