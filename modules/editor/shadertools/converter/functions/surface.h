#ifndef SURFACE_H
#define SURFACE_H

#include "function.h"

class Fresnel : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

    Q_PROPERTY(float Power READ power WRITE setPower NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Fresnel() :
            m_power(5.0f) {

        m_inputs.push_back(make_pair("Normal", QMetaType::QVector3D));
        m_inputs.push_back(make_pair("View Dir", QMetaType::QVector3D));
        m_inputs.push_back(make_pair("Power", QMetaType::Float));

        m_outputs.push_back(make_pair("Output", QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList args = getArguments(code, stack, depth, type);

            // f0 = 0.04
            code.append(QString("float local%1 = 0.04 + (1.0 - 0.04) * pow(1.0 - dot(%2, -%3), %4);\n")
                        .arg(QString::number(depth), args[0], args[1], args[2]));
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const string &key, uint32_t &) const override {
        if(key == "Normal") {
            return "_n";
        } else if(key == "View Dir") {
            return "_view";
        }
        return QString::number(m_power);
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

class SurfaceDepth : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE SurfaceDepth() {
        m_inputs.push_back(make_pair("Vertex Position", QMetaType::QVector3D));

        m_outputs.push_back(make_pair("Depth", QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList args = getArguments(code, stack, depth, type);

            code.append(QString("float local%1 = (- _modelView * vec4(%2, 1.0f)).z;\n")
                        .arg(QString::number(depth), args[0]));
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const string &key, uint32_t &) const override {
        return "_vertex.xyz";
    }
};

class WorldBitangent : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldBitangent() {
        m_outputs.push_back(make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_b");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class WorldNormal : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldNormal() {
        m_outputs.push_back(make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_n");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class WorldPosition : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldPosition() {
        m_outputs.push_back(make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_vertex.xyz");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class WorldTangent : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Surface")

public:
    Q_INVOKABLE WorldTangent() {
        m_outputs.push_back(make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_t");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

#endif // SURFACE_H
