#ifndef MATRIXOPERATIONS_H
#define MATRIXOPERATIONS_H

#include "function.h"

class MatrixOperation : public ShaderNode {
    A_OBJECT(MatrixOperation, ShaderNode, Shader/Matrix Operations)

    A_PROPERTIES(
        A_PROPERTY(Vector4, Value0, MatrixOperation::value0, MatrixOperation::setValue0),
        A_PROPERTY(Vector4, Value1, MatrixOperation::value1, MatrixOperation::setValue1),
        A_PROPERTY(Vector4, Value2, MatrixOperation::value2, MatrixOperation::setValue2),
        A_PROPERTY(Vector4, Value3, MatrixOperation::value3, MatrixOperation::setValue3)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    MatrixOperation() {
        m_inputs.push_back(std::make_pair("Matrix", MetaType::MATRIX4));

        m_type = MetaType::MATRIX4;

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
                code.append(localValue(MetaType::FLOAT, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const TString &, uint32_t &type) const override {
        type = MetaType::MATRIX4;
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

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

    QString m_function;

    uint16_t m_type;

};

class Determinant : public MatrixOperation {
    A_OBJECT(Determinant, MatrixOperation, Shader/Matrix Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Determinant() {
        m_function = "determinant";
        m_outputs.push_back(std::make_pair("Output", MetaType::FLOAT));
    }
};

class Inverse : public MatrixOperation {
    A_OBJECT(Inverse, MatrixOperation, Shader/Matrix Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Inverse() {
        m_function = "inverse";
        m_outputs.push_back(std::make_pair("Output", MetaType::MATRIX4));
    }
};

class Transpose : public MatrixOperation {
    A_OBJECT(Transpose, MatrixOperation, Shader/Matrix Operations)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Transpose() {
        m_function = "transpose";
        m_outputs.push_back(std::make_pair("Output", MetaType::MATRIX4));
    }
};

class ExtractPosition : public ShaderNode {
    A_OBJECT(ExtractPosition, ShaderNode, Shader/Matrix Operations)

    A_PROPERTIES(
        A_PROPERTY(Vector4, Vector0, ExtractPosition::value0, ExtractPosition::setValue0),
        A_PROPERTY(Vector4, Vector1, ExtractPosition::value1, ExtractPosition::setValue1),
        A_PROPERTY(Vector4, Vector2, ExtractPosition::value2, ExtractPosition::setValue2),
        A_PROPERTY(Vector4, Vector3, ExtractPosition::value3, ExtractPosition::setValue3)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ExtractPosition() {
        m_inputs.push_back(std::make_pair("Matrix", MetaType::MATRIX4));

        m_outputs.push_back(std::make_pair("XYZW", MetaType::VECTOR4));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            type = m_outputs.front().second;

            TString func = "vec4 ExtractPosition(mat4 m) {\n"
                          "    return vec4(m[3][0], m[3][1], m[3][2], m[3][3]);\n"
                          "}\n";

            static_cast<ShaderGraph *>(m_graph)->addFunction("ExtractPosition", func);

            QStringList arguments = getArguments(code, stack, depth, type);

            QString expr = QString("ExtractPosition(%1)").arg(arguments[0]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(MetaType::FLOAT, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const TString &, uint32_t &type) const override {
        type = MetaType::MATRIX4;
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

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

};

class MakeMatrix : public ShaderNode {
    A_OBJECT(MakeMatrix, ShaderNode, Shader/Matrix Operations)

    A_PROPERTIES(
        A_PROPERTY(Vector4, Vector0, MakeMatrix::value0, MakeMatrix::setValue0),
        A_PROPERTY(Vector4, Vector1, MakeMatrix::value1, MakeMatrix::setValue1),
        A_PROPERTY(Vector4, Vector2, MakeMatrix::value2, MakeMatrix::setValue2),
        A_PROPERTY(Vector4, Vector3, MakeMatrix::value3, MakeMatrix::setValue3)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    MakeMatrix() {
        m_inputs.push_back(std::make_pair("Vector0", MetaType::VECTOR4));
        m_inputs.push_back(std::make_pair("Vector1", MetaType::VECTOR4));
        m_inputs.push_back(std::make_pair("Vector2", MetaType::VECTOR4));
        m_inputs.push_back(std::make_pair("Vector3", MetaType::VECTOR4));

        m_outputs.push_back(std::make_pair("", MetaType::MATRIX4));
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
                code.append(localValue(MetaType::FLOAT, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const TString &key, uint32_t &) const override {
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

protected:
    Vector4 m_value0;
    Vector4 m_value1;
    Vector4 m_value2;
    Vector4 m_value3;

};

#endif // MATRIXOPERATIONS_H
