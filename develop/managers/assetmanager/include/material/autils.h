#ifndef AUTILS
#define AUTILS

#include "../shaderbuilder.h"

#define IN  "In"

#define AGB "A>B"
#define BGA "A<B"
#define AEB "A=B"

class Mask : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

    Q_PROPERTY(bool R READ r WRITE setR NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool G READ g WRITE setG NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool B READ b WRITE setB NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool A READ a WRITE setA NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Mask() :
        m_R(true),
        m_G(true),
        m_B(true),
        m_A(true) {

    }

    AbstractSchemeModel::Node *createNode(ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        int i   = 0;
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = IN;
            out->out    = false;
            out->pos    = 0;
            out->type   = QMetaType::Void;
            result->list.push_back(out);
            i++;
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = "";
            out->out    = true;
            out->pos    = 0;
            out->type   = QMetaType::Void;
            result->list.push_back(out);
        }
        return result;
    }

    int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) {
        size    = 0;

        const AbstractSchemeModel::Link *l = m_pModel->findLink(m_pNode, IN);
        if(l) {
            ShaderFunction *node = static_cast<ShaderFunction *>(l->sender->ptr);

            uint8_t type;
            int32_t index = node->build(value, *l, depth, type);
            if(index >= 0) {
                QString mask;
                if(m_R && type > 0) {
                    mask += "r";
                    size++;
                }
                if(m_G && type > 1) {
                    mask += "g";
                    size++;
                }
                if(m_B && type > 2) {
                    mask += "b";
                    size++;
                }
                if(m_A && type > 3) {
                    mask += "a";
                    size++;
                }

                switch (size) {
                    case QMetaType::QVector2D:  value.append("\tvec2");  break;
                    case QMetaType::QVector3D:  value.append("\tvec3");  break;
                    case QMetaType::QVector4D:  value.append("\tvec4");  break;
                    default: value.append("\tfloat"); break;
                }
                value.append(QString(" local%1 = local%2.%3;\n").arg(depth).arg(index).arg(mask));
            } else {
                return -1;
            }
        } else {
            m_pModel->reportError(this, "Missing argument");
            return -1;
        }
        return ShaderFunction::build(value, link, depth, size);
    }

    bool r() const { return m_R; }
    bool g() const { return m_G; }
    bool b() const { return m_B; }
    bool a() const { return m_A; }

    void setR(bool value) { m_R = value; emit updated();}
    void setG(bool value) { m_G = value; emit updated();}
    void setB(bool value) { m_B = value; emit updated();}
    void setA(bool value) { m_A = value; emit updated();}

    bool m_R;
    bool m_G;
    bool m_B;
    bool m_A;
};

class If : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    Q_INVOKABLE If() {

    }

    AbstractSchemeModel::Node *createNode(ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        int i   = 0;
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = a;
            out->out    = false;
            out->pos    = 0;
            out->type   = QMetaType::Float;
            result->list.push_back(out);
            i++;
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = b;
            out->out    = false;
            out->pos    = 1;
            out->type   = QMetaType::Float;
            result->list.push_back(out);
            i++;
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = AGB;
            out->out    = false;
            out->pos    = 2;
            out->type   = QMetaType::Void;
            result->list.push_back(out);
            i++;
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = AEB;
            out->out    = false;
            out->pos    = 3;
            out->type   = QMetaType::Void;
            result->list.push_back(out);
            i++;
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = BGA;
            out->out    = false;
            out->pos    = 4;
            out->type   = QMetaType::Void;
            result->list.push_back(out);
            i++;
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = "";
            out->out    = true;
            out->pos    = 0;
            out->type   = QMetaType::Void;
            result->list.push_back(out);
        }
        return result;
    }

    int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) {
        size    = 0;

        const AbstractSchemeModel::Link *al  = m_pModel->findLink(m_pNode, a);
        const AbstractSchemeModel::Link *bl  = m_pModel->findLink(m_pNode, b);
        const AbstractSchemeModel::Link *agbl= m_pModel->findLink(m_pNode, AGB);
        const AbstractSchemeModel::Link *bgal= m_pModel->findLink(m_pNode, BGA);

        if(al && bl && agbl && bgal) {
            uint8_t type;
            ShaderFunction *aNode = static_cast<ShaderFunction *>(al->sender->ptr);
            uint32_t aIndex = aNode->build(value, *al, depth, type);

            ShaderFunction *bNode = static_cast<ShaderFunction *>(bl->sender->ptr);
            uint32_t bIndex = bNode->build(value, *bl, depth, type);

            ShaderFunction *agbNode = static_cast<ShaderFunction *>(agbl->sender->ptr);
            uint32_t agbIndex = agbNode->build(value, *agbl, depth, size);

            ShaderFunction *bgaNode = static_cast<ShaderFunction *>(bgal->sender->ptr);
            uint32_t bgaIndex = bgaNode->build(value, *bgal, depth, type);

            QString equal;
            const AbstractSchemeModel::Link *aebl= m_pModel->findLink(m_pNode, AEB);
            if(aebl) {
                ShaderFunction *aebNode = static_cast<ShaderFunction *>(aebl->sender->ptr);
                uint32_t aebIndex = aebNode->build(value, *aebl, depth, type);


                equal = QString("((local%1 < local%2) ? local%3 : local%4)").arg(aIndex).arg(bIndex).arg(bgaIndex).arg(aebIndex);
            } else {
                equal = QString("local%1").arg(bgaIndex);
            }

            QString args = QString("(local%1 > local%2) ? local%3 : %4").arg(aIndex).arg(bIndex).arg(agbIndex).arg(equal);

            switch(size) {
                case QMetaType::QVector2D:  value.append("\tvec2"); break;
                case QMetaType::QVector3D:  value.append("\tvec3"); break;
                case QMetaType::QVector4D:  value.append("\tvec4"); break;
                default: value.append("\tfloat"); break;
            }
            value.append(QString(" local%1 = %2;\n").arg(depth).arg(args));

        } else {
            m_pModel->reportError(this, "Missing argument");
            return -1;
        }

        return ShaderFunction::build(value, link, depth, size);
    }
};
#endif // AUTILS

