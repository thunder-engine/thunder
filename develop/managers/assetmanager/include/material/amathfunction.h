#ifndef AMATHFUNCTIONS
#define AMATHFUNCTIONS

#include "../shaderbuilder.h"

#define MINV    "Min"
#define MAXV    "Max"

class MathFunction : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Math")

public:
    AbstractSchemeModel::Node *createNode (ShaderBuilder *model, const QString &path) override {
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

    int32_t compile(AbstractSchemeModel::Node *object, const QString &func, QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size, uint8_t expect = 0, uint8_t last = 0) {
        if(m_Position == -1) {
            QString args;

            const AbstractSchemeModel::Link *l;
            int i = 0;
            foreach(QString it, m_Params) {
                l   = m_pModel->findLink(object, qPrintable(it));
                if(l) {
                    ShaderFunction *node = static_cast<ShaderFunction *>(l->sender->ptr);
                    if(node) {
                        uint8_t type;
                        int32_t index = node->build(value, *l, depth, type);
                        if(index >= 0) {
                            if(i == 0 && !expect) {
                                expect = type;
                            }

                            uint8_t final = expect;
                            if(i == (m_Params.size() - 1) && last) {
                                final = last;
                            }

                            args += convert(QString("local%1").arg(index), type, final) + ((i == m_Params.size() - 1) ? "" : ", ");
                        } else {
                            return -1;
                        }
                    }
                } else {
                    m_pModel->reportError(this, QString("Missing argument ") + it);
                    return -1;
                }
                i++;
            }

            if(!size) {
                size = expect;
            }

            switch (size) {
                case QMetaType::QVector2D:  value.append("\tvec2");  break;
                case QMetaType::QVector3D:  value.append("\tvec3");  break;
                case QMetaType::QVector4D:  value.append("\tvec4");  break;
                default: value.append("\tfloat"); break;
            }
            value.append(QString(" local%1 = %2(%3);\n").arg(depth).arg(func).arg(args));
        }
        return ShaderFunction::build(value, link, depth, size);
    }
    QStringList m_Params;
};

class DotProduct : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE DotProduct() { m_Params << a << b; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        size = QMetaType::Double;
        return compile(m_pNode, "dot", value, link, depth, size);
    }
};

class CrossProduct : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE CrossProduct() { m_Params << a << b; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        size = QMetaType::QVector3D;
        return compile(m_pNode, "cross", value, link, depth, size, size);
    }
};
class Smoothstep : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Smoothstep() { m_Params << x << y << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "smoothstep", value, link, depth, size, 0, 1);
    }
};

class Mix : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Mix() { m_Params << x << y << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "mix", value, link, depth, size, 0, 1);
    }
};

class Clamp : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Clamp() { m_Params << a << MINV << MAXV; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "clamp", value, link, depth, size);
    }
};

class Min : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Min() { m_Params << x << y; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "min", value, link, depth, size);
    }
};

class Max : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Max() { m_Params << x << y; }

    int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) {
        return compile(m_pNode, "max", value, link, depth, size);
    }
};

class Mod : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Mod() { m_Params << x << y; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "mod", value, link, depth, size);
    }
};

class Power : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Power() { m_Params << "Base" << "Exp"; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "pow", value, link, depth, size);
    }
};

class SquareRoot : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE SquareRoot() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "sqrt", value, link, depth, size);
    }
};

class Logarithm : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Logarithm() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "log", value, link, depth, size);
    }
};

class Logarithm2 : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Logarithm2() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "log2", value, link, depth, size);
    }
};

class FWidth : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE FWidth() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "fwidth", value, link, depth, size);
    }
};


class Abs : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Abs() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "abs", value, link, depth, size);
    }
};

class Sign : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Sign() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "sign", value, link, depth, size);
    }
};

class Floor : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Floor() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "floor", value, link, depth, size);
    }
};

class Ceil : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Ceil() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "ceil", value, link, depth, size);
    }
};

class Round : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Round() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "round", value, link, depth, size);
    }
};

class Truncate : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Truncate() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "trunc", value, link, depth, size);
    }
};

class Fract : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Fract() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "fract", value, link, depth, size);
    }
};

class Normalize : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Normalize() { m_Params << x; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "normalize", value, link, depth, size);
    }
};

class Sine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Sine() { m_Params << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "sin", value, link, depth, size);
    }
};

class Cosine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Cosine() { m_Params << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "cos", value, link, depth, size);
    }
};

class Tangent : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE Tangent() { m_Params << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "tan", value, link, depth, size);
    }
};

class ArcSine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcSine() { m_Params << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "asin", value, link, depth, size);
    }
};

class ArcCosine : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcCosine() { m_Params << a; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "acos", value, link, depth, size);
    }
};

class ArcTangent : public MathFunction {
    Q_OBJECT

public:
    Q_INVOKABLE ArcTangent() {m_Params << a << b; }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        return compile(m_pNode, "atan", value, link, depth, size);
    }
};

#endif // AMATHFUNCTIONS
