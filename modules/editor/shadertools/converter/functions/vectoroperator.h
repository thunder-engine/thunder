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
    A_OBJECT(CrossProduct, ShaderNode, Shader/Vector Operators)

public:
    CrossProduct() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(b, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::VECTOR3));

        m_expression = "cross";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::VECTOR3;
        return compile(code, stack, link, depth, type);
    }
};

class Distance : public ShaderNode {
    A_OBJECT(Distance, ShaderNode, Shader/Vector Operators)

public:
    Distance() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(b, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "distance";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class DotProduct : public ShaderNode {
    A_OBJECT(DotProduct, ShaderNode, Shader/Vector Operators)

public:
    DotProduct() {
        m_inputs.push_back(std::make_pair(a, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(b, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "dot";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Length : public ShaderNode {
    A_OBJECT(Length, ShaderNode, Shader/Vector Operators)

public:
    Length() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "length";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Normalize : public ShaderNode {
    A_OBJECT(Normalize, ShaderNode, Shader/Vector Operators)

public:
    Normalize() {
        m_inputs.push_back(std::make_pair(x, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::FLOAT));

        m_expression = "normalize";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::FLOAT;
        return compile(code, stack, link, depth, type);
    }
};

class Reflect : public ShaderNode {
    A_OBJECT(Reflect, ShaderNode, Shader/Vector Operators)

public:
    Reflect() {
        m_inputs.push_back(std::make_pair("i", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair("n", MetaType::VECTOR3));

        m_outputs.push_back(std::make_pair("", MetaType::VECTOR3));

        m_expression = "reflect";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::VECTOR3;
        return compile(code, stack, link, depth, type);
    }
};

class Refract : public ShaderNode {
    A_OBJECT(Refract, ShaderNode, Shader/Vector Operators)

public:
    Refract() {
        m_inputs.push_back(std::make_pair("i", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair("n", MetaType::VECTOR3));
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::VECTOR3));

        m_expression = "refract";
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        type = MetaType::VECTOR3;
        return compile(code, stack, link, depth, type);
    }
};

class Append : public VectorOperator {
    A_OBJECT(Append, ShaderNode, Shader/Vector Operators)

public:
    Append() {
        m_inputs.push_back(std::make_pair(x, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(y, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(z, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(w, MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR4));
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
                                value += convert(QString("local%1").arg(QString::number(index)), l_type, MetaType::FLOAT);
                            } else {
                                value += convert(stack.pop(), l_type, MetaType::FLOAT);
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
                code.append(localValue(MetaType::VECTOR4, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

private:
    Vector4 m_default;

};

class Split : public VectorOperator {
    A_OBJECT(Split, ShaderNode, Shader/Vector Operators)

    A_PROPERTIES(
        A_PROPERTY(float, default_X, Split::valueX, Split::setValueX),
        A_PROPERTY(float, default_Y, Split::valueY, Split::setValueY),
        A_PROPERTY(float, default_Z, Split::valueZ, Split::setValueZ),
        A_PROPERTY(float, default_W, Split::valueW, Split::setValueW)
    )


public:
    Split() {
        m_inputs.push_back(std::make_pair("Input", MetaType::VECTOR4));

        m_outputs.push_back(std::make_pair(x, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(y, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(z, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(w, MetaType::FLOAT));
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
                        value = convert(QString("local%1").arg(QString::number(index)), l_type, MetaType::VECTOR4);
                    } else {
                        value = convert(stack.pop(), l_type, MetaType::VECTOR4);
                    }
                }
            }

            for(auto &it : m_ports) {
                if(link.oport == &it) {
                    value += QString(".%1").arg(it.m_name.toLower().data());
                }
            }

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(MetaType::FLOAT, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    float valueX() const { return m_default.x; }
    float valueY() const { return m_default.y; }
    float valueZ() const { return m_default.z; }
    float valueW() const { return m_default.w; }

    void setValueX(float value) { m_default.x = value; }
    void setValueY(float value) { m_default.y = value; }
    void setValueZ(float value) { m_default.z = value; }
    void setValueW(float value) { m_default.w = value; }

private:
    Vector4 m_default;

};

class Swizzle : public VectorOperator {
    A_OBJECT(Swizzle, ShaderNode, Shader/Vector Operators)

    A_PROPERTIES(
        A_PROPERTYEX(Components, channel_0, Swizzle::channel0, Swizzle::setChannel0, "enum=Components"),
        A_PROPERTYEX(Components, channel_1, Swizzle::channel1, Swizzle::setChannel1, "enum=Components"),
        A_PROPERTYEX(Components, channel_2, Swizzle::channel2, Swizzle::setChannel2, "enum=Components"),
        A_PROPERTYEX(Components, channel_3, Swizzle::channel3, Swizzle::setChannel3, "enum=Components"),
        A_PROPERTY(float, default_X, Swizzle::valueX, Swizzle::setValueX),
        A_PROPERTY(float, default_Y, Swizzle::valueY, Swizzle::setValueY),
        A_PROPERTY(float, default_Z, Swizzle::valueZ, Swizzle::setValueZ),
        A_PROPERTY(float, default_W, Swizzle::valueW, Swizzle::setValueW)
    )
    A_ENUMS(
        A_ENUM(Components,
               A_VALUE(X),
               A_VALUE(Y),
               A_VALUE(Z),
               A_VALUE(Z))
    )

public:
    enum Components {
        X = 0,
        Y,
        Z,
        W
    };

public:
    Swizzle() {
        m_channel[0] = Components::X;
        m_channel[1] = Components::Y;
        m_channel[2] = Components::Z;
        m_channel[3] = Components::W;

        m_inputs.push_back(std::make_pair("Input", MetaType::VECTOR4));

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR4));
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
                        value = convert(QString("local%1").arg(QString::number(index)), l_type, MetaType::VECTOR4);
                    } else {
                        value = convert(stack.pop(), l_type, MetaType::VECTOR4);
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
                code.append(localValue(MetaType::VECTOR4, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    Components channel0() const { return m_channel[0]; }
    Components channel1() const { return m_channel[1]; }
    Components channel2() const { return m_channel[2]; }
    Components channel3() const { return m_channel[3]; }

    void setChannel0(Components value) { m_channel[0] = value; }
    void setChannel1(Components value) { m_channel[1] = value; }
    void setChannel2(Components value) { m_channel[2] = value; }
    void setChannel3(Components value) { m_channel[3] = value; }

    float valueX() const { return m_default.x; }
    float valueY() const { return m_default.y; }
    float valueZ() const { return m_default.z; }
    float valueW() const { return m_default.w; }

    void setValueX(float value) { m_default.x = value; }
    void setValueY(float value) { m_default.y = value; }
    void setValueZ(float value) { m_default.z = value; }
    void setValueW(float value) { m_default.w = value; }

private:
    Vector4 m_default;

    Components m_channel[4];

};

class Mask : public VectorOperator {
    A_OBJECT(Mask, ShaderNode, Shader/Vector Operators)

    A_PROPERTIES(
        A_PROPERTY(bool, R, Mask::r, Mask::setR),
        A_PROPERTY(bool, G, Mask::g, Mask::setG),
        A_PROPERTY(bool, B, Mask::b, Mask::setB),
        A_PROPERTY(bool, A, Mask::a, Mask::setA)
    )

public:
    Mask() :
            m_r(true),
            m_g(true),
            m_b(true),
            m_a(true) {

        m_inputs.push_back(std::make_pair("Input", MetaType::VECTOR4));

        m_outputs.push_back(std::make_pair("Output", MetaType::INVALID));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            const AbstractNodeGraph::Link *l = m_graph->findLink(this, &m_ports.back());
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);

                int32_t l_type = MetaType::VECTOR4;
                int32_t index = node->build(code, stack, *l, depth, l_type);
                if(index >= 0) {
                    QString mask;
                    if(m_r) {
                        mask += "r";
                        type = MetaType::FLOAT;
                    }
                    if(m_g) {
                        mask += "g";
                        if(type == 0) {
                            type = MetaType::FLOAT;
                        } else {
                            type = MetaType::VECTOR2;
                        }
                    }
                    if(m_b) {
                        mask += "b";
                        if(type == 0) {
                            type = MetaType::FLOAT;
                        } else if(type == MetaType::FLOAT) {
                            type = MetaType::VECTOR2;
                        } else {
                            type++;
                        }
                    }
                    if(m_a) {
                        mask += "a";
                        if(type == 0) {
                            type = MetaType::FLOAT;
                        } else if(type == MetaType::FLOAT) {
                            type = MetaType::VECTOR2;
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

    void setR(bool code) { m_r = code; }
    void setG(bool code) { m_g = code; }
    void setB(bool code) { m_b = code; }
    void setA(bool code) { m_a = code; }

private:
    bool m_r;
    bool m_g;
    bool m_b;
    bool m_a;

};

#endif // VECTOROPERATOR_H

