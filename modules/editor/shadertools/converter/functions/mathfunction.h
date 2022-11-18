#ifndef MATHFUNCTIONS
#define MATHFUNCTIONS

#include "function.h"

#define MINV    "Min"
#define MAXV    "Max"

class MathFunction : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    bool isPreview() const override {
        return true;
    }

    int32_t compile(const QString &func, QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type, int32_t expect = 0, int32_t last = 0) {
        if(m_position == -1) {
            QString args;

            int i = 0;
            for(NodePort &it : m_ports) {
                if(it.m_out == true) {
                    continue;
                }
                const AbstractNodeGraph::Link *l = graph->findLink(this, &it);
                if(l) {
                    ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);
                    if(node) {
                        int32_t type = 0;
                        int32_t index = node->build(code, stack, graph, *l, depth, type);
                        if(index >= 0) {
                            if(i == 0 && !expect) {
                                expect = type;
                            }

                            uint8_t final = expect;
                            if(i == (m_params.size() - 1) && last) {
                                final = last;
                            }

                            if(stack.isEmpty()) {
                                args.append(convert(QString("local%1").arg(index), type, final));
                            } else {
                                args.append(convert(stack.pop(), type, final));
                            }

                            args.append(((i == m_params.size() - 1) ? "" : ", "));
                        } else {
                            return index;
                        }
                    }
                } else {
                    graph->reportMessage(this, QString("Missing argument ") + it.m_name.c_str());
                    return m_position;
                }
                i++;
            }

            if(!type) {
                type = expect;
                m_type = type;
            }

            switch(type) {
                case QMetaType::QVector2D: code.append("\tvec2");  break;
                case QMetaType::QVector3D: code.append("\tvec3");  break;
                case QMetaType::QVector4D: code.append("\tvec4");  break;
                default: code.append("\tfloat"); break;
            }
            code.append(QString(" local%1 = %2(%3);\n").arg(QString::number(depth), func, args));
        } else {
            type = m_type;
        }
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }

    void createParams() {
        int i = 0;
        for(QString &it : m_params) {
            m_ports.push_back(NodePort(this, false, QMetaType::Void, i + 1, qPrintable(it), m_portColors[QMetaType::Void]));
            i++;
        }

        m_ports.push_back(NodePort(this, true, QMetaType::Void, 0, "Output", m_portColors[QMetaType::Void]));
    }

protected:
    QStringList m_params;

};

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
class Smoothstep : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Smoothstep() {
        m_params << x << y << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("smoothstep", code, stack, graph, link, depth, type, 0, 1);
    }
};

class Mix : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Mix() {
        m_params << x << y << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("mix", code, stack, graph, link, depth, type, 0, 1);
    }
};

class Clamp : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Clamp() {
        m_params << a << MINV << MAXV;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("clamp", code, stack, graph, link, depth, type);
    }
};

class Min : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Min() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("min", code, stack, graph, link, depth, type);
    }
};

class Max : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Max() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        return compile("max", code, stack, graph, link, depth, type);
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

class Power : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Power() {
        m_params << "Base" << "Exp";
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("pow", code, stack, graph, link, depth, type);
    }
};

class SquareRoot : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE SquareRoot() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sqrt", code, stack, graph, link, depth, type);
    }
};

class Logarithm : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Logarithm() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("log", code, stack, graph, link, depth, type);
    }
};

class Logarithm2 : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Logarithm2() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("log2", code, stack, graph, link, depth, type);
    }
};

class FWidth : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE FWidth() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("fwidth", code, stack, graph, link, depth, type);
    }
};


class Abs : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Abs() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("abs", code, stack, graph, link, depth, type);
    }
};

class Sign : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Sign() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sign", code, stack, graph, link, depth, type);
    }
};

class Floor : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Floor() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("floor", code, stack, graph, link, depth, type);
    }
};

class Ceil : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Ceil() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("ceil", code, stack, graph, link, depth, type);
    }
};

class Round : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Round() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("round", code, stack, graph, link, depth, type);
    }
};

class Truncate : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Truncate() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("trunc", code, stack, graph, link, depth, type);
    }
};

class Fract : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Fract() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("fract", code, stack, graph, link, depth, type);
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

class Sine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Sine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sin", code, stack, graph, link, depth, type);
    }
};

class Cosine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Cosine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("cos", code, stack, graph, link, depth, type);
    }
};

class Tangent : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Tangent() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("tan", code, stack, graph, link, depth, type);
    }
};

class ArcSine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcSine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("asin", code, stack, graph, link, depth, type);
    }
};

class ArcCosine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcCosine() {
        m_params << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("acos", code, stack, graph, link, depth, type);
    }
};

class ArcTangent : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcTangent() {
        m_params << a << b;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("atan", code, stack, graph, link, depth, type);
    }
};

#endif // MATHFUNCTIONS
