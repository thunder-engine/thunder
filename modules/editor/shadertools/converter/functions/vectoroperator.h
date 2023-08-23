#ifndef VECTOROPERATOR_H
#define VECTOROPERATOR_H

#include "function.h"

class VectorOperator : public ShaderNode {
private:
    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }
};

class CrossProduct : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE CrossProduct() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));
        m_inputs.push_back(make_pair(b, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::QVector3D));

        m_expression = "cross";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::QVector3D;
        return compile(code, stack, link, depth, type);
    }
};

class Distance : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE Distance() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));
        m_inputs.push_back(make_pair(b, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Float));

        m_expression = "distance";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class DotProduct : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE DotProduct() {
        m_inputs.push_back(make_pair(a, QMetaType::Void));
        m_inputs.push_back(make_pair(b, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Float));

        m_expression = "dot";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Length : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE Length() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Float));

        m_expression = "length";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Normalize : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE Normalize() {
        m_inputs.push_back(make_pair(x, QMetaType::Void));

        m_outputs.push_back(make_pair("", QMetaType::Float));

        m_expression = "normalize";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::Float;
        return compile(code, stack, link, depth, type);
    }
};

class Reflect : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE Reflect() {
        m_inputs.push_back(make_pair("i", QMetaType::QVector3D));
        m_inputs.push_back(make_pair("n", QMetaType::QVector3D));

        m_outputs.push_back(make_pair("", QMetaType::QVector3D));

        m_expression = "reflect";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::QVector3D;
        return compile(code, stack, link, depth, type);
    }
};

class Refract : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE Refract() {
        m_inputs.push_back(make_pair("i", QMetaType::QVector3D));
        m_inputs.push_back(make_pair("n", QMetaType::QVector3D));
        m_inputs.push_back(make_pair(a, QMetaType::Float));

        m_outputs.push_back(make_pair("", QMetaType::QVector3D));

        m_expression = "refract";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = QMetaType::QVector3D;
        return compile(code, stack, link, depth, type);
    }
};

