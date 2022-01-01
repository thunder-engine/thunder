#ifndef FUNCTION_H
#define FUNCTION_H

#include "../shaderschememodel.h"

#include <editor/assetconverter.h>

static const char *a("A");
static const char *b("B");
static const char *x("X");
static const char *y("Y");

class ShaderFunction : public QObject {
    Q_OBJECT

public:
    ShaderFunction() :
            m_pModel(nullptr),
            m_pNode(nullptr) {
        reset();
    }

    void reset() {
        m_Position  = -1;
    }

    virtual AbstractSchemeModel::Node *createNode(ShaderSchemeModel *model, const QString &path) {
        m_pNode = new AbstractSchemeModel::Node;
        m_pNode->root = false;
        m_pNode->name = path;
        m_pNode->ptr = this;
        m_pNode->pos = QPoint();

        m_pModel = model;

        return m_pNode;
    }

    virtual int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) {
        Q_UNUSED(value)
        Q_UNUSED(link)
        Q_UNUSED(size)

        if(m_Position == -1) {
            m_Position  = depth;
            depth++;
        }
        return m_Position;
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
    ShaderSchemeModel *m_pModel;
    AbstractSchemeModel::Node *m_pNode;

    int32_t m_Position;

};

#endif // FUNCTION_H
