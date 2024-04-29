#ifndef MATRIXOPERATIONS_H
#define MATRIXOPERATIONS_H

#include "function.h"

class MatrixOperation : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Matrix Operations")

    Q_PROPERTY(Vector4 Value0 READ value0 WRITE setValue0 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Value1 READ value1 WRITE setValue1 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Value2 READ value2 WRITE setValue2 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Value3 READ value3 WRITE setValue3 NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE MatrixOperation() {
        m_inputs.push_back(make_pair("Matrix", QMetaType::QMatrix4x4));

        m_type = QMetaType::QMatrix4x4;

        m_value0[0] = 1.0f;
        m_value1[1] = 1.0f;
        m_value2[2] = 1.0f;
        m_value3[3] = 1.0f;
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            type = m_outputs.front().second;

            QStringList arguments = getArguments(code, stack, depth, type);

            QString expr = QString("%1(%2)").arg(m_function, arguments[0]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(QMetaType::Float, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const string &, uint32_t &type) const override {
        type = QMetaType::QMatrix4x4;
        return QString("mat4(%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16)")
                .arg(m_value0.x).arg(m_value1.x).arg(m_value2.x).arg(m_value3.x)
                .arg(m_value0.y).arg(m_value1.y).arg(m_value2.y).arg(m_value3.y)
                .arg(m_value0.z).arg(m_value1.z).arg(m_value2.z).arg(m_value3.z)
                .arg(m_value0.w).arg(m_value1.w).arg(m_value2.w).arg(m_value3.w);
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

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

    QString m_function;

    uint16_t m_type;

};

class Determinant : public MatrixOperation {
    Q_OBJECT
public:
    Q_INVOKABLE Determinant() {
        m_function = "determinant";
        m_outputs.push_back(make_pair("Output", QMetaType::Float));
    }
};

class Inverse : public MatrixOperation {
    Q_OBJECT
public:
    Q_INVOKABLE Inverse() {
        m_function = "inverse";
        m_outputs.push_back(make_pair("Output", QMetaType::QMatrix4x4));
    }
};

class Transpose : public MatrixOperation {
    Q_OBJECT
public:
    Q_INVOKABLE Transpose() {
        m_function = "transpose";
        m_outputs.push_back(make_pair("Output", QMetaType::QMatrix4x4));
    }
};

class ExtractPosition : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Matrix Operations")

    Q_PROPERTY(Vector4 Vector0 READ value0 WRITE setValue0 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Vector1 READ value1 WRITE setValue1 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Vector2 READ value2 WRITE setValue2 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Vector3 READ value3 WRITE setValue3 NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE ExtractPosition() {
        m_inputs.push_back(make_pair("Matrix", QMetaType::QMatrix4x4));

        m_outputs.push_back(make_pair("XYZW", QMetaType::QVector4D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            type = m_outputs.front().second;

            QString func = "vec4 ExtractPosition(mat4 m) {\n"
                           "    return vec4(m[3][0], m[3][1], m[3][2], m[3][3]);\n"
                           "}\n";

            static_cast<ShaderGraph *>(m_graph)->addFunction("ExtractPosition", func);

            QStringList arguments = getArguments(code, stack, depth, type);

            QString expr = QString("ExtractPosition(%1)").arg(arguments[0]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(QMetaType::Float, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const string &, uint32_t &type) const override {
        type = QMetaType::QMatrix4x4;
        return QString("mat4(%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16)")
                .arg(m_value0.x).arg(m_value1.x).arg(m_value2.x).arg(m_value3.x)
                .arg(m_value0.y).arg(m_value1.y).arg(m_value2.y).arg(m_value3.y)
                .arg(m_value0.z).arg(m_value1.z).arg(m_value2.z).arg(m_value3.z)
                .arg(m_value0.w).arg(m_value1.w).arg(m_value2.w).arg(m_value3.w);
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

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

};

class MakeMatrix : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Matrix Operations")

    Q_PROPERTY(Vector4 Vector0 READ value0 WRITE setValue0 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Vector1 READ value1 WRITE setValue1 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Vector2 READ value2 WRITE setValue2 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Vector4 Vector3 READ value3 WRITE setValue3 NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE MakeMatrix() {
        m_inputs.push_back(make_pair("Vector0", QMetaType::QVector4D));
        m_inputs.push_back(make_pair("Vector1", QMetaType::QVector4D));
        m_inputs.push_back(make_pair("Vector2", QMetaType::QVector4D));
        m_inputs.push_back(make_pair("Vector3", QMetaType::QVector4D));

        m_outputs.push_back(make_pair("", QMetaType::QMatrix4x4));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            type = m_outputs.front().second;

            QStringList arguments = getArguments(code, stack, depth, type);

            QString expr = QString("mat4(%1, %2, %3, %4)")
                            .arg(arguments[0], arguments[1], arguments[2], arguments[3]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(QMetaType::Float, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const string &key, uint32_t &) const override {
        if(key == "Vector0") {
            return QString("vec4(%1, %2, %3, %4)")
                    .arg(m_value0.x).arg(m_value0.y).arg(m_value0.z).arg(m_value0.w);
        } else if(key == "Vector1") {
            return QString("vec4(%1, %2, %3, %4)")
                    .arg(m_value1.x).arg(m_value1.y).arg(m_value1.z).arg(m_value1.w);
        } else if(key == "Vector2") {
            return QString("vec4(%1, %2, %3, %4)")
                    .arg(m_value2.x).arg(m_value2.y).arg(m_value2.z).arg(m_value2.w);
        } else if(key == "Vector3") {
            return QString("vec4(%1, %2, %3, %4)")
                    .arg(m_value3.x).arg(m_value3.y).arg(m_value3.z).arg(m_value3.w);
        }
        return QString();
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

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

};

#endif // MATRIXOPERATIONS_H
