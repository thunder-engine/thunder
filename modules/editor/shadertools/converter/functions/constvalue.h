#ifndef CONSTVALUE_H
#define CONSTVALUE_H

#include "function.h"

#include <QColor>

class ConstPi : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

public:
    Q_INVOKABLE ConstPi() {
        m_outputs.push_back(std::make_pair("Value", QMetaType::Float));

        setObjectName("PI");
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("3.141592653589793");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ConstEuler : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

public:
    Q_INVOKABLE ConstEuler() {
        m_outputs.push_back(std::make_pair("Value", QMetaType::Float));

        setObjectName("Euler");
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("2.718281828459045");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ConstGoldenRatio : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

public:
    Q_INVOKABLE ConstGoldenRatio() {
        m_outputs.push_back(std::make_pair("Value", QMetaType::Float));

        setObjectName("GoldenRatio");
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push("1.618033988749895");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ConstFloat : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(float Value READ value WRITE setValue NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstFloat() :
            m_value(0.0f) {

        m_outputs.push_back(std::make_pair("Value", QMetaType::Float));

        setObjectName("Float");
    }

    float value() const {
        return m_value;
    }

    void setValue(float value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString::number(m_value));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    float m_value;

};

class ConstInt : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(int Value READ value WRITE setValue NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstInt() :
            m_value(0) {

        m_outputs.push_back(std::make_pair("Value", QMetaType::Int));

        setObjectName("Integer");
    }

    int value() const {
        return m_value;
    }

    void setValue(int value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString::number(m_value));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    int m_value;

};

class ConstVector2 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(Vector2 Value READ value WRITE setValue NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector2() {
        m_outputs.push_back(std::make_pair("Value", QMetaType::QVector2D));

        setObjectName("Vector2");
    }

    Vector2 value() const {
        return m_value;
    }

    void setValue(Vector2 value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("vec2(%1, %2)").arg(m_value.x).arg(m_value.y));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector2 m_value;

};

class ConstVector3 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(Vector3 Value READ value WRITE setValue NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector3() {
        m_outputs.push_back(std::make_pair("Value", QMetaType::QVector3D));

        setObjectName("Vector3");
    }

    Vector3 value() const {
        return m_value;
    }

    void setValue(const Vector3 &value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("vec3(%1, %2, %3)").arg(m_value.x).arg(m_value.y).arg(m_value.z));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector3 m_value;

};

class ConstVector4 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(Vector4 Value READ value WRITE setValue NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector4() :
            m_value(Vector4(0, 0, 0, 0)) {

        m_outputs.push_back(std::make_pair("Value", QMetaType::QVector4D));

        setObjectName("Vector4");
    }

    Vector4 value() const {
        return m_value;
    }

    void setValue(const Vector4 &value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("vec4(%1, %2, %3, %4)").arg(m_value.x).arg(m_value.y).arg(m_value.z).arg(m_value.w));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector4 m_value;

};

class ConstColor : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(QColor Value READ value WRITE setValue NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstColor() :
            m_value(QColor(0, 0, 0, 0)) {

        m_outputs.push_back(std::make_pair("Color", QMetaType::QVector4D));

        setObjectName("Color");
    }

    QColor value() const {
        return m_value;
    }

    void setValue(const QColor &value) {
        m_value = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("vec4(%1, %2, %3, %4)").arg(m_value.redF()).arg(m_value.greenF()).arg(m_value.blueF()).arg(m_value.alphaF()));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    QColor m_value;

};

class ConstMatrix3 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(Vector3 Value0 READ value0 WRITE setValue0 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Value1 READ value1 WRITE setValue1 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Value2 READ value2 WRITE setValue2 NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstMatrix3() {
        m_value0.x = 1.0f;
        m_value1.y = 1.0f;
        m_value2.z = 1.0f;

        m_outputs.push_back(std::make_pair("Value", QMetaType::QTransform));

        setObjectName("Matrix3");
    }

    Vector3 value0() const {
        return m_value0;
    }

    void setValue0(const Vector3 &value) {
        m_value0 = value;
        emit updated();
    }

    Vector3 value1() const {
        return m_value1;
    }

    void setValue1(const Vector3 &value) {
        m_value1 = value;
        emit updated();
    }

    Vector3 value2() const {
        return m_value2;
    }

    void setValue2(const Vector3 &value) {
        m_value2 = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("mat3(%1, %2, %3, %4, %5, %6, %7, %8, %9)")
                   .arg(m_value0.x).arg(m_value0.y).arg(m_value0.z)
                   .arg(m_value1.x).arg(m_value1.y).arg(m_value1.z)
                   .arg(m_value2.x).arg(m_value2.y).arg(m_value2.z));

        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector3 m_value0;
    Vector3 m_value1;
    Vector3 m_value2;

};

class ConstMatrix4 : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(Vector4 Value0 READ value0 WRITE setValue0 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Value1 READ value1 WRITE setValue1 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Value2 READ value2 WRITE setValue2 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Value3 READ value3 WRITE setValue3 NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstMatrix4() {
        m_value0.x = 1.0f;
        m_value1.y = 1.0f;
        m_value2.z = 1.0f;
        m_value3.w = 1.0f;

        m_outputs.push_back(std::make_pair("Value", QMetaType::QMatrix4x4));

        setObjectName("Matrix4");
    }

    Vector4 value0() const {
        return m_value0;
    }

    void setValue0(const Vector4 &value) {
        m_value0 = value;
        emit updated();
    }

    Vector4 value1() const {
        return m_value1;
    }

    void setValue1(const Vector4 &value) {
        m_value1 = value;
        emit updated();
    }

    Vector4 value2() const {
        return m_value2;
    }

    void setValue2(const Vector4 &value) {
        m_value2 = value;
        emit updated();
    }

    Vector4 value3() const {
        return m_value3;
    }

    void setValue3(const Vector4 &value) {
        m_value3 = value;
        emit updated();
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("mat4(%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16)")
                   .arg(m_value0.x).arg(m_value0.y).arg(m_value0.z).arg(m_value0.w)
                   .arg(m_value1.x).arg(m_value1.y).arg(m_value1.z).arg(m_value1.w)
                   .arg(m_value2.x).arg(m_value2.y).arg(m_value2.z).arg(m_value2.w)
                   .arg(m_value3.x).arg(m_value3.y).arg(m_value3.z).arg(m_value3.w));

        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

};

#endif // CONSTVALUE_H
