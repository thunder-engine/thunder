#ifndef VECTOROPERATOR_H
#define VECTOROPERATOR_H

#include "function.h"

#define AGB "A>B"
#define BGA "A<B"
#define AEB "A==B"

class Mask : public ShaderFunction {
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

        m_ports.push_back(NodePort(this, false, QMetaType::Void, 1, "Input", m_portColors[QMetaType::Void]));
        m_ports.push_back(NodePort(this, true, QMetaType::Void, 0, "Output", m_portColors[QMetaType::Void]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            const AbstractNodeGraph::Link *l = graph->findLink(this, &m_ports.front());
            if(l) {
                ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);

                int32_t l_type = 0;
                int32_t index = node->build(code, stack, graph, *l, depth, l_type);
                if(index >= 0) {
                    QString mask;
                    if(m_r && l_type > 0) {
                        mask += "r";
                        type = QMetaType::Float;
                    }
                    if(m_g && l_type > 1) {
                        mask += "g";
                        if(type == 0) {
                            type = QMetaType::Float;
                        } else {
                            type = QMetaType::QVector2D;
                        }
                    }
                    if(m_b && l_type > 2) {
                        mask += "b";
                        if(type == 0) {
                            type = QMetaType::Float;
                        } else if(type == QMetaType::Float) {
                            type = QMetaType::QVector2D;
                        } else {
                            type++;
                        }
                    }
                    if(m_a && l_type > 3) {
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

                    if(graph->isSingleConnection(link.oport)) {
                        stack.push(value);
                    } else {
                        QString s_type;
                        switch(type) {
                            case QMetaType::QVector2D: s_type = "\tvec2";  break;
                            case QMetaType::QVector3D: s_type = "\tvec3";  break;
                            case QMetaType::QVector4D: s_type = "\tvec4";  break;
                            default: s_type = "\tfloat"; break;
                        }

                        code.append(QString("%1 local%2 = %3;\n").arg(s_type, QString::number(depth), value));
                    }

                    m_type = type;
                } else {
                    return m_position;
                }
            } else {
                graph->reportMessage(this, "Missing argument");
                return m_position;
            }
        } else {
            type = m_type;
        }
        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }

    bool r() const { return m_r; }
    bool g() const { return m_g; }
    bool b() const { return m_b; }
    bool a() const { return m_a; }

    void setR(bool code) { m_r = code; emit updated();}
    void setG(bool code) { m_g = code; emit updated();}
    void setB(bool code) { m_b = code; emit updated();}
    void setA(bool code) { m_a = code; emit updated();}

private:
    bool m_r;
    bool m_g;
    bool m_b;
    bool m_a;
};

#endif // VECTOROPERATOR_H

