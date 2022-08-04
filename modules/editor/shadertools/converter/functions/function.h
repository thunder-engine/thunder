#ifndef FUNCTION_H
#define FUNCTION_H

#include "../shadernodegraph.h"

#include <editor/assetconverter.h>
#include <editor/scheme/graphnode.h>

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
    }

    int32_t position() const {
        return m_position;
    }

    virtual int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) {
        Q_UNUSED(code)
        Q_UNUSED(link)

        if(!size) {
            size = link.oport->m_type;
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
                    case QMetaType::Double:     { prefix = "vec2("; suffix = ")"; } break;
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xy"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector3D: {
                switch(current) {
                    case QMetaType::Double:     { prefix = "vec3("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec3("; suffix = ", 0.0)"; } break;
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xyz"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector4D: {
                switch(current) {
                    case QMetaType::Double:     { prefix = "vec4("; suffix = ")"; } break;
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
    int32_t m_position;

};

#endif // FUNCTION_H
