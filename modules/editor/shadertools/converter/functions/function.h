#ifndef FUNCTION_H
#define FUNCTION_H

#include "../shadernodegraph.h"

#include <editor/assetconverter.h>
#include <editor/graph/graphnode.h>

#include <QStack>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QDynamicPropertyChangeEvent>

namespace {
    static const char *a("A");
    static const char *b("B");
    static const char *d("D");
    static const char *r("R");
    static const char *g("G");
    static const char *x("X");
    static const char *y("Y");
    static const char *z("Z");
    static const char *w("W");
}

class ShaderNode : public GraphNode {
    Q_OBJECT

public:
    ShaderNode() {
        reset();
    }

    void reset() {
        m_position = -1;
        m_type = 0;
    }

    Vector4 color() const override {
        return Vector4(0.513f, 0.192f, 0.290f, 1.0f);
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    virtual void createParams() {
        int i = 0;
        for(auto &it : m_outputs) {
            m_ports.push_back(NodePort(this, true, it.second, i, it.first, m_portColors[it.second]));
            i++;
        }

        for(auto &it : m_inputs) {
            m_ports.push_back(NodePort(this, false, it.second, i, it.first, m_portColors[it.second]));
            i++;
        }
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

    int32_t compile(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
        if(m_position == -1) {
            QStringList args = getArguments(code, stack, depth, type);

            if(args.size() == m_inputs.size()) {
                QString expr = makeExpression(args);
                if(m_graph->isSingleConnection(link.oport)) {
                    stack.push(expr);
                } else {
                    code.append(localValue(type, depth, expr));
                }
            } else {
                m_graph->reportMessage(this, QString("Missing argument"));
                return m_position;
            }
        } else {
            type = m_type;
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    virtual QString makeExpression(const QStringList &args) const {
        return QString("%1(%2)").arg(m_expression, args.join(", "));
    }

    QStringList getArguments(QString &code, QStack<QString> &stack, int32_t &depth, int32_t &type) {
        QStringList result;

        for(const NodePort &it : m_ports) {
            if(it.m_out == true) {
                continue;
            }

            uint32_t defaultType = 0;
            QString value = defaultValue(it.m_name, defaultType);

            const AbstractNodeGraph::Link *l = m_graph->findLink(this, &it);
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);

                int32_t l_type = 0;
                int32_t index = node->build(code, stack, *l, depth, l_type);
                if(index >= 0) {
                    type = getOutType(l_type, l);

                    if(stack.isEmpty()) {
                        value = QString("local%1").arg(QString::number(index));
                    } else {
                        value = stack.pop();
                    }
                }
            }

            if(!value.isEmpty()) {
                result << value;
            }
        }

        return result;
    }

    virtual QString defaultValue(const string &key, uint32_t &type) const {
        Q_UNUSED(key);
        type = QMetaType::Void;
        return QString();
    }

    virtual int getOutType(int inType, const AbstractNodeGraph::Link *l) {
        Q_UNUSED(l);

        if(m_type == 0) {
            m_type = inType;
        }
        return m_type;
    }

    static QString convert(const QString &value, uint32_t current, uint32_t target, uint8_t component = 0) {
        QString prefix;
        QString suffix;

        const char *names[] = {".x", ".y", ".z", ".w"};

        switch(target) {
            case QMetaType::Int: {
                switch(current) {
                case QMetaType::Float:      { prefix = "int("; suffix = ")"; } break;
                case QMetaType::QVector2D:
                case QMetaType::QVector3D:
                case QMetaType::QVector4D:  { prefix = "int("; suffix = QString(names[component]) + ")"; } break;
                case QMetaType::QTransform:
                case QMetaType::QMatrix4x4: { prefix = "int("; suffix = QString("[0]") + QString(names[component]) + ")"; } break;
                case QMetaType::QImage:     { prefix = "int(texture("; suffix = ", _uv0).x)"; } break;
                default: break;
                }
            }
            case QMetaType::Float: {
                switch(current) {
                case QMetaType::Int:        { prefix = "float("; suffix = ")"; } break;
                case QMetaType::QVector2D:
                case QMetaType::QVector3D:
                case QMetaType::QVector4D:  { prefix = ""; suffix = names[component]; } break;
                case QMetaType::QTransform:
                case QMetaType::QMatrix4x4: { prefix = ""; suffix = QString("[0]") + names[component]; } break;
                case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0).x"; } break;
                default: break;
                }
            } break;
            case QMetaType::QVector2D: {
                switch(current) {
                case QMetaType::Int:        { prefix = "vec2(float("; suffix = "))"; } break;
                case QMetaType::Float:      { prefix = "vec2("; suffix = ")"; } break;
                case QMetaType::QVector3D:
                case QMetaType::QVector4D:  { prefix = ""; suffix = ".xy"; } break;
                case QMetaType::QTransform: { prefix = ""; suffix = "[0].xy"; } break;
                case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0].xy"; } break;
                case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0).xy"; } break;
                default: break;
                }
            } break;
            case QMetaType::QVector3D: {
                switch(current) {
                case QMetaType::Int:        { prefix = "vec3(float("; suffix = "))"; } break;
                case QMetaType::Float:      { prefix = "vec3("; suffix = ")"; } break;
                case QMetaType::QVector2D:  { prefix = "vec3("; suffix = ", 0.0)"; } break;
                case QMetaType::QVector4D:  { prefix = ""; suffix = ".xyz"; } break;
                case QMetaType::QTransform: { prefix = ""; suffix = "[0].xyz"; } break;
                case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0].xyz"; } break;
                case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0).xyz"; } break;
                default: break;
                }
            } break;
            case QMetaType::QVector4D: {
                switch(current) {
                case QMetaType::Int:        { prefix = "vec4(float("; suffix = "))"; } break;
                case QMetaType::Float:      { prefix = "vec4("; suffix = ")"; } break;
                case QMetaType::QVector2D:  { prefix = "vec4("; suffix = ", 0.0, 1.0)"; } break;
                case QMetaType::QVector3D:  { prefix = "vec4("; suffix = ", 1.0)"; } break;
                case QMetaType::QTransform: { prefix = "vec4("; suffix = "[0], 1.0)"; } break;
                case QMetaType::QMatrix4x4: { prefix = ""; suffix = "[0]"; } break;
                case QMetaType::QImage:     { prefix = "texture("; suffix = ", _uv0)"; } break;
                default: break;
                }
            } break;
            default: break;
        }
        return (prefix + value + suffix);
    }

    static QString localValue(int type, int index, const QString &value, const QString &name = QString()) {
        QString s_name = name;
        if(s_name.isEmpty()) {
            s_name = "local" + QString::number(index);
        }

        return QString("\t%1 %2 = %3;\n").arg(typeToString(type), s_name, value);
    }

    static QString typeToString(int type) {
        switch(type) {
            case QMetaType::Int: return "int"; break;
            case QMetaType::QVector2D: return "vec2"; break;
            case QMetaType::QVector3D: return "vec3"; break;
            case QMetaType::QVector4D: return "vec4"; break;
            case QMetaType::QTransform: return "mat3"; break;
            case QMetaType::QMatrix4x4: return "mat4"; break;
            default: return "float"; break;
        }

        return QString();
    }

    static Variant fromQVariant(const QVariant &value) {
        switch(value.type()) {
            case QMetaType::Int: {
                return Variant(value.toInt());
            } break;
            case QMetaType::Float: {
                return Variant(value.toFloat());
            } break;
            case QMetaType::QVector2D: {
                QVector2D v = value.value<QVector2D>();
                return Variant(Vector2(v.x(), v.y()));
            } break;
            case QMetaType::QVector3D: {
                QVector3D v = value.value<QVector3D>();
                return Variant(Vector3(v.x(), v.y(), v.z()));
            } break;
            case QMetaType::QVector4D: {
                QVector4D v = value.value<QVector4D>();
                return Variant(Vector4(v.x(), v.y(), v.z(), v.w()));
            } break;
            default: break;
        }

        return Variant();
    }

protected:
    friend class ShaderNodeGraph;

    list<pair<string, uint32_t>> m_inputs;
    list<pair<string, uint32_t>> m_outputs;

    QString m_expression;

    int32_t m_position;
    int32_t m_type;

    static map<uint32_t, Vector4> m_portColors;

};

#endif // FUNCTION_H
