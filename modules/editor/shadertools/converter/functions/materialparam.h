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

        ports.push_back(new NodePort(true, QMetaType::Double, 0, ""));
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        if(m_position == -1) {
            graph->addUniform(objectName(), size, m_defaultValue);
            stack.push(QString("uni.%1").arg(objectName()));
        }
        return ShaderFunction::build(code, stack, graph, link, depth, size);
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

        ports.push_back(new NodePort(true, QMetaType::QVector4D, 0, ""));
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        if(m_position == -1) {
            graph->addUniform(objectName(), size, m_defaultValue);
            stack.push(QString("uni.%1").arg(objectName()));
        }
        return ShaderFunction::build(code, stack, graph, link, depth, size);
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

