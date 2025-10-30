#ifndef TRIGONOMETRY_H
#define TRIGONOMETRY_H

#include "function.h"

class ArcCosine : public ShaderNode {
    A_OBJECT(ArcCosine, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ArcCosine() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "acos";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class ArcSine : public ShaderNode {
    A_OBJECT(ArcSine, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ArcSine() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "asin";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class ArcTangent : public ShaderNode {
    A_OBJECT(ArcTangent, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ArcTangent() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "atan";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class ArcTangent2 : public ShaderNode {
    A_OBJECT(ArcTangent2, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ArcTangent2() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(b, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "atan";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Cosine : public ShaderNode {
    A_OBJECT(Cosine, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Cosine() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "cos";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class CosineHyperbolic : public ShaderNode {
    A_OBJECT(CosineHyperbolic, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    CosineHyperbolic() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "cosh";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Sine : public ShaderNode {
    A_OBJECT(Sine, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Sine() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "sin";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class SineHyperbolic : public ShaderNode {
    A_OBJECT(SineHyperbolic, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    SineHyperbolic() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "sinh";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Tangent : public ShaderNode {
    A_OBJECT(Tangent, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Tangent() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "tan";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class TangentHyperbolic : public ShaderNode {
    A_OBJECT(TangentHyperbolic, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TangentHyperbolic() {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "tanh";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Degrees : public ShaderNode {
    A_OBJECT(Degrees, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Degrees() {
        m_inputs.push_back(std::make_pair(r, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "degrees";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Radians : public ShaderNode {
    A_OBJECT(Radians, ShaderNode, Shader/Trigonometry Operators)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Radians() {
        m_inputs.push_back(std::make_pair(r, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "radians";
    }

    int32_t build(TString &code, std::stack<TString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

#endif // TRIGONOMETRY_H
