#ifndef CONSTVALUE_H
#define CONSTVALUE_H

#include "function.h"

class ConstPi : public ShaderNode {
    A_OBJECT(ConstPi, ShaderNode, Shader/Constant)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstPi() {
        m_outputs.push_back(std::make_pair("Value", MetaType::FLOAT));

        setName("PI");
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push("3.141592653589793");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ConstEuler : public ShaderNode {
    A_OBJECT(ConstEuler, ShaderNode, Shader/Constant)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstEuler() {
        m_outputs.push_back(std::make_pair("Value", MetaType::FLOAT));

        setName("Euler");
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push("2.718281828459045");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ConstGoldenRatio : public ShaderNode {
    A_OBJECT(ConstGoldenRatio, ShaderNode, Shader/Constant)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstGoldenRatio() {
        m_outputs.push_back(std::make_pair("Value", MetaType::FLOAT));

        setName("GoldenRatio");
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push("1.618033988749895");
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class ConstFloat : public ShaderNode {
    A_OBJECT(ConstFloat, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(float, Value, ConstFloat::value, ConstFloat::setValue)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstFloat() :
            m_value(0.0f) {

        m_outputs.push_back(std::make_pair("Value", MetaType::FLOAT));

        setName("Float");
    }

    float value() const {
        return m_value;
    }

    void setValue(float value) {
        m_value = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(TString::number(m_value));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    float m_value;

};

class ConstInt : public ShaderNode {
    A_OBJECT(ConstInt, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(int, Value, ConstFloat::value, ConstFloat::setValue)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstInt() :
            m_value(0) {

        m_outputs.push_back(std::make_pair("Value", MetaType::INTEGER));

        setName("Integer");
    }

    int value() const {
        return m_value;
    }

    void setValue(int value) {
        m_value = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(TString::number(m_value));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    int m_value;

};

class ConstVector2 : public ShaderNode {
    A_OBJECT(ConstVector2, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(Vector2, Value, ConstVector2::value, ConstVector2::setValue)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstVector2() {
        m_outputs.push_back(std::make_pair("Value", MetaType::VECTOR2));

        setName("Vector2");
    }

    Vector2 value() const {
        return m_value;
    }

    void setValue(Vector2 value) {
        m_value = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(TString("vec2(%1, %2)").arg(TString::number(m_value.x), TString::number(m_value.y)));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector2 m_value;

};

class ConstVector3 : public ShaderNode {
    A_OBJECT(ConstVector3, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(Vector3, Value, ConstVector3::value, ConstVector3::setValue)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstVector3() {
        m_outputs.push_back(std::make_pair("Value", MetaType::VECTOR3));

        setName("Vector3");
    }

    Vector3 value() const {
        return m_value;
    }

    void setValue(const Vector3 &value) {
        m_value = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(TString("vec3(%1, %2, %3)").arg(TString::number(m_value.x), TString::number(m_value.y), TString::number(m_value.z)));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector3 m_value;

};

class ConstVector4 : public ShaderNode {
    A_OBJECT(ConstVector4, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(Vector4, Value, ConstVector4::value, ConstVector4::setValue)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstVector4() :
            m_value(Vector4(0.0f)) {

        m_outputs.push_back(std::make_pair("Value", MetaType::VECTOR4));

        setName("Vector4");
    }

    Vector4 value() const {
        return m_value;
    }

    void setValue(const Vector4 &value) {
        m_value = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(TString("vec4(%1, %2, %3, %4)").arg(TString::number(m_value.x), TString::number(m_value.y),
                                                       TString::number(m_value.z), TString::number(m_value.w)));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector4 m_value;

};

class ConstColor : public ShaderNode {
    A_OBJECT(ConstColor, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTYEX(Vector4, Value, ConstColor::value, ConstColor::setValue, "editor=Color")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstColor() :
            m_value(Vector4(0.0f)) {

        m_outputs.push_back(std::make_pair("Color", MetaType::VECTOR4));

        setName("Color");
    }

    Vector4 value() const {
        return m_value;
    }

    void setValue(const Vector4 &value) {
        m_value = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(TString("vec4(%1, %2, %3, %4)").arg(TString::number(m_value.x), TString::number(m_value.y),
                                                       TString::number(m_value.z), TString::number(m_value.w)));
        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector4 m_value;

};

class ConstMatrix3 : public ShaderNode {
    A_OBJECT(ConstMatrix3, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(Vector3, Value0, ConstMatrix3::value0, ConstMatrix3::setValue0),
        A_PROPERTY(Vector3, Value1, ConstMatrix3::value1, ConstMatrix3::setValue1),
        A_PROPERTY(Vector3, Value2, ConstMatrix3::value2, ConstMatrix3::setValue2)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstMatrix3() {
        m_value0.x = 1.0f;
        m_value1.y = 1.0f;
        m_value2.z = 1.0f;

        m_outputs.push_back(std::make_pair("Value", MetaType::MATRIX3));

        setName("Matrix3");
    }

    Vector3 value0() const {
        return m_value0;
    }

    void setValue0(const Vector3 &value) {
        m_value0 = value;
    }

    Vector3 value1() const {
        return m_value1;
    }

    void setValue1(const Vector3 &value) {
        m_value1 = value;
    }

    Vector3 value2() const {
        return m_value2;
    }

    void setValue2(const Vector3 &value) {
        m_value2 = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("mat3(%1, %2, %3, %4, %5, %6, %7, %8, %9)")
                   .arg(m_value0.x).arg(m_value0.y).arg(m_value0.z)
                   .arg(m_value1.x).arg(m_value1.y).arg(m_value1.z)
                   .arg(m_value2.x).arg(m_value2.y).arg(m_value2.z).toStdString());

        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector3 m_value0;
    Vector3 m_value1;
    Vector3 m_value2;

};

class ConstMatrix4 : public ShaderNode {
    A_OBJECT(ConstMatrix4, ShaderNode, Shader/Constant)

    A_PROPERTIES(
        A_PROPERTY(Vector4, Value0, ConstMatrix4::value0, ConstMatrix4::setValue0),
        A_PROPERTY(Vector4, Value1, ConstMatrix4::value1, ConstMatrix4::setValue1),
        A_PROPERTY(Vector4, Value2, ConstMatrix4::value2, ConstMatrix4::setValue2),
        A_PROPERTY(Vector4, Value3, ConstMatrix4::value3, ConstMatrix4::setValue3)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ConstMatrix4() {
        m_value0.x = 1.0f;
        m_value1.y = 1.0f;
        m_value2.z = 1.0f;
        m_value3.w = 1.0f;

        m_outputs.push_back(std::make_pair("Value", MetaType::MATRIX4));

        setName("Matrix4");
    }

    Vector4 value0() const {
        return m_value0;
    }

    void setValue0(const Vector4 &value) {
        m_value0 = value;
    }

    Vector4 value1() const {
        return m_value1;
    }

    void setValue1(const Vector4 &value) {
        m_value1 = value;
    }

    Vector4 value2() const {
        return m_value2;
    }

    void setValue2(const Vector4 &value) {
        m_value2 = value;
    }

    Vector4 value3() const {
        return m_value3;
    }

    void setValue3(const Vector4 &value) {
        m_value3 = value;
    }

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("mat4(%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16)")
                   .arg(m_value0.x).arg(m_value0.y).arg(m_value0.z).arg(m_value0.w)
                   .arg(m_value1.x).arg(m_value1.y).arg(m_value1.z).arg(m_value1.w)
                   .arg(m_value2.x).arg(m_value2.y).arg(m_value2.z).arg(m_value2.w)
                   .arg(m_value3.x).arg(m_value3.y).arg(m_value3.z).arg(m_value3.w).toStdString());

        return ShaderNode::build(code, stack, link, depth, type);
    }

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

};

#endif // CONSTVALUE_H
