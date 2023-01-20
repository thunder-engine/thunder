#ifndef FUNCTION_H
#define FUNCTION_H

#include "../shadernodegraph.h"

#include <editor/assetconverter.h>
#include <editor/graph/graphnode.h>

#include <QStack>

namespace {
    static const char *a("A");
    static const char *b("B");
    static const char *r("R");
    static const char *g("G");
    static const char *x("X");
    static const char *y("Y");
    static const char *z("Z");
    static const char *w("W");
}

class ShaderFunction : public GraphNode {
    Q_OBJECT

public:
    ShaderFunction() {
        reset();
    }

    void reset() {
        m_position = -1;
        m_type = 0;
    }

    Vector4 color() const override {
        return Vector4(0.513f, 0.192f, 0.290f, 1.0f);
    }

    virtual int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        Q_UNUSED(code)
        Q_UNUSED(link)

        if(type == 0) {
            type = link.oport->m_type;
        }

        if(m_position == -1) {
            m_position = depth;
            depth++;
        }
        return m_position;
    }

    static QString convert(const QString &value, uint8_t current, uint32_t target, uint8_t component = 0) {
        QString prefix;
        QString suffix;

        const char *names[] = {".x", ".y", ".z", ".w"};

        switch(target) {
            case QMetaType::Float: {
                switch(current) {
                    case QMetaType::QVector2D:
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = names[component]; } break;
                    case QMetaType::QTransform:
                    case QMetaType::QMatrix4x4: { prefix = ""; suffix = QString("[0]") + names[component]; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector2D: {
                switch(current) {
                    case QMetaType::Float:      { prefix = "vec2("; suffix = ")"; } break;
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xy"; } break;
                    case QMetaType::QTransform: { prefix = ""; suffix = "[0].xy"; } break;
                    case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0].xy"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector3D: {
                switch(current) {
                    case QMetaType::Float:      { prefix = "vec3("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec3("; suffix = ", 0.0)"; } break;
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xyz"; } break;
                    case QMetaType::QTransform: { prefix = ""; suffix = "[0].xyz"; } break;
                    case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0].xyz"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector4D: {
                switch(current) {
                    case QMetaType::Float:      { prefix = "vec4("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec4("; suffix = ", 0.0, 1.0)"; } break;
                    case QMetaType::QVector3D:  { prefix = "vec4("; suffix = ", 1.0)"; } break;
                    case QMetaType::QTransform: { prefix = "vec4("; suffix = "[0], 1.0)"; } break;
                    case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0]"; } break;
                    default: break;
                }
            } break;
            default: break;
        }
        return(prefix + value + suffix);
    }

protected:
    friend class ShaderNodeGraph;

    int32_t m_position;
    int32_t m_type;

    static map<uint32_t, Vector4> m_portColors;

};

class MathFunction : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t compile(const QString &func, QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type, int32_t expect = 0, int32_t last = 0) {
        if(m_position == -1) {
            QString args;

            int i = 0;
            for(NodePort &it : m_ports) {
                if(it.m_out == true) {
                    continue;
                }
                const AbstractNodeGraph::Link *l = m_graph->findLink(this, &it);
                if(l) {
                    ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);
                    if(node) {
                        int32_t type = 0;
                        int32_t index = node->build(code, stack, *l, depth, type);
                        if(index >= 0) {
                            if(i == 0 && !expect) {
                                expect = type;
                            }

                            uint8_t final = expect;
                            if(i == (m_params.size() - 1) && last) {
                                final = last;
                            }

                            if(stack.isEmpty()) {
                                args.append(convert(QString("local%1").arg(index), type, final));
                            } else {
                                args.append(convert(stack.pop(), type, final));
                            }

                            args.append(((i == m_params.size() - 1) ? "" : ", "));
                        } else {
                            return index;
                        }
                    }
                } else {
                    m_graph->reportMessage(this, QString("Missing argument ") + it.m_name.c_str());
                    return m_position;
                }
                i++;
            }

            if(!type) {
                type = expect;
                m_type = type;
            }

            switch(type) {
                case QMetaType::QVector2D: code.append("\tvec2");  break;
                case QMetaType::QVector3D: code.append("\tvec3");  break;
                case QMetaType::QVector4D: code.append("\tvec4");  break;
                default: code.append("\tfloat"); break;
            }
            code.append(QString(" local%1 = %2(%3);\n").arg(QString::number(depth), func, args));
        } else {
            type = m_type;
        }
        return ShaderFunction::build(code, stack, link, depth, type);
    }

    void createParams() {
        int i = 0;
        for(QString &it : m_params) {
            m_ports.push_back(NodePort(this, false, QMetaType::Void, i + 1, qPrintable(it), m_portColors[QMetaType::Void]));
            i++;
        }

        m_ports.push_back(NodePort(this, true, QMetaType::Void, 0, "Output", m_portColors[QMetaType::Void]));
    }

protected:
    QStringList m_params;

};

#endif // FUNCTION_H
