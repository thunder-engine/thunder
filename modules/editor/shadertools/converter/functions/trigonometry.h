#ifndef TRIGONOMETRY_H
#define TRIGONOMETRY_H

#include "function.h"

class ArcCosine : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcCosine() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "acos";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class ArcSine : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcSine() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "asin";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class ArcTangent : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcTangent() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "atan";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class ArcTangent2 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcTangent2() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));
        m_inputs.push_back(std::make_pair(b, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "atan";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Cosine : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Cosine() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "cos";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class CosineHyperbolic : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE CosineHyperbolic() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "cosh";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Sine : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Sine() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "sin";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class SineHyperbolic : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE SineHyperbolic() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "sinh";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Tangent : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Tangent() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "tan";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class TangentHyperbolic : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE TangentHyperbolic() {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "tanh";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Degrees : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Degrees() {
        m_inputs.push_back(std::make_pair(r, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "degrees";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Radians : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Radians() {
        m_inputs.push_back(std::make_pair(r, QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Float));

        m_expression = "radians";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

#endif // TRIGONOMETRY_H
