#ifndef MATHOPERATOR
#define MATHOPERATOR

#include "function.h"

#define MINV    "Min"
#define MAXV    "Max"

class MathOperation : public ShaderNode {
    A_OBJECT(MathOperation, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    MathOperation() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(b, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("Output", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(%1 %2 %3)").arg(args[0], m_expression, args[1]);
    }
};

class Subtraction : public MathOperation {
    A_OBJECT(Subtraction, MathOperation, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Subtraction() {
        m_expression = "-";
    }
};

class Add : public MathOperation {
    A_OBJECT(Add, MathOperation, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Add() {
        m_expression = "+";
    }
};

class Divide : public MathOperation {
    A_OBJECT(Divide, MathOperation, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Divide() {
        m_expression = "/";
    }
};

class Multiply : public MathOperation {
    A_OBJECT(Multiply, MathOperation, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Multiply() {
        m_expression = "*";
    }
};

class Step : public ShaderNode {
    A_OBJECT(Step, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Step() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(y, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "step";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Smoothstep : public ShaderNode {
    A_OBJECT(Smoothstep, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Smoothstep() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(y, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "smoothstep";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Mix : public ShaderNode {
    A_OBJECT(Mix, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Mix() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(y, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "mix";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Clamp : public ShaderNode {
    A_OBJECT(Clamp, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Clamp() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(MINV, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(MAXV, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "clamp";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Min : public ShaderNode {
    A_OBJECT(Min, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Min() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(y, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "min";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Max : public ShaderNode {
    A_OBJECT(Max, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Max() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(y, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "max";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Power : public ShaderNode {
    A_OBJECT(Power, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Power() {
        m_inputs.push_back(std::make_pair("Base", MetaType::INVALID));
        m_inputs.push_back(std::make_pair("Exp", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "pow";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class SquareRoot : public ShaderNode {
    A_OBJECT(SquareRoot, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    SquareRoot() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "sqrt";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Logarithm : public ShaderNode {
    A_OBJECT(Logarithm, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Logarithm() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "log";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Logarithm10 : public ShaderNode {
    A_OBJECT(Logarithm10, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Logarithm10() {
        m_inputs.push_back(std::make_pair(x, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("((1.0f / log(10.0f)) * log(%1))").arg(args[0]);
    }
};

class Logarithm2 : public ShaderNode {
    A_OBJECT(Logarithm2, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Logarithm2() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "log2";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class FWidth : public ShaderNode {
    A_OBJECT(FWidth, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    FWidth() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "fwidth";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};


class Abs : public ShaderNode {
    A_OBJECT(Abs, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Abs() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "abs";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Sign : public ShaderNode {
    A_OBJECT(Sign, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Sign() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "sign";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Floor : public ShaderNode {
    A_OBJECT(Floor, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Floor() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "floor";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Ceil : public ShaderNode {
    A_OBJECT(Ceil, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Ceil() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "ceil";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Round : public ShaderNode {
    A_OBJECT(Round, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Round() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "round";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Truncate : public ShaderNode {
    A_OBJECT(Truncate, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Truncate() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "trunc";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Fract : public ShaderNode {
    A_OBJECT(Fract, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Fract() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "fract";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class DDX : public ShaderNode {
    A_OBJECT(DDX, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    DDX() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "dFdx";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class DDY : public ShaderNode {
    A_OBJECT(DDY, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    DDY() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "dFdy";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Exp : public ShaderNode {
    A_OBJECT(Exp, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Exp() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "exp";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Exp2 : public ShaderNode {
    A_OBJECT(Exp2, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Exp2() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "exp2";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Remainder : public ShaderNode {
    A_OBJECT(Remainder, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Remainder() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "mod";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class RSqrt : public ShaderNode {
    A_OBJECT(RSqrt, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RSqrt() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

        m_expression = "inversesqrt";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }
};

class Fmod : public ShaderNode {
    A_OBJECT(Fmod, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Fmod() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(y, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(%1 - %2 * trunc(%1 / %2)) ").arg(args[0], args[1]);
    }
};

class InverseLerp : public ShaderNode {
    A_OBJECT(InverseLerp, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    InverseLerp() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(b, MetaType::INVALID));
        m_inputs.push_back(std::make_pair("Value", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("((%3 - %1) / (%2 - %1))").arg(args[0], args[1], args[2]);
    }
};

class Negate : public ShaderNode {
    A_OBJECT(Negate, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Negate() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(-%1)").arg(args[0]);
    }
};

class Saturate : public ShaderNode {
    A_OBJECT(Saturate, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Saturate() {
        m_inputs.push_back(std::make_pair("in", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("clamp(%1, 0.0f, 1.0f)").arg(args[0]);
    }
};

class Scale : public ShaderNode {
    A_OBJECT(Scale, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Scale() {
        m_inputs.push_back(std::make_pair("In", MetaType::INVALID));
        m_inputs.push_back(std::make_pair("Scale", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(%1 * %2)").arg(args[0], args[1]);
    }
};

class ScaleAndOffset : public ShaderNode {
    A_OBJECT(ScaleAndOffset, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ScaleAndOffset() {
        m_inputs.push_back(std::make_pair("In", MetaType::INVALID));
        m_inputs.push_back(std::make_pair("Scale", MetaType::INVALID));
        m_inputs.push_back(std::make_pair("Offset", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(%1 * %2 + %3)").arg(args[0], args[1], args[2]);
    }
};

class OneMinus : public ShaderNode {
    A_OBJECT(OneMinus, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    OneMinus() {
        m_inputs.push_back(std::make_pair("in", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(%1 - %2)").arg(convert("1.0f", MetaType::FLOAT, m_type), args[0]);
    }
};

class TriangleWave : public ShaderNode {
    A_OBJECT(TriangleWave, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TriangleWave() {
        m_inputs.push_back(std::make_pair("in", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(2.0f * abs(2.0f * (%1 - floor(0.5f + %1)) ) - 1.0f)").arg(args[0]);
    }
};

class SquareWave : public ShaderNode {
    A_OBJECT(SquareWave, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    SquareWave() {
        m_inputs.push_back(std::make_pair("in", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(1.0f - 2.0f * round(fract(%1))").arg(args[0]);
    }
};

class SawtoothWave : public ShaderNode {
    A_OBJECT(SawtoothWave, ShaderNode, Shader/Math Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    SawtoothWave() {
        m_inputs.push_back(std::make_pair("in", MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        return compile(code, stack, link, depth, type);
    }

    TString makeExpression(const std::vector<TString> &args) const override {
        return TString("(2.0f * (%1 - floor(0.5f + %1)))").arg(args[0]);
    }
};

#endif // MATHOPERATOR

