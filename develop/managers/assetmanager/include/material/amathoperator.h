#ifndef AMATHOPERATOR
#define AMATHOPERATOR

#include "../shaderbuilder.h"

#define A       "A"
#define B       "B"
#define OUT     "Out"

class MathOperation : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Operations")

public:
    AbstractSchemeModel::Node *createNode(ShaderBuilder *model, const QString &path) {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = A;
            out->out    = false;
            out->pos    = 0;
            out->type   = QMetaType::QVector2D;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = B;
            out->out    = false;
            out->pos    = 1;
            out->type   = QMetaType::QVector2D;
            result->list.push_back(out);
        }
        {
            AbstractSchemeModel::Item *out  = new AbstractSchemeModel::Item;
            out->name   = "";
            out->out    = true;
            out->pos    = 0;
            out->type   = QMetaType::QVector2D;
            result->list.push_back(out);
        }
        return result;
    }

    void compile(AbstractSchemeModel::Node *object, const QString &operation, QString &value, const AbstractSchemeModel::Link &, uint32_t &depth, uint8_t &size) {
        QString args;

        vector<char *> names    = {(char *)A, (char *)B};

        const AbstractSchemeModel::Link *l;

        for(uint8_t i = 0; i < 2; i++) {
            l   = m_pModel->findLink(object, names[i]);
            if(l) {
                ShaderFunction *node   = static_cast<ShaderFunction *>(l->sender->ptr);
                if(node) {
                    uint8_t type;
                    node->build(value, *l, depth, type);
                    if(i == 0) {
                        size    = type;
                    }
                    args    += ((i == 0) ? "" : operation) + convert("local" + QString::number(depth), type, size);
                    depth++;
                }
            }
        }

        switch (size) {
            case QMetaType::QVector2D:  value.append("\tvec2"); break;
            case QMetaType::QVector3D:  value.append("\tvec3"); break;
            case QMetaType::QVector4D:  value.append("\tvec4"); break;
            default: value.append("\tfloat"); break;
        }
        value.append(QString(" local%1 = %2;\n").arg(depth).arg(args));
    }
};

class Subtraction : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Subtraction() {}

    bool build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        compile(m_pNode, " - ", value, link, depth, size);
        return true;
    }
};

class Add : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Add() {}

    bool build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        compile(m_pNode, " + ", value, link, depth, size);
        return true;
    }
};

class Divide : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Divide() {}

    bool build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        compile(m_pNode, " / ", value, link, depth, size);
        return true;
    }
};

class Multiply : public MathOperation {
    Q_OBJECT

public:
    Q_INVOKABLE Multiply() {}

    bool build(QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        compile(m_pNode, " * ", value, link, depth, size);
        return true;
    }
};

#endif // AMATHOPERATOR

