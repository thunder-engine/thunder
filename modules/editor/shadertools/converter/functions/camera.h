#ifndef CAMERAFUNCTIONS_H
#define CAMERAFUNCTIONS_H

#include "function.h"

class CameraPosition : public ShaderNode {
    A_OBJECT(CameraPosition, ShaderNode, Shader/Camera)

public:
    CameraPosition() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("g.cameraPosition.xyz");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class CameraDirection : public ShaderNode {
    A_OBJECT(CameraDirection, ShaderNode, Shader/Camera)

public:
    CameraDirection() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("g.cameraTarget.xyz");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ScreenSize : public ShaderNode {
    A_OBJECT(ScreenSize, ShaderNode, Shader/Camera)

public:
    ScreenSize() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR4));
        m_outputs.push_back(std::make_pair("Width", MetaType::FLOAT));
        m_outputs.push_back(std::make_pair("Height", MetaType::FLOAT));
        m_outputs.push_back(std::make_pair("1/Width", MetaType::FLOAT));
        m_outputs.push_back(std::make_pair("1/Height", MetaType::FLOAT));
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
    A_OBJECT(ScreenPosition, ShaderNode, Shader/Camera)

    A_PROPERTIES(
        A_PROPERTY(bool, normalized, ScreenPosition::normalized, ScreenPosition::setNormalized)
    )

public:
    ScreenPosition() :
            m_normalized(true) {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR4));
        m_outputs.push_back(std::make_pair(x, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(y, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(z, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(w, MetaType::FLOAT));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            code += QString("\tvec4 local%1 = gl_FragCoord").arg(depth) + (m_normalized ? " / g.cameraScreen;\n" : ";\n");
        }

        int32_t result = ShaderNode::build(code, stack, link, depth, type);

        if(link.oport->m_name == x) {
            stack.append(convert("local" + QString::number(m_position), MetaType::VECTOR4, MetaType::FLOAT, 0));
        } else if(link.oport->m_name == y) {
            stack.append(convert("local" + QString::number(m_position), MetaType::VECTOR4, MetaType::FLOAT, 1));
        } else if(link.oport->m_name == z) {
            stack.append(convert("local" + QString::number(m_position), MetaType::VECTOR4, MetaType::FLOAT, 2));
        } else if(link.oport->m_name == w) {
            stack.append(convert("local" + QString::number(m_position), MetaType::VECTOR4, MetaType::FLOAT, 3));
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

class ProjectionMatrix : public ShaderNode {
    A_OBJECT(ProjectionMatrix, ShaderNode, Shader/Camera)

    A_PROPERTIES(
        A_PROPERTY(bool, inverted, ProjectionMatrix::inverted, ProjectionMatrix::setInverted)
    )

public:
    ProjectionMatrix() :
            m_inverted(false) {

        m_outputs.push_back(std::make_pair("Output", MetaType::MATRIX4));
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
    }

protected:
    bool m_inverted;

};

#endif // CAMERAFUNCTIONS_H
