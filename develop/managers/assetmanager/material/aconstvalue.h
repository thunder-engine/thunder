#ifndef ACONSTVALUE_H
#define ACONSTVALUE_H

#include "../shaderbuilder.h"

#include <QColor>
#include <QVector2D>

class ConstFloat : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(double Value READ value WRITE setValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstFloat  () { m_Value    = 0.0; }

    AbstractSchemeModel::Node *createNode (ShaderBuilder *model, const QString &path) override {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
        out->name   = "";
        out->out    = true;
        out->pos    = 0;
        out->type   = QMetaType::Double;
        result->list.push_back(out);

        return result;
    }

    double value () const {
        return m_Value;
    }

    void setValue (double value) {
        m_Value = value;
        emit updated();
    }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        if(m_Position == -1) {
            size    = QMetaType::Double;
            value  += QString("\tfloat local%1 = %2;\n").arg(depth).arg(m_Value);
        }
        return ShaderFunction::build(value, link, depth, size);
    }
protected:
    double      m_Value;
};

class ConstVector2 : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(double R READ valueR WRITE setValueR DESIGNABLE true USER true)
    Q_PROPERTY(double G READ valueG WRITE setValueG DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector2() { m_Value = QVector2D(); }

    virtual AbstractSchemeModel::Node *createNode (ShaderBuilder *model, const QString &path) override {
        AbstractSchemeModel::Node *result = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Item *out = new AbstractSchemeModel::Item;
        out->name = "";
        out->out  = true;
        out->pos  = 0;
        out->type = QMetaType::QVector2D;
        result->list.push_back(out);

        return result;
    }

    double valueR () const {
        return m_Value.x();
    }

    void setValueR (const double value) {
        m_Value.setX(value);
        emit updated();
    }

    double valueG () const {
        return m_Value.y();
    }

    void setValueG (const double value) {
        m_Value.setY(value);
        emit updated();
    }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        if(m_Position == -1) {
            size   = QMetaType::QVector2D;
            value += QString("\tvec2 local%1 = vec2(%2, %3);\n").arg(depth).arg(m_Value.x()).arg(m_Value.y());
        }
        return ShaderFunction::build(value, link, depth, size);
    }
protected:
    QVector2D   m_Value;
};

class ConstVector3 : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(QColor Value READ value WRITE setValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector3() { m_Value    = QColor(0, 0, 0); }

    virtual AbstractSchemeModel::Node *createNode (ShaderBuilder *model, const QString &path) override {
        AbstractSchemeModel::Node *result   = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Item *out      = new AbstractSchemeModel::Item;
        out->name   = "";
        out->out    = true;
        out->pos    = 0;
        out->type   = QMetaType::QVector3D;
        result->list.push_back(out);

        return result;
    }

    QColor value () const {
        return m_Value;
    }

    void setValue (const QColor &value) {
        m_Value = value.rgb();
        emit updated();
    }

    int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        if(m_Position == -1) {
            size   = QMetaType::QVector3D;
            value += QString("\tvec3 local%1 = vec3(%2, %3, %4);\n").arg(depth).arg(m_Value.redF()).arg(m_Value.greenF()).arg(m_Value.blueF());
        }
        return ShaderFunction::build(value, link, depth, size);
    }
protected:
    QColor m_Value;
};

class ConstVector4 : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Constant")

    Q_PROPERTY(QColor Value READ value WRITE setValue DESIGNABLE true USER true)

public:
    Q_INVOKABLE ConstVector4() { m_Value = QColor(0, 0, 0, 0); }

    virtual AbstractSchemeModel::Node *createNode (ShaderBuilder *model, const QString &path) override {
        AbstractSchemeModel::Node *result = ShaderFunction::createNode(model, path);
        AbstractSchemeModel::Item *out = new AbstractSchemeModel::Item;
        out->name   = "";
        out->out    = true;
        out->pos    = 0;
        out->type   = QMetaType::QVector4D;
        result->list.push_back(out);

        return result;
    }

    QColor value () const {
        return m_Value;
    }

    void setValue (const QColor &value) {
        m_Value = value;
        emit updated();
    }

    int32_t build (QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) override {
        if(m_Position == -1) {
            size   = QMetaType::QVector4D;
            value += QString("\tvec4 local%1 = vec4(%2, %3, %4, %5);\n").arg(depth).arg(m_Value.redF()).arg(m_Value.greenF()).arg(m_Value.blueF()).arg(m_Value.alphaF());
        }
        return ShaderFunction::build(value, link, depth, size);
    }
protected:
    QColor m_Value;
};

#endif // ACONSTVALUE_H
