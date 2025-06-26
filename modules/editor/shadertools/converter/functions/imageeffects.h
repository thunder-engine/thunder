#ifndef IMAGEEFFECTS_H
#define IMAGEEFFECTS_H

#include "customfunction.h"

class Desaturate : public ShaderNode {
    A_OBJECT(Desaturate, ShaderNode, Shader/Image Effects)

    A_PROPERTIES(
        A_PROPERTY(Vector3, RGB, Desaturate::rgb, Desaturate::setRgb),
        A_PROPERTY(float, fraction, Desaturate::fraction, Desaturate::setFraction)
    )

public:
    Desaturate() :
            m_fraction(0.0f) {

        m_inputs.push_back(std::make_pair("RGB", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair("Fraction", MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList arguments = getArguments(code, stack, depth, type);

            QString expr = QString("mix(%1, vec3(dot(%1, vec3(0.299, 0.587, 0.114))), %2)").arg(arguments[0], arguments[1]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(MetaType::VECTOR3, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const std::string &key, uint32_t &) const override {
        if(key == "Fraction") {
            return QString::number(m_fraction);
        }

        return QString("vec3(%1, %2, %3)").arg(m_default.x).arg(m_default.y).arg(m_default.z);
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
