#ifndef TEXTURESAMPLE_H
#define TEXTURESAMPLE_H

#include "function.h"

#define UV      "UV"

class TextureFunction : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

public:
    Q_INVOKABLE TextureFunction() { }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            type = QMetaType::QVector4D;

            QString expr = makeExpression(getArguments(code, stack, depth, type));

            code.append(localValue(type, depth, expr));
        }

        int32_t result = ShaderNode::build(code, stack, link, depth, type);

        int channel = -1;
        if(link.oport->m_name == r) {
            channel = 0;
        } else if(link.oport->m_name == g) {
            channel = 1;
        } else if(link.oport->m_name == b) {
            channel = 2;
        } else if(link.oport->m_name == a) {
            channel = 3;
        }
        stack.push(convert("local" + QString::number(result),
                           QMetaType::QVector4D,
                           (channel > -1) ? QMetaType::Float : QMetaType::QVector4D,
                           channel));

        type = QMetaType::QVector4D;
        if(channel > -1) {
            type = QMetaType::Float;
        }

        return result;
    }

    QString defaultValue(const string &, uint32_t &) const override {
        return "_uv0";
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("texture(%2, %3)").arg(m_name, args[0]);
    }

protected:
    QString m_name;

    Vector4 m_sub;

};

class TextureObject : public TextureFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

    Q_PROPERTY(Template Texture READ texture WRITE setTexture NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE TextureObject() {
        m_outputs.push_back(make_pair("Texture", QMetaType::QImage));

        m_path = Template("", MetaType::type<Texture *>());
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderNodeGraph *>(m_graph)->addTexture(m_path.path, m_sub, false);
        if(result < 0) {
            m_graph->reportMessage(this, "Missing texture");
            return -1;
        }

        stack.push(QString("texture%1").arg(result));
        return ShaderNode::build(code, stack, link, depth, type);
    }

    Template texture() const {
        return m_path;
    }

    void setTexture(const Template &path) {
        m_path.path = path.path;
        emit updated();
    }

protected:
    Template m_path;

};

class TextureSample : public TextureFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

    Q_PROPERTY(Template Texture READ texture WRITE setTexture NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE TextureSample() {
        m_inputs.push_back(make_pair(UV, QMetaType::QVector2D));

        m_outputs.push_back(make_pair("RGBA", QMetaType::QVector4D));
        m_outputs.push_back(make_pair(r, QMetaType::Float));
        m_outputs.push_back(make_pair(g, QMetaType::Float));
        m_outputs.push_back(make_pair(b, QMetaType::Float));
        m_outputs.push_back(make_pair(a, QMetaType::Float));

        m_path = Template("", MetaType::type<Texture *>());
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderNodeGraph *>(m_graph)->addTexture(m_path.path, m_sub, false);
        if(result < 0) {
            m_graph->reportMessage(this, "Missing texture");
            return -1;
        }

        m_name = QString("texture%1").arg(result);
        return TextureFunction::build(code, stack, link, depth, type);
    }

    Template texture() const {
        return m_path;
    }

    void setTexture(const Template &path) {
        m_path.path = path.path;
        emit updated();
    }

protected:
    Template m_path;

};

class RenderTargetSample : public TextureFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

    Q_PROPERTY(QString Target_Name READ targetName WRITE setTargetName NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE RenderTargetSample() {
        m_inputs.push_back(make_pair(UV, QMetaType::QVector2D));

        m_outputs.push_back(make_pair("RGBA", QMetaType::QVector4D));
        m_outputs.push_back(make_pair(r, QMetaType::Float));
        m_outputs.push_back(make_pair(g, QMetaType::Float));
        m_outputs.push_back(make_pair(b, QMetaType::Float));
        m_outputs.push_back(make_pair(a, QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        static_cast<ShaderNodeGraph *>(m_graph)->addTexture(m_name, m_sub, ShaderRootNode::Target);

        return TextureFunction::build(code, stack, link, depth, type);
    }

    QString targetName() const {
        return m_name;
    }

    void setTargetName(const QString &name) {
        m_name = name;
        emit updated();
    }

};

class TextureSampleCube : public TextureSample {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

public:
    Q_INVOKABLE TextureSampleCube() {
        m_inputs.push_back(make_pair(UV, QMetaType::QVector3D));

        m_outputs.push_back(make_pair("RGBA", QMetaType::QVector4D));
        m_outputs.push_back(make_pair(r, QMetaType::Float));
        m_outputs.push_back(make_pair(g, QMetaType::Float));
        m_outputs.push_back(make_pair(b, QMetaType::Float));
        m_outputs.push_back(make_pair(a, QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderNodeGraph *>(m_graph)->addTexture(m_path.path, m_sub, ShaderRootNode::Cube);
        if(result < 0) {
            m_graph->reportMessage(this, "Missing texture");
            return -1;
        }

        m_name = QString("texture%1").arg(result);
        return TextureFunction::build(code, stack, link, depth, type);
    }

};

#endif // TEXTURESAMPLE_H
