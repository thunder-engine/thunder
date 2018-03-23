#ifndef MATERIALPARAM
#define MATERIALPARAM

#include "../shaderbuilder.h"

class ParamFloat : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Parameters")

public:
    Q_INVOKABLE ParamFloat() { }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        size    = MetaType::FLOAT;
        m_pModel->addUniform(objectName(), size); // \todo review object name
        value  += QString("\tfloat local%1 = %2;\n").arg(depth).arg(objectName()); // \todo review object name

        return true;
    }
};

class ParamVector : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Parameters")

public:
    Q_INVOKABLE ParamVector() { }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        size    = MetaType::VECTOR4;
        m_pModel->addUniform(objectName(), size); // \todo review object name
        value  += QString("\tvec4 local%1 = ;\n").arg(depth).arg(objectName()); // \todo review object name
        return true;
    }
};

#endif // MATERIALPARAM

