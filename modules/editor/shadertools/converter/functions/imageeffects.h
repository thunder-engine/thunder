#ifndef IMAGEEFFECTS_H
#define IMAGEEFFECTS_H

#include "customfunction.h"

class Desaturate : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Image Effects")

    Q_PROPERTY(Vector3 RGB READ rgb WRITE setRgb NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float fraction READ fraction WRITE setFraction NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Desaturate() :
            m_fraction(0.0f) {

        m_inputs.push_back(std::make_pair("RGB", QMetaType::QVector3D));
        m_inputs.push_back(std::make_pair("Fraction", QMetaType::Float));

        m_outputs.push_back(std::make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList arguments = getArguments(code, stack, depth, type);

            QString expr = QString("mix(%1, vec3(dot(%1, vec3(0.299, 0.587, 0.114))), %2)").arg(arguments[0], arguments[1]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(QMetaType::QVector3D, depth, expr));
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

    Vector3 rgb() {
        return m_default;
    }

    void setRgb(const Vector3 &value) {
        m_default = value;
        emit updated();
    }

    float fraction() {
        return m_fraction;
    }

    void setFraction(const float value) {
        m_fraction = value;
        emit updated();
    }

protected:
    Vector3 m_default;

    float m_fraction;

};

#endif // IMAGEEFFECTS_H
