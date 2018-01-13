#ifndef AMATHFUNCTIONS
#define AMATHFUNCTIONS

#include "../shaderbuilder.h"

#define A       "A"
#define B       "B"
#define MINV    "Min"
#define MAXV    "Max"

class MathFunction : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    AbstractSchemeModel::Node *createNode(ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        int i   = 0;
        foreach(QString it, m_Params) {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = it;
            out->out    = false;
            out->pos    = i;
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

    bool compile(AbstractSchemeModel::Node *object, const QString &func, QString &value, uint32_t &depth, uint8_t &size, uint8_t expect = 0) {
        QString args;

        const AbstractSchemeModel::Link *l;
        int i       = 0;
        foreach(QString it, m_Params) {
            l   = m_pModel->findLink(object, qPrintable(it));
            if(l) {
                ShaderFunction *node    = static_cast<ShaderFunction *>(l->sender->ptr);
                if(node) {
                    uint8_t type;
                    if(!node->build(value, *l, depth, type)) {
                        return false;
                    }
                    if(i == 0 && !expect) {
                        expect  = type;
                    }
                    args    += convert(QString("local%1").arg(depth), type, expect) + ((i == m_Params.size() - 1) ? "" : ", ");
                    depth++;
                }
            } else {
                m_pModel->reportError(this, QString("Missing argument ") + it);
                return false;
            }
            i++;
        }

        if(!size) {
            size    = expect;
        }

        switch (size) {
            case QMetaType::QVector2D:  value.append("\tvec2");  break;
            case QMetaType::QVector3D:  value.append("\tvec3");  break;
            case QMetaType::QVector4D:  value.append("\tvec4");  break;
            default: value.append("\tfloat"); break;
        }
        value.append(QString(" local%1 = %2(%3);\n").arg(depth).arg(func).arg(args));

        return true;
    }
    QStringList     m_Params;
};

class DotProduct : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE DotProduct() { m_Params << A << B; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        size    = QMetaType::Double;
        return compile(m_pNode, "dot", value, depth, size);
    }
};

class CrossProduct : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE CrossProduct() { m_Params << A << B; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        size    = QMetaType::QVector3D;
        return compile(m_pNode, "cross", value, depth, size, size);
    }
};

class Clamp : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Clamp() { m_Params << A << MINV << MAXV; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "clamp", value, depth, size);
    }
};

class Mod : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Mod() { m_Params << A << B; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "mod", value, depth, size);
    }
};

class Abs : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Abs() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "abs", value, depth, size);
    }
};

class Floor : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Floor() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "floor", value, depth, size);
    }
};

class Ceil : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Ceil() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "ceil", value, depth, size);
    }
};

class Sine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Sine() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "sin", value, depth, size);
    }
};

class Cosine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Cosine() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "cos", value, depth, size);
    }
};

class Tangent : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Tangent() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "tan", value, depth, size);
    }
};

class ArcSine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcSine() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "asin", value, depth, size);
    }
};

class ArcCosine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcCosine() { m_Params << A; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "acos", value, depth, size);
    }
};

class ArcTangent : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcTangent() {m_Params << A << B; }

    bool build(QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        return compile(m_pNode, "atan", value, depth, size);
    }
};

#endif // AMATHFUNCTIONS
