#ifndef MATHOPERATOR
#define MATHOPERATOR

#include "function.h"

#define MINV    "Min"
#define MAXV    "Max"

class MathOperation : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    MathOperation() {
        m_ports.push_back(NodePort(this, false, QMetaType::QVector2D, 1, a, m_portColors[QMetaType::QVector2D]));
        m_ports.push_back(NodePort(this, false, QMetaType::QVector2D, 2, b, m_portColors[QMetaType::QVector2D]));
        m_ports.push_back(NodePort(this, true,  QMetaType::QVector2D, 0, "Output", m_portColors[QMetaType::QVector2D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t compile(const QString &operation, QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        if(m_position == -1) {
            QString args("(");

            for(NodePort &it : m_ports) {
                if(it.m_out) {
                    continue;
                }
                const AbstractNodeGraph::Link *l = m_graph->findLink(this, &it);
                if(l) {
                    ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);
                    if(node) {
                        int32_t l_type = 0;
                        int32_t index = node->build(code, stack, *l, depth, l_type);
                        if(index >= 0) {
                            if(portPosition(&it) == 1) { // take A type
                                if(type == 0) {
                                    type = l_type;
                                    m_type = type;
                                }
                            } else {
                                args.append(operation);
                            }

                            if(stack.isEmpty()) {
                                args.append(convert("local" + QString::number(index), type, type));
                            } else {
                                args.append(convert(stack.pop(), type, type));
                            }
                        } else {
                            return m_position;
                        }
                    }
                } else {
                    m_graph->reportMessage(this, QString("Missing argument ") + it.m_name.c_str());
                    return m_position;
                }
            }
            args.append(")");

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(args);
            } else {
                switch(type) {
                    case QMetaType::QVector2D: code.append("\tvec2"); break;
                    case QMetaType::QVector3D: code.append("\tvec3"); break;
                    case QMetaType::QVector4D: code.append("\tvec4"); break;
                    default: code.append("\tfloat"); break;
                }

                code.append(QString(" local%1 = %2;\n").arg(depth).arg(args));
            }
        } else {
            type = m_type;
        }

        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

class Subtraction : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Subtraction() {}

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" - ", code, stack, link, depth, type);
    }
};

class Add : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Add() {}

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" + ", code, stack, link, depth, type);
    }
};

class Divide : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Divide() {}

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" / ", code, stack, link, depth, type);
    }
};

class Multiply : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Multiply() {}

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" * ", code, stack, link, depth, type);
    }
};

class Step : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Step() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("step", code, stack, link, depth, type);
    }
};

class Smoothstep : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Smoothstep() {
        m_params << x << y << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("smoothstep", code, stack, link, depth, type, 0, 1);
    }
};

class Mix : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Mix() {
        m_params << x << y << a;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("mix", code, stack, link, depth, type, 0, 1);
    }
};

class Clamp : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Clamp() {
        m_params << a << MINV << MAXV;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("clamp", code, stack, link, depth, type);
    }
};

class Min : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Min() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("min", code, stack, link, depth, type);
    }
};

class Max : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Max() {
        m_params << x << y;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        return compile("max", code, stack, link, depth, type);
    }
};


class Power : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Power() {
        m_params << "Base" << "Exp";
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("pow", code, stack, link, depth, type);
    }
};

class SquareRoot : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE SquareRoot() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sqrt", code, stack, link, depth, type);
    }
};

class Logarithm : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Logarithm() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("log", code, stack, link, depth, type);
    }
};

class Logarithm2 : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Logarithm2() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("log2", code, stack, link, depth, type);
    }
};

class FWidth : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE FWidth() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("fwidth", code, stack, link, depth, type);
    }
};


class Abs : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Abs() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("abs", code, stack, link, depth, type);
    }
};

class Sign : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Sign() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("sign", code, stack, link, depth, type);
    }
};

class Floor : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Floor() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("floor", code, stack, link, depth, type);
    }
};

class Ceil : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Ceil() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("ceil", code, stack, link, depth, type);
    }
};

class Round : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Round() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("round", code, stack, link, depth, type);
    }
};

class Truncate : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Truncate() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("trunc", code, stack, link, depth, type);
    }
};

class Fract : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Fract() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("fract", code, stack, link, depth, type);
    }
};

class DDX : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE DDX() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("dFdx", code, stack, link, depth, type);
    }
};

class DDY : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE DDY() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("dFdy", code, stack, link, depth, type);
    }
};

class Exp : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Exp() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("exp", code, stack, link, depth, type);
    }
};

class Exp2 : public MathFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Exp2() {
        m_params << x;
        createParams();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile("exp2", code, stack, link, depth, type);
    }
};

#endif // MATHOPERATOR

