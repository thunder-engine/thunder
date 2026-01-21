#ifndef IMAGEEFFECTS_H
#define IMAGEEFFECTS_H

#include "function.h"

class Desaturate : public ShaderNode {
    A_OBJECT(Desaturate, ShaderNode, Shader/Image Effects)

    A_PROPERTIES(
        A_PROPERTY(Vector3, RGB, Desaturate::rgb, Desaturate::setRgb),
        A_PROPERTY(float, fraction, Desaturate::fraction, Desaturate::setFraction)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Desaturate() :
            m_fraction(0.0f) {

        m_inputs.push_back(std::make_pair("RGB", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair("Fraction", MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            std::vector<TString> args = getArguments(code, stack, depth, type);

            TString expr = TString("mix(%1, vec3(dot(%1, vec3(0.299, 0.587, 0.114))), %2)").arg(args[0], args[1]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(MetaType::VECTOR3, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    TString defaultValue(const TString &key, uint32_t &) const override {
        if(key == "Fraction") {
            return TString::number(m_fraction);
        }

        return TString("vec3(%1, %2, %3)").arg(TString::number(m_default.x), TString::number(m_default.y), TString::number(m_default.z));
    }

    Vector3 rgb() const {
        return m_default;
    }

    void setRgb(const Vector3 &value) {
        m_default = value;
    }

    float fraction() const {
        return m_fraction;
    }

    void setFraction(const float value) {
        m_fraction = value;
    }

protected:
    Vector3 m_default;

    float m_fraction;

};

#endif // IMAGEEFFECTS_H
