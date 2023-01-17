#ifndef MATHFUNCTIONS
#define MATHFUNCTIONS

#include "function.h"

class DotProduct : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE DotProduct() {
        m_params << a << b;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile("dot", code, stack, graph, link, depth, type);
    }
};

class CrossProduct : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE CrossProduct() {
        m_params << a << b;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::QVector3D;
        return compile("cross", code, stack, graph, link, depth, type, type);
    }
};

class Mod : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Mod() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("mod", code, stack, graph, link, depth, type);
    }
};

class Normalize : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Normalize() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("normalize", code, stack, graph, link, depth, type);
    }
};

#endif // MATHFUNCTIONS
