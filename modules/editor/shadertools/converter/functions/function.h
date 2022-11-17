#ifndef FUNCTION_H
#define FUNCTION_H

#include "../shadernodegraph.h"

#include <editor/assetconverter.h>
#include <editor/graph/graphnode.h>

#include <QStack>

static const char *a("A");
static const char *b("B");
static const char *r("R");
static const char *g("G");
static const char *x("X");
static const char *y("Y");

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

    virtual int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) {
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
            case QMetaType::QVector2D: {
                switch(current) {
                    case QMetaType::Float:      { prefix = "vec2("; suffix = ")"; } break;
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xy"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector3D: {
                switch(current) {
                    case QMetaType::Float:      { prefix = "vec3("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec3("; suffix = ", 0.0)"; } break;
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xyz"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector4D: {
                switch(current) {
                    case QMetaType::Float:      { prefix = "vec4("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec4("; suffix = ", 0.0, 1.0)"; } break;
                    case QMetaType::QVector3D:  { prefix = "vec4("; suffix = ", 1.0)"; } break;
                    default: break;
                }
            } break;
            default: {
                switch(current) {
                    case QMetaType::QVector2D:
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = names[component]; } break;
                    default: break;
                }
            } break;
        }
        return(prefix + value + suffix);
    }

signals:
    void updated();

protected:
    friend class ShaderNodeGraph;

    int32_t m_position;
    int32_t m_type;

};

static map<uint32_t, Vector4> m_portColors {
    { QMetaType::Void, Vector4(0.6f, 0.6f, 0.6f, 1.0f) },
    { QMetaType::Int, Vector4(0.22f, 0.46, 0.11f, 1.0f) },
    { QMetaType::Float, Vector4(0.16f, 0.52f, 0.80f, 1.0f) },
    { QMetaType::QVector2D, Vector4(0.95f, 0.26f, 0.21f, 1.0f) },
    { QMetaType::QVector3D, Vector4(0.41f, 0.19f, 0.62f, 1.0f) },
    { QMetaType::QVector4D, Vector4(0.94f, 0.76f, 0.20f, 1.0f) },
};

#endif // FUNCTION_H
