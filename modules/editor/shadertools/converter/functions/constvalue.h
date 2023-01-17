#ifndef CONSTVALUE_H
#define CONSTVALUE_H

#include "function.h"

#include <QColor>
#include <QVector2D>

class ConstPi : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

public:
    Q_INVOKABLE ConstPi() {
        m_ports.push_back(NodePort(this, true, QMetaType::Float, 0, "Value", m_portColors[QMetaType::Float]));

        setObjectName("PI");
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("3.141592653589793");
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }
};

class ConstGoldenRatio : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

public:
    Q_INVOKABLE ConstGoldenRatio() {
        m_ports.push_back(NodePort(this, true, QMetaType::Float, 0, "Value", m_portColors[QMetaType::Float]));

        setObjectName("GoldenRatio");
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("1.618033988749895");
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }
};

class ConstFloat : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(float Value READ value WRITE setValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstFloat() {
        m_value = 0.0;

        m_ports.push_back(NodePort(this, true, QMetaType::Float, 0, "Value", m_portColors[QMetaType::Float]));

        setObjectName("Value");
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    float value() const {
        return m_value;
    }

    void setValue(float value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            if(graph->isSingleConnection(link.oport)) {
                stack.push(QString::number(m_value));
            } else {
                code += QString("\tfloat local%1 = %2;\n").arg(depth).arg(m_value);
            }
        }
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }

protected:
    float m_value;

};

class ConstVector2 : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(float R READ valueR WRITE setValueR DESIGNABLE true USER true)
    Q_PROPERTY(float G READ valueG WRITE setValueG DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector2() {
        m_value = QVector2D();

        m_ports.push_back(NodePort(this, true, QMetaType::QVector2D, 0, "Value", m_portColors[QMetaType::QVector2D]));

        setObjectName("Vector2");
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    float valueR() const {
        return m_value.x();
    }

    void setValueR(const float value) {
        m_value.setX(value);
        emit updated();
    }

    float valueG() const {
        return m_value.y();
    }

    void setValueG(const float value) {
        m_value.setY(value);
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QString value = QString("vec2(%1, %2)").arg(m_value.x()).arg(m_value.y());
            if(graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code += QString("\tvec2 local%1 = %2;\n").arg(depth).arg(value);
            }
        }
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }

protected:
    QVector2D m_value;

};

class ConstVector3 : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(QColor Value READ value WRITE setValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector3() {
        m_value = QColor(0, 0, 0);

        m_ports.push_back(NodePort(this, true, QMetaType::QVector3D, 0, "Color", m_portColors[QMetaType::QVector3D]));

        setObjectName("RGB");
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    QColor value() const {
        return m_value;
    }

    void setValue(const QColor &value) {
        m_value = value.rgb();
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QString value = QString("vec3(%1, %2, %3)").arg(m_value.redF()).arg(m_value.greenF()).arg(m_value.blueF());
            if(graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code += QString("\tvec3 local%1 = %2;\n").arg(depth).arg(value);
            }
        }
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }

protected:
    QColor m_value;

};

class ConstVector4 : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(QColor Value READ value WRITE setValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector4() :
            m_value(QColor(0, 0, 0, 0)) {

        m_ports.push_back(NodePort(this, true, QMetaType::QVector4D, 0, "Color", m_portColors[QMetaType::QVector4D]));

        setObjectName("RGBA");
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    QColor value() const {
        return m_value;
    }

    void setValue(const QColor &value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QString value = QString("vec4(%1, %2, %3, %4)").arg(m_value.redF()).arg(m_value.greenF()).arg(m_value.blueF()).arg(m_value.alphaF());
            if(graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code += QString("\tvec4 local%1 = %2;\n").arg(depth).arg(value);
            }
        }
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }

protected:
    QColor m_value;

};

#endif // CONSTVALUE_H
