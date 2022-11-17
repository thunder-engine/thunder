#ifndef MATHOPERATOR
#define MATHOPERATOR

#include "function.h"

class MathOperation : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Operations")

public:
    MathOperation() {
        m_ports.push_back(NodePort(this, false, QMetaType::QVector2D, 1, a, m_portColors[QMetaType::QVector2D]));
        m_ports.push_back(NodePort(this, false, QMetaType::QVector2D, 2, b, m_portColors[QMetaType::QVector2D]));
        m_ports.push_back(NodePort(this, true,  QMetaType::QVector2D, 0, "Output", m_portColors[QMetaType::QVector2D]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t compile(const QString &operation, QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        if(m_position == -1) {
            QString args("(");

            for(NodePort &it : m_ports) {
                if(it.m_out) {
                    continue;
                }
                const AbstractNodeGraph::Link *l = graph->findLink(this, &it);
                if(l) {
                    ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);
                    if(node) {
                        int32_t l_type = 0;
                        int32_t index = node->build(code, stack, graph, *l, depth, l_type);
                        if(index >= 0) {
                            if(portPosition(&it) == 1) { // take A type
                                if(type == 0) {
                                    type = l_type;
                                    m_type = type;
                                }
                            } else {
                                args.append(operation);
                            }

                            if(stack.isEmpty()) {
                                args.append(convert("local" + QString::number(index), type, type));
                            } else {
                                args.append(convert(stack.pop(), type, type));
                            }
                        } else {
                            return m_position;
                        }
                    }
                } else {
                    graph->reportMessage(this, QString("Missing argument ") + it.m_name.c_str());
                    return m_position;
                }
            }
            args.append(")");

            if(graph->isSingleConnection(link.oport)) {
                stack.push(args);
            } else {
                switch(type) {
                    case QMetaType::QVector2D: code.append("\tvec2"); break;
                    case QMetaType::QVector3D: code.append("\tvec3"); break;
                    case QMetaType::QVector4D: code.append("\tvec4"); break;
                    default: code.append("\tfloat"); break;
                }

                code.append(QString(" local%1 = %2;\n").arg(depth).arg(args));
            }
        } else {
            type = m_type;
        }

        return ShaderFunction::build(code, stack, graph, link, depth, type);
    }
};

class Subtraction : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Subtraction() {}

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" - ", code, stack, graph, link, depth, type);
    }
};

class Add : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Add() {}

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" + ", code, stack, graph, link, depth, type);
    }
};

class Divide : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Divide() {}

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" / ", code, stack, graph, link, depth, type);
    }
};

class Multiply : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Multiply() {}

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        return compile(" * ", code, stack, graph, link, depth, type);
    }
};

#endif // MATHOPERATOR

