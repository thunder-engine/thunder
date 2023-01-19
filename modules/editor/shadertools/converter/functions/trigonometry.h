#ifndef TRIGONOMETRY_H
#define TRIGONOMETRY_H

#include "function.h"

class ArcCosine : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcCosine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("acos", code, stack, link, depth, type);
    }
};

class ArcSine : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcSine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("asin", code, stack, link, depth, type);
    }
};

class ArcTangent : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE ArcTangent() {
        m_params << a << b;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("atan", code, stack, link, depth, type);
    }
};

class Cosine : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Cosine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("cos", code, stack, link, depth, type);
    }
};

class CosineHyperbolic : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE CosineHyperbolic() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("cosh", code, stack, link, depth, type);
    }
};

class Sine : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Sine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sin", code, stack, link, depth, type);
    }
};

class SineHyperbolic : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE SineHyperbolic() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sinh", code, stack, link, depth, type);
    }
};

class Tangent : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE Tangent() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("tan", code, stack, link, depth, type);
    }
};

class TangentHyperbolic : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Trigonometry Operators")

public:
    Q_INVOKABLE TangentHyperbolic() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("tanh", code, stack, link, depth, type);
    }
};

#endif // TRIGONOMETRY_H
