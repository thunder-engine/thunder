#ifndef MATERIALPARAM
#define MATERIALPARAM

#include "function.h"

class ParamFloat : public ShaderNode {
    A_OBJECT(ParamFloat, ShaderNode, Shader/Parameters)

    A_PROPERTIES(
        A_PROPERTY(string, Parameter_Name, ParamFloat::name, ParamFloat::setName),
        A_PROPERTY(float, Default_Value, ParamFloat::defaultValue, ParamFloat::setDefaultValue)
    )

public:
    ParamFloat() :
            m_defaultValue(0.0f) {

        m_outputs.push_back(std::make_pair("Output", MetaType::FLOAT));

        setName("ParamFloat");
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(type == 0) {
            type = link.oport->m_type;
        }
        static_cast<ShaderGraph *>(m_graph)->addUniform(name().c_str(), type, m_defaultValue);
        stack.push(QString("uni.%1").arg(name().c_str()));

        return ShaderNode::build(code, stack, link, depth, type);
    }

    float defaultValue() const {
        return m_defaultValue;
    }

    void setDefaultValue(float value) {
        if(m_defaultValue != value) {
            m_defaultValue = value;

        }
    }

private:
    float m_defaultValue;

};

class ParamVector : public ShaderNode {
    A_OBJECT(ParamVector, ShaderNode, Shader/Parameters)

    A_PROPERTIES(
        A_PROPERTY(string, Parameter_Name, ParamFloat::name, ParamFloat::setName),
        A_PROPERTY(Vector4, Default_Value, ParamFloat::defaultValue, ParamFloat::setDefaultValue)
    )

public:
    ParamVector() :
            m_defaultValue(Vector4(0, 0, 0, 0)) {

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR4));

        setName("ParamVector");
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(type == 0) {
            type = link.oport->m_type;
        }
        static_cast<ShaderGraph *>(m_graph)->addUniform(name().c_str(), type, Vector4(m_defaultValue.x,
                                                                              m_defaultValue.y,
                                                                              m_defaultValue.z,
                                                                              m_defaultValue.z));
        stack.push(QString("uni.%1").arg(name().c_str()));

        return ShaderNode::build(code, stack, link, depth, type);
    }

    Vector4 defaultValue() const {
        return m_defaultValue;
    }

    void setDefaultValue(const Vector4 &value) {
        if(m_defaultValue != value) {
            m_defaultValue = value;
        }
    }

private:
    Vector4 m_defaultValue;

};

#endif // MATERIALPARAM

