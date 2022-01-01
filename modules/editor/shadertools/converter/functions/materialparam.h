#ifndef MATERIALPARAM
#define MATERIALPARAM

#include "function.h"

class ParamFloat : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Parameters")

    Q_PROPERTY(QString Parameter_Name READ objectName WRITE setObjectName DESIGNABLE true USER true)
    Q_PROPERTY(float Default_Value READ defaultValue WRITE setDefaultValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ParamFloat() :
        m_defaultValue(0.0f) {
        connect(this, SIGNAL(objectNameChanged(QString)), this, SIGNAL(updated()));
    }

    virtual AbstractSchemeModel::Node *createNode(ShaderSchemeModel *model, const QString &path) override {
        AbstractSchemeModel::Node *result = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Port *out = new AbstractSchemeModel::Port;
        out->name = "";
        out->out  = true;
        out->pos  = 0;
        out->type = QMetaType::Double;
        result->list.push_back(out);

        return result;
    }

    int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        if(m_Position == -1) {
            size    = QMetaType::Double;
            m_pModel->addUniform(objectName(), size, m_defaultValue);
            value  += QString("\tfloat local%1 = uni.%2;\n").arg(depth).arg(objectName());
        }
        return ShaderFunction::build(value, link, depth, size);
    }

    float defaultValue() const {
        return m_defaultValue;
    }

    void setDefaultValue(float value) {
        if(m_defaultValue != value) {
            m_defaultValue = value;
            emit updated();
        }
    }

private:
    float m_defaultValue;
};

class ParamVector : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Parameters")

    Q_PROPERTY(QString Parameter_Name READ objectName WRITE setObjectName DESIGNABLE true USER true)
    Q_PROPERTY(QColor Default_Value READ defaultValue WRITE setDefaultValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ParamVector() :
        m_defaultValue(QColor(0, 0, 0, 0)) {
        connect(this, SIGNAL(objectNameChanged(QString)), this, SIGNAL(updated()));
    }

    virtual AbstractSchemeModel::Node *createNode(ShaderSchemeModel *model, const QString &path) override {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Port *out      = new AbstractSchemeModel::Port;
        out->name = "";
        out->out  = true;
        out->pos  = 0;
        out->type = QMetaType::QVector4D;
        result->list.push_back(out);

        return result;
    }

    int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        if(m_Position == -1) {
            size    = QMetaType::QVector4D;
            m_pModel->addUniform(objectName(), size, m_defaultValue);
            value  += QString("\tvec4 local%1 = uni.%2;\n").arg(depth).arg(objectName());
        }
        return ShaderFunction::build(value, link, depth, size);
    }

    QColor defaultValue() const {
        return m_defaultValue;
    }

    void setDefaultValue(const QColor &value) {
        if(m_defaultValue != value) {
            m_defaultValue = value;
            emit updated();
        }
    }

private:
    QColor m_defaultValue;

};

#endif // MATERIALPARAM

