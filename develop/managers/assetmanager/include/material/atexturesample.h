#ifndef TEXTURESAMPLE_H
#define TEXTURESAMPLE_H

#include "../shaderbuilder.h"

#include "assetmanager.h"

#define UV      "UV"
#define R       "R"
#define G       "G"
#define B       "B"
#define A       "A"

class TextureSample : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

    Q_PROPERTY(Template Texture READ texture WRITE setTexture DESIGNABLE true USER true)
    //Q_PROPERTY(QString Element READ element WRITE setElement DESIGNABLE true USER true)

public:
    Q_INVOKABLE TextureSample() {
        m_Path  = Template("", IConverter::ContentTexture);
        m_Element.clear();
    }

    virtual AbstractSchemeModel::Node  *createNode  (ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        {
            AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
            out->name   = UV;
            out->out    = false;
            out->pos    = 0;
            out->type   = QMetaType::QVector2D;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
            out->name   = "";
            out->out    = true;
            out->pos    = 0;
            out->type   = QMetaType::QVector4D;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
            out->name   = R;
            out->out    = true;
            out->pos    = 1;
            out->type   = QMetaType::Double;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
            out->name   = G;
            out->out    = true;
            out->pos    = 2;
            out->type   = QMetaType::Double;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
            out->name   = B;
            out->out    = true;
            out->pos    = 3;
            out->type   = QMetaType::Double;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
            out->name   = A;
            out->out    = true;
            out->pos    = 4;
            out->type   = QMetaType::Double;
            result->list.push_back(out);
        }

        return result;
    }

    bool build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        Vector4 sub;
        int result  = m_pModel->setTexture(m_Path.path, sub, false);

        if(result > -1) {
            QString uv  = "p.uv";
            const AbstractSchemeModel::Link *l  = m_pModel->findLink(m_pNode, UV);
            if(l) {
                ShaderFunction *node    = static_cast<ShaderFunction *>(l->sender->ptr);
                if(node) {
                    uint8_t type;
                    if(node->build(value, *l, depth, type)) {
                        uv  = convert("local" + QString::number(depth), type, QMetaType::QVector2D);
                    }

                    depth++;
                }
            }

            uv      = QString("vec2(%1, %2) + %3 * vec2(%4, %5)").arg(sub.x).arg(sub.y).arg(uv).arg(sub.z).arg(sub.w);
            value  += QString("\tvec4 lt%1 = texture(texture%2, %3);\n").arg(depth).arg(result).arg(uv);

            if(link.sitem->name == "") {
                size    = QMetaType::QVector4D;
                value  += QString("\tvec4 local%1 = lt%1;\n").arg(depth);
            } else {
                size    = QMetaType::Double;

                QString channel = "x";
                if(link.sitem->name == G) {
                    channel = "y";
                } else if(link.sitem->name == B) {
                    channel = "z";
                } else if(link.sitem->name == A) {
                    channel = "w";
                }
                value  += QString("\tfloat local%1 = lt%1.%2;\n").arg(depth).arg(channel);
            }
        } else {
            m_pModel->reportError(this, "Missing texture");
            return false;
        }
        return true;
    }

    Template    texture     () const {
        return m_Path;
    }

    void        setTexture  (const Template &path) {
        m_Path.path = path.path;
        emit updated();
    }

protected:
    Template    m_Path;
    QString     m_Element;

};

class TextureSampleCube : public TextureSample {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

public:
    Q_INVOKABLE TextureSampleCube() {}

    bool build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        Vector4 sub;
        int result  = m_pModel->setTexture(m_Path.path, sub, true);

        if(result > -1) {
            QString uv  = "p.uv";
            const AbstractSchemeModel::Link *l  = m_pModel->findLink(m_pNode, UV);
            if(l) {
                TextureSample *node = static_cast<TextureSample *>(l->sender->ptr);
                if(node) {
                    uint8_t type;
                    node->build(value, *l, depth, type);
                    uv  = convert("local" + QString::number(depth), type, QMetaType::QVector3D);

                    depth++;
                }
            }

            uv      = "vec3(" + uv + ", 1.0)";
            value  += QString("\tvec4 lt%1 = texture(texture%2, %3);\n").arg(depth).arg(result).arg(uv);
            if(link.sitem->name == "") {
                size    = AMetaType::VECTOR4;
                value  += QString("\tvec4 local%1 = lt%1;\n").arg(depth);
            } else {
                size    = AMetaType::DOUBLE;
                QString channel = "x";
                if(link.sitem->name == G) {
                    channel = "y";
                } else if(link.sitem->name == B) {
                    channel = "z";
                } else if(link.sitem->name == A) {
                    channel = "w";
                }
                value  += QString("\tfloat local%1 = lt%1.%2;\n").arg(depth).arg(channel);
            }
        } else {
            m_pModel->reportError(this, "Missing texture");
            return false;
        }
        return true;
    }

};

#endif // TEXTURESAMPLE_H
