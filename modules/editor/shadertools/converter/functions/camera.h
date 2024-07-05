#ifndef CAMERAFUNCTIONS_H
#define CAMERAFUNCTIONS_H

#include "function.h"

class CameraPosition : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

public:
    Q_INVOKABLE CameraPosition() {
        m_outputs.push_back(std::make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("g.cameraPosition.xyz");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class CameraDirection : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

public:
    Q_INVOKABLE CameraDirection() {
        m_outputs.push_back(std::make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("g.cameraTarget.xyz");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ScreenSize : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

public:
    Q_INVOKABLE ScreenSize() {
        m_outputs.push_back(std::make_pair("Output", QMetaType::QVector4D));
        m_outputs.push_back(std::make_pair("Width", QMetaType::Float));
        m_outputs.push_back(std::make_pair("Height", QMetaType::Float));
        m_outputs.push_back(std::make_pair("1/Width", QMetaType::Float));
        m_outputs.push_back(std::make_pair("1/Height", QMetaType::Float));
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

        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ScreenPosition : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

    Q_PROPERTY(bool normalized READ normalized WRITE setNormalized NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ScreenPosition() :
            m_normalized(true) {
        m_outputs.push_back(std::make_pair("Output", QMetaType::QVector4D));
        m_outputs.push_back(std::make_pair(x, QMetaType::Float));
        m_outputs.push_back(std::make_pair(y, QMetaType::Float));
        m_outputs.push_back(std::make_pair(z, QMetaType::Float));
        m_outputs.push_back(std::make_pair(w, QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            code += QString("\tvec4 local%1 = gl_FragCoord").arg(depth) + (m_normalized ? " / g.cameraScreen;\n" : ";\n");
        }

        int32_t result = ShaderNode::build(code, stack, link, depth, type);

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
        emit updated();
    }

protected:
    bool m_normalized;

};

class ProjectionMatrix : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Camera")

    Q_PROPERTY(bool inverted READ inverted WRITE setInverted NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ProjectionMatrix() :
            m_inverted(false) {

        m_outputs.push_back(std::make_pair("Output", QMetaType::QMatrix4x4));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_inverted) {
            stack.push("g.cameraProjectionInv");
        } else {
            stack.push("g.cameraProjection");
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    bool inverted() const {
        return m_inverted;
    }

    void setInverted(bool value) {
        m_inverted = value;
        emit updated();
    }

protected:
    bool m_inverted;

};

#endif // CAMERAFUNCTIONS_H