class Append : public VectorOperator {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

public:
    Q_INVOKABLE Append() {
        m_inputs.push_back(make_pair(x, QMetaType::Float));
        m_inputs.push_back(make_pair(y, QMetaType::Float));
        m_inputs.push_back(make_pair(z, QMetaType::Float));
        m_inputs.push_back(make_pair(w, QMetaType::Float));

        m_outputs.push_back(make_pair("Output", QMetaType::QVector4D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            int i = 0;

            QString value("vec4(");

            for(auto &it : m_ports) {
                if(it.m_out == false) {
                    const AbstractNodeGraph::Link *l = m_graph->findLink(this, &it);
                    if(l) {
                        ShaderNode *node = static_cast<ShaderNode *>(l->sender);

                        int32_t l_type = 0;
                        int32_t index = node->build(code, stack, *l, depth, l_type);
                        if(index >= 0) {
                            if(stack.isEmpty()) {
                                value += convert(QString("local%1").arg(QString::number(index)), l_type, QMetaType::Float);
                            } else {
                                value += convert(stack.pop(), l_type, QMetaType::Float);
                            }
                        } else {
                            value += QString::number(m_default[i]);
                        }
                    } else {
                        value += QString::number(m_default[i]);
                    }
                    if(i < 3) {
                        value += ", ";
                    }
                    ++i;
                }
            }
            value += ")";

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(QMetaType::QVector4D, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

private:
    Vector4 m_default;

};

class Split : public VectorOperator {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

    Q_PROPERTY(float default_X READ valueX WRITE setValueX NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float default_Y READ valueY WRITE setValueY NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float default_Z READ valueZ WRITE setValueZ NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float default_W READ valueW WRITE setValueW NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Split() {
        m_inputs.push_back(make_pair("Input", QMetaType::QVector4D));

        m_outputs.push_back(make_pair(x, QMetaType::Float));
        m_outputs.push_back(make_pair(y, QMetaType::Float));
        m_outputs.push_back(make_pair(z, QMetaType::Float));
        m_outputs.push_back(make_pair(w, QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            int i = 0;

            QString value = QString("vec4(%1, %2, %3, %4)").arg(m_default.x, m_default.y, m_default.z, m_default.w);

            const AbstractNodeGraph::Link *l = m_graph->findLink(this, &m_ports.back());
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);

                int32_t l_type = 0;
                int32_t index = node->build(code, stack, *l, depth, l_type);
                if(index >= 0) {
                    if(stack.isEmpty()) {
                        value = convert(QString("local%1").arg(QString::number(index)), l_type, QMetaType::QVector4D);
                    } else {
                        value = convert(stack.pop(), l_type, QMetaType::QVector4D);
                    }
                }
            }

            for(auto &it : m_ports) {
                if(link.oport == &it) {
                    value += QString(".%1").arg(QString(it.m_name.c_str()).toLower());
                }
            }

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(QMetaType::Float, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    float valueX() const { return m_default.x; }
    float valueY() const { return m_default.y; }
    float valueZ() const { return m_default.z; }
    float valueW() const { return m_default.w; }

    void setValueX(float value) { m_default.x = value; emit updated(); }
    void setValueY(float value) { m_default.y = value; emit updated(); }
    void setValueZ(float value) { m_default.z = value; emit updated(); }
    void setValueW(float value) { m_default.w = value; emit updated(); }

private:
    Vector4 m_default;

};

class Swizzle : public VectorOperator {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

    Q_PROPERTY(Components channel_0 READ channel0 WRITE setChannel0 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Components channel_1 READ channel1 WRITE setChannel1 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Components channel_2 READ channel2 WRITE setChannel2 NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Components channel_3 READ channel3 WRITE setChannel3 NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(float default_X READ valueX WRITE setValueX NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float default_Y READ valueY WRITE setValueY NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float default_Z READ valueZ WRITE setValueZ NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float default_W READ valueW WRITE setValueW NOTIFY updated DESIGNABLE true USER true)

public:
    enum class Components {
        X = 0,
        Y,
        Z,
        W
    };
    Q_ENUM(Components)

public:
    Q_INVOKABLE Swizzle() {
        m_channel[0] = Components::X;
        m_channel[1] = Components::Y;
        m_channel[2] = Components::Z;
        m_channel[3] = Components::W;

        m_inputs.push_back(make_pair("Input", QMetaType::QVector4D));

        m_outputs.push_back(make_pair("Output", QMetaType::QVector4D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            int i = 0;

            QString value = QString("vec4(%1, %2, %3, %4)").arg(m_default.x, m_default.y, m_default.z, m_default.w);

            const AbstractNodeGraph::Link *l = m_graph->findLink(this, &m_ports.back());
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);

                int32_t l_type = 0;
                int32_t index = node->build(code, stack, *l, depth, l_type);
                if(index >= 0) {
                    if(stack.isEmpty()) {
                        value = convert(QString("local%1").arg(QString::number(index)), l_type, QMetaType::QVector4D);
                    } else {
                        value = convert(stack.pop(), l_type, QMetaType::QVector4D);
                    }
                }
            }

            value += ".";
            const char names[] = {'x', 'y', 'z', 'w'};
            for(int i = 0; i < 4; i++) {
                value += names[static_cast<int>(m_channel[i])];
            }

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(QMetaType::QVector4D, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    Components channel0() const { return m_channel[0]; }
    Components channel1() const { return m_channel[1]; }
    Components channel2() const { return m_channel[2]; }
    Components channel3() const { return m_channel[3]; }

    void setChannel0(Components value) { m_channel[0] = value; emit updated(); }
    void setChannel1(Components value) { m_channel[1] = value; emit updated(); }
    void setChannel2(Components value) { m_channel[2] = value; emit updated(); }
    void setChannel3(Components value) { m_channel[3] = value; emit updated(); }

    float valueX() const { return m_default.x; }
    float valueY() const { return m_default.y; }
    float valueZ() const { return m_default.z; }
    float valueW() const { return m_default.w; }

    void setValueX(float value) { m_default.x = value; emit updated(); }
    void setValueY(float value) { m_default.y = value; emit updated(); }
    void setValueZ(float value) { m_default.z = value; emit updated(); }
    void setValueW(float value) { m_default.w = value; emit updated(); }

private:
    Vector4 m_default;

    Components m_channel[4];

};

class Mask : public VectorOperator {
    Q_OBJECT
    Q_CLASSINFO("Group", "Vector Operators")

    Q_PROPERTY(bool R READ r WRITE setR NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool G READ g WRITE setG NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool B READ b WRITE setB NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool A READ a WRITE setA NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Mask() :
        m_r(true),
        m_g(true),
        m_b(true),
        m_a(true) {

        m_inputs.push_back(make_pair("Input", QMetaType::QVector4D));

        m_outputs.push_back(make_pair("Output", QMetaType::Void));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            const AbstractNodeGraph::Link *l = m_graph->findLink(this, &m_ports.back());
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);

                int32_t l_type = QMetaType::QVector4D;
                int32_t index = node->build(code, stack, *l, depth, l_type);
                if(index >= 0) {
                    QString mask;
                    if(m_r) {
                        mask += "r";
                        type = QMetaType::Float;
                    }
                    if(m_g) {
                        mask += "g";
                        if(type == 0) {
                            type = QMetaType::Float;
                        } else {
                            type = QMetaType::QVector2D;
                        }
                    }
                    if(m_b) {
                        mask += "b";
                        if(type == 0) {
                            type = QMetaType::Float;
                        } else if(type == QMetaType::Float) {
                            type = QMetaType::QVector2D;
                        } else {
                            type++;
                        }
                    }
                    if(m_a) {
                        mask += "a";
                        if(type == 0) {
                            type = QMetaType::Float;
                        } else if(type == QMetaType::Float) {
                            type = QMetaType::QVector2D;
                        } else {
                            type++;
                        }
                    }

                    QString value;
                    if(stack.isEmpty()) {
                        value = QString("local%1.%2").arg(QString::number(index), mask);
                    } else {
                        value = QString("%1.%2").arg(stack.pop(), mask);
                    }

                    if(m_graph->isSingleConnection(link.oport)) {
                        stack.push(value);
                    } else {
                        code.append(localValue(type, depth, value));
                    }

                    m_type = type;
                } else {
                    return m_position;
                }
            } else {
                m_graph->reportMessage(this, "Missing argument");
                return m_position;
            }
        } else {
            type = m_type;
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    bool r() const { return m_r; }
    bool g() const { return m_g; }
    bool b() const { return m_b; }
    bool a() const { return m_a; }

    void setR(bool code) { m_r = code; emit updated(); }
    void setG(bool code) { m_g = code; emit updated(); }
    void setB(bool code) { m_b = code; emit updated(); }
    void setA(bool code) { m_a = code; emit updated(); }

private:
    bool m_r;
    bool m_g;
    bool m_b;
    bool m_a;

};

#endif // VECTOROPERATOR_H

