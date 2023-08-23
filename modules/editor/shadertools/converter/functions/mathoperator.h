#ifndef MATHOPERATOR
#define MATHOPERATOR

#include "function.h"

#define MINV    "Min"
#define MAXV    "Max"

class MathOperation : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    MathOperation() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));
        m_inputs.push_back(make_pair(b, QMetaType::Void));

        m_outputs.push_back(make_pair("Output", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(%1 %2 %3)").arg(args[0], m_expression, args[1]);
    }
};

class Subtraction : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Subtraction() {
        m_expression = "-";
    }
};

class Add : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Add() {
        m_expression = "+";
    }
};

class Divide : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Divide() {
        m_expression = "/";
    }
};

class Multiply : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Multiply() {
        m_expression = "*";
    }
};

class Step : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Step() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));
        m_inputs.push_back(make_pair(y, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "step";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Smoothstep : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Smoothstep() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));
        m_inputs.push_back(make_pair(y, QMetaType::Void));
        m_inputs.push_back(make_pair(a, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "smoothstep";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Mix : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Mix() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));
        m_inputs.push_back(make_pair(y, QMetaType::Void));
        m_inputs.push_back(make_pair(a, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "mix";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Clamp : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Clamp() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));
        m_inputs.push_back(make_pair(MINV, QMetaType::Void));
        m_inputs.push_back(make_pair(MAXV, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "clamp";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Min : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Min() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));
        m_inputs.push_back(make_pair(y, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "min";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Max : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Max() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));
        m_inputs.push_back(make_pair(y, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "max";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        return compile(code, stack, link, depth, type);
    }
};

class Power : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Power() {
        m_inputs.push_back(make_pair("Base", QMetaType::Void));
        m_inputs.push_back(make_pair("Exp", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "pow";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class SquareRoot : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE SquareRoot() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "sqrt";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Logarithm : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Logarithm() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "log";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Logarithm10 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Logarithm10() {
        m_inputs.push_back(make_pair(x, QMetaType::Float));

        m_outputs.push_back(make_pair("", QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("((1.0f / log(10.0f)) * log(%1))").arg(args[0]);
    }
};

class Logarithm2 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Logarithm2() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "log2";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class FWidth : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE FWidth() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "fwidth";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};


class Abs : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Abs() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "abs";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Sign : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Sign() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "sign";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Floor : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Floor() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "floor";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Ceil : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Ceil() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "ceil";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Round : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Round() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "round";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Truncate : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Truncate() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "trunc";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Fract : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Fract() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "fract";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class DDX : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE DDX() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "dFdx";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class DDY : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE DDY() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "dFdy";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Exp : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Exp() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "exp";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Exp2 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Exp2() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "exp2";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Remainder : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Remainder() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "mod";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class RSqrt : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE RSqrt() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));

        m_expression = "inversesqrt";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Fmod : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Fmod() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));
        m_inputs.push_back(make_pair(y, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(%1 - %2 * trunc(%1 / %2)) ").arg(args[0], args[1]);
    }
};

class InverseLerp : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE InverseLerp() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));
        m_inputs.push_back(make_pair(b, QMetaType::Void));
        m_inputs.push_back(make_pair("Value", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("((%3 - %1) / (%2 - %1))").arg(args[0], args[1], args[2]);
    }
};

class Negate : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Negate() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(-%1)").arg(args[0]);
    }
};

class Saturate : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Saturate() {
        m_inputs.push_back(make_pair("in", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("clamp(%1, 0.0f, 1.0f)").arg(args[0]);
    }
};

class Scale : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE Scale() {
        m_inputs.push_back(make_pair("In", QMetaType::Void));
        m_inputs.push_back(make_pair("Scale", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(%1 * %2)").arg(args[0], args[1]);
    }
};

class ScaleAndOffset : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE ScaleAndOffset() {
        m_inputs.push_back(make_pair("In", QMetaType::Void));
        m_inputs.push_back(make_pair("Scale", QMetaType::Void));
        m_inputs.push_back(make_pair("Offset", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(%1 * %2 + %3)").arg(args[0], args[1], args[2]);
    }
};

class OneMinus : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE OneMinus() {
        m_inputs.push_back(make_pair("in", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(%1 - %2)").arg(convert("1.0f", QMetaType::Float, m_type), args[0]);
    }
};

class TriangleWave : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE TriangleWave() {
        m_inputs.push_back(make_pair("in", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(2.0f * abs(2.0f * (%1 - floor(0.5f + %1)) ) - 1.0f)").arg(args[0]);
    }
};

class SquareWave : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE SquareWave() {
        m_inputs.push_back(make_pair("in", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(1.0f - 2.0f * round(fract(%1))").arg(args[0]);
    }
};

class SawtoothWave : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math Operations")

public:
    Q_INVOKABLE SawtoothWave() {
        m_inputs.push_back(make_pair("in", QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("(2.0f * (%1 - floor(0.5f + %1)))").arg(args[0]);
    }
};

#endif // MATHOPERATOR

