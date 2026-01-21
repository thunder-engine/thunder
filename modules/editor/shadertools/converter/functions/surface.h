#ifndef SURFACE_H
#define SURFACE_H

#include "function.h"

class Fresnel : public ShaderNode {
    A_OBJECT(Fresnel, ShaderNode, Shader/Surface)

    A_PROPERTIES(
        A_PROPERTY(float, Power, Fresnel::power, Fresnel::setPower)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Fresnel() :
            m_power(5.0f) {

        m_inputs.push_back(std::make_pair("Normal", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair("View Dir", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair("Power", MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("Output", MetaType::FLOAT));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            std::vector<TString> args = getArguments(code, stack, depth, type);

            // f0 = 0.04
            code.append(TString("float local%1 = 0.04 + (1.0 - 0.04) * pow(1.0 - dot(%2, -%3), %4);\n")
                        .arg(TString::number(depth), args[0], args[1], args[2]));
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    TString defaultValue(const TString &key, uint32_t &) const override {
        if(key == "Normal") {
            return "_n";
        } else if(key == "View Dir") {
            return "normalize(_vertex - cameraPosition())";
        }
        return TString::number(m_power);
    }

    float power() const {
        return m_power;
    }

    void setPower(float power) {
        m_power = power;

    }

private:
    float m_power;

};

class SurfaceDepth : public ShaderNode {
    A_OBJECT(SurfaceDepth, ShaderNode, Shader/Surface)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    SurfaceDepth() {
        m_inputs.push_back(std::make_pair("Vertex Position", MetaType::VECTOR3));

        m_outputs.push_back(std::make_pair("Depth", MetaType::FLOAT));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            std::vector<TString> args = getArguments(code, stack, depth, type);

            code.append(TString("float local%1 = (- modelViewMatrix() * vec4(%2, 1.0f)).z;\n")
                        .arg(TString::number(depth), args[0]));
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    TString defaultValue(const TString &key, uint32_t &) const override {
        return "_vertex.xyz";
    }
};

class WorldBitangent : public ShaderNode {
    A_OBJECT(WorldBitangent, ShaderNode, Shader/Surface)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    WorldBitangent() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_b");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class WorldNormal : public ShaderNode {
    A_OBJECT(WorldNormal, ShaderNode, Shader/Surface)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    WorldNormal() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_n");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class WorldPosition : public ShaderNode {
    A_OBJECT(WorldPosition, ShaderNode, Shader/Surface)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    WorldPosition() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_vertex.xyz");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class WorldTangent : public ShaderNode {
    A_OBJECT(WorldTangent, ShaderNode, Shader/Surface)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    WorldTangent() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            stack.push("_t");
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

#endif // SURFACE_H
