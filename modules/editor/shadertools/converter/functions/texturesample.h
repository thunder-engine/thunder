#ifndef TEXTURESAMPLE_H
#define TEXTURESAMPLE_H

#include "function.h"

#define UV      "UV"

class TextureFunction : public ShaderNode {
    A_OBJECT(TextureFunction, ShaderNode, Shader/Texture)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureFunction() { }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            type = MetaType::VECTOR4;

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
                           MetaType::VECTOR4,
                           (channel > -1) ? MetaType::FLOAT : MetaType::VECTOR4,
                           channel));

        type = MetaType::VECTOR4;
        if(channel > -1) {
            type = MetaType::FLOAT;
        }

        return result;
    }

    QString defaultValue(const TString &, uint32_t &) const override {
        return "_uv0";
    }

    QString makeExpression(const QStringList &args) const override {
        return QString("texture(%2, %3)").arg(m_name.c_str(), args[0]);
    }

protected:
    std::string m_name;

    Vector4 m_sub;

};

class TextureObject : public TextureFunction {
    A_OBJECT(TextureObject, TextureFunction, Shader/Texture)

    A_PROPERTIES(
        A_PROPERTYEX(Texture *, Texture, TextureObject::texture, TextureObject::setTexture, "editor=Asset")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureObject() :
            m_texture(nullptr) {

        m_outputs.push_back(std::make_pair("Texture", MetaType::STRING));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderGraph *>(m_graph)->addTexture(Engine::reference(m_texture), m_sub, false);
        if(result < 0) {
            m_graph->reportMessage(this, "Missing texture");
            return -1;
        }

        stack.push(QString("texture%1").arg(result));
        return ShaderNode::build(code, stack, link, depth, type);
    }

    Texture *texture() const {
        return m_texture;
    }

    void setTexture(Texture *texture) {
        m_texture = texture;

    }

protected:
    Texture *m_texture;

};

class TextureSample : public TextureFunction {
    A_OBJECT(TextureSample, TextureFunction, Shader/Texture)

    A_PROPERTIES(
        A_PROPERTYEX(Texture *, Texture, TextureSample::texture, TextureSample::setTexture, "editor=Asset")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureSample() :
            m_texture(nullptr) {
        m_inputs.push_back(std::make_pair(UV, MetaType::VECTOR2));

        m_outputs.push_back(std::make_pair("RGBA", MetaType::VECTOR4));
        m_outputs.push_back(std::make_pair(r, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(g, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(b, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(a, MetaType::FLOAT));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderGraph *>(m_graph)->addTexture(Engine::reference(m_texture), m_sub, false);
        if(result < 0) {
            m_graph->reportMessage(this, "Missing texture");
            return -1;
        }

        m_name = QString("texture%1").arg(result).toStdString();
        return TextureFunction::build(code, stack, link, depth, type);
    }

    Texture *texture() const {
        return m_texture;
    }

    void setTexture(Texture *texture) {
        m_texture = texture;
    }

protected:
    Texture *m_texture;


};

class RenderTargetSample : public TextureFunction {
    A_OBJECT(RenderTargetSample, TextureFunction, Shader/Texture)

    A_PROPERTIES(
        A_PROPERTY(TString, Target_Name, RenderTargetSample::targetName, RenderTargetSample::setTargetName)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetSample() {
        m_inputs.push_back(std::make_pair(UV, MetaType::VECTOR2));

        m_outputs.push_back(std::make_pair("RGBA", MetaType::VECTOR4));
        m_outputs.push_back(std::make_pair(r, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(g, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(b, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(a, MetaType::FLOAT));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        static_cast<ShaderGraph *>(m_graph)->addTexture(m_name, m_sub, ShaderRootNode::Target);

        return TextureFunction::build(code, stack, link, depth, type);
    }

    std::string targetName() const {
        return m_name;
    }

    void setTargetName(const std::string &name) {
        m_name = name;

    }

};

class TextureSampleCube : public TextureSample {
    A_OBJECT(TextureSampleCube, TextureFunction, Shader/Texture)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureSampleCube() {
        m_inputs.push_back(std::make_pair(UV, MetaType::VECTOR3));

        m_outputs.push_back(std::make_pair("RGBA", MetaType::VECTOR4));
        m_outputs.push_back(std::make_pair(r, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(g, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(b, MetaType::FLOAT));
        m_outputs.push_back(std::make_pair(a, MetaType::FLOAT));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderGraph *>(m_graph)->addTexture(Engine::reference(m_texture), m_sub, ShaderRootNode::Cube);
        if(result < 0) {
            m_graph->reportMessage(this, "Missing texture");
            return -1;
        }

        m_name = QString("texture%1").arg(result).toStdString();
        return TextureFunction::build(code, stack, link, depth, type);
    }

};

#endif // TEXTURESAMPLE_H
