#ifndef TEXTURESAMPLE_H
#define TEXTURESAMPLE_H

#include "function.h"

#define UV      "UV"

class TextureFunction : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

public:
    Q_INVOKABLE TextureFunction() {
        m_ports.push_back(NodePort(this, false, QMetaType::QVector2D, 5, UV, m_portColors[QMetaType::QVector2D]));
        m_ports.push_back(NodePort(this, true,  QMetaType::QVector4D, 0, "Output", m_portColors[QMetaType::QVector4D]));
        m_ports.push_back(NodePort(this, true,  QMetaType::Float, 1, r, m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true,  QMetaType::Float, 2, g, m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true,  QMetaType::Float, 3, b, m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, true,  QMetaType::Float, 4, a, m_portColors[QMetaType::Float]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QString uv = "_uv0";
            const AbstractNodeGraph::Link *l = m_graph->findLink(this, port(5)); // UV
            if(l) {
                ShaderNode *node = static_cast<ShaderNode *>(l->sender);
                if(node) {
                    int32_t type = 0;
                    int32_t index = node->build(code, stack, *l, depth, type);
                    if(index >= 0) {
                        if(stack.isEmpty()) {
                            uv = convert("local" + QString::number(index), type, QMetaType::QVector2D);
                        } else {
                            uv = convert(stack.pop(), type, QMetaType::QVector2D);
                        }
                    }
                }
            }
            uv = QString("vec2(%1, %2) + %3 * vec2(%4, %5)").arg(m_sub.x).arg(m_sub.y).arg(uv).arg(m_sub.z).arg(m_sub.w);
            code += QString("\tvec4 lt%1 = texture(%2, %3);\n").arg(QString::number(depth), m_name, uv);
        }

        int32_t result = ShaderNode::build(code, stack, link, depth, type);

        QString channel = "lt" + QString::number(result);
        if(link.oport->m_name == r) {
            channel.append(".x");
        } else if(link.oport->m_name == g) {
            channel.append(".y");
        } else if(link.oport->m_name == b) {
            channel.append(".z");
        } else if(link.oport->m_name == a) {
            channel.append(".w");
        }
        stack.push(channel);

        return result;
    }

protected:
    QString m_name;

    Vector4 m_sub;

};

class TextureSample : public TextureFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

    Q_PROPERTY(Template Texture READ texture WRITE setTexture NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE TextureSample() {
        m_path = Template("", MetaType::type<Texture *>());
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        int result = static_cast<ShaderNodeGraph *>(m_graph)->setTexture(m_path.path, m_sub, false);

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
    Q_INVOKABLE RenderTargetSample() { }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        static_cast<ShaderNodeGraph *>(m_graph)->setTexture(m_name, m_sub, ShaderRootNode::Target);

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
    Q_INVOKABLE TextureSampleCube() {}

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            Vector4 sub;
            int result = static_cast<ShaderNodeGraph *>(m_graph)->setTexture(m_path.path, sub, ShaderRootNode::Cube);

            if(result > -1) {
                QString uv = "_uv0";
                const AbstractNodeGraph::Link *l = m_graph->findLink(this, port(0)); // UV
                if(l) {
                    TextureSample *node = static_cast<TextureSample *>(l->sender);
                    if(node) {
                        int32_t type = 0;
                        uint32_t index = node->build(code, stack, *l, depth, type);
                        if(stack.isEmpty()) {
                            uv = convert("local" + QString::number(index), type, QMetaType::QVector3D);
                        } else {
                            uv = convert(stack.pop(), type, QMetaType::QVector3D);
                        }
                    }
                }

                uv = "vec3(" + uv + ", 1.0)";
                code += QString("\tvec4 lt%1 = texture(texture%2, %3);\n").arg(depth).arg(result).arg(uv);
            } else {
                m_graph->reportMessage(this, "Missing texture");
                return -1;
            }
        }

        int32_t result = ShaderNode::build(code, stack, link, depth, type);

        QString channel = "lt" + QString::number(result);
        if(link.oport->m_name == g) {
            channel.append(".x");
        } else if(link.oport->m_name == g) {
            channel.append(".y");
        } else if(link.oport->m_name == b) {
            channel.append(".z");
        } else if(link.oport->m_name == a) {
            channel.append(".w");
        }
        stack.push(channel);

        return result;
    }

};

#endif // TEXTURESAMPLE_H
