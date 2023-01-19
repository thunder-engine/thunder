#ifndef CAMERAFUNCTIONS_H
#define CAMERAFUNCTIONS_H

#include "function.h"

class CameraPosition : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

public:
    Q_INVOKABLE CameraPosition() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Output", m_portColors[QMetaType::QVector3D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("g.cameraPosition.xyz");
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class CameraDirection : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

public:
    Q_INVOKABLE CameraDirection() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Output", m_portColors[QMetaType::QVector3D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("g.cameraTarget.xyz");
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class ScreenSize : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

public:
    Q_INVOKABLE ScreenSize() {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector4D, 0, "Output", m_portColors[QMetaType::QVector4D]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float, 1, "Width",    m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float, 2, "Height",   m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float, 3, "1/Width",  m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float, 4, "1/Height", m_portColors[QMetaType::Float]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(link.oport->m_name == "Width") {
            stack.append("g.cameraScreen.x");
        } else if(link.oport->m_name == "Height") {
            stack.append("g.cameraScreen.y");
        } else if(link.oport->m_name == "1/Width") {
            stack.append("g.cameraScreen.z");
        } else if(link.oport->m_name == "1/Height") {
            stack.append("g.cameraScreen.w");
        }

        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class ScreenPosition : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

    Q_PROPERTY(bool normalized READ normalized WRITE setNormalized NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ScreenPosition() :
            m_normalized(true) {
        m_ports.push_back(NodePort(this, true, QMetaType::QVector4D, 0, "Output", m_portColors[QMetaType::QVector4D]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float,     1, x, m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float,     2, y, m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float,     3, z, m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true, QMetaType::Float,     4, w, m_portColors[QMetaType::Float]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            code += QString("\tvec4 local%1 = gl_FragCoord").arg(depth) + (m_normalized ? " / g.cameraScreen;\n" : ";\n");
        }

        int32_t result = ShaderFunction::build(code, stack, link, depth, type);

        if(link.oport->m_name == x) {
            stack.append(convert("local" + QString::number(m_position), QMetaType::QVector4D, QMetaType::Float, 0));
        } else if(link.oport->m_name == y) {
            stack.append(convert("local" + QString::number(m_position), QMetaType::QVector4D, QMetaType::Float, 1));
        } else if(link.oport->m_name == z) {
            stack.append(convert("local" + QString::number(m_position), QMetaType::QVector4D, QMetaType::Float, 2));
        } else if(link.oport->m_name == w) {
            stack.append(convert("local" + QString::number(m_position), QMetaType::QVector4D, QMetaType::Float, 3));
        }

        return result;
    }

    bool normalized() const {
        return m_normalized;
    }

    void setNormalized(bool value) {
        m_normalized = value;
    }

protected:
    bool m_normalized;

};

#endif // CAMERAFUNCTIONS_H
