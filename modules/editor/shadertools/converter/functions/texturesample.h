#ifndef TEXTURESAMPLE_H
#define TEXTURESAMPLE_H

#include "function.h"

#define UV      "UV"

class TextureFunction : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Texture")

public:
    Q_INVOKABLE TextureFunction() {
        ports.push_back(new NodePort(false, QMetaType::QVector2D, 0, UV));
        ports.push_back(new NodePort(true,  QMetaType::QVector4D, 0, ""));
        ports.push_back(new NodePort(true,  QMetaType::Double, 1, r));
        ports.push_back(new NodePort(true,  QMetaType::Double, 2, g));
        ports.push_back(new NodePort(true,  QMetaType::Double, 3, b));
        ports.push_back(new NodePort(true,  QMetaType::Double, 4, a));
    }

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        if(m_position == -1) {
            QString uv = "_uv0";
            const AbstractNodeGraph::Link *l = graph->findLink(this, ports.at(0)); // UV
            if(l) {
                ShaderFunction *node = static_cast<ShaderFunction *>(l->sender);
                if(node) {
                    int32_t type = 0;
                    int32_t index = node->build(code, stack, graph, *l, depth, type);
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

        int32_t result = ShaderFunction::build(code, stack, graph, link, depth, size);

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

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        int result = graph->setTexture(m_path.path, m_sub, false);

        if(result < 0) {
            graph->reportMessage(this, "Missing texture");
            return -1;
        }
        m_name = QString("texture%1").arg(result);
        return TextureFunction::build(code, stack, graph, link, depth, size);
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

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        graph->setTexture(m_name, m_sub, ShaderRootNode::Target);

        return TextureFunction::build(code, stack, graph, link, depth, size);
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

    int32_t build(QString &code, QStack<QString> &stack, ShaderNodeGraph *graph, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &size) override {
        if(m_position == -1) {
            Vector4 sub;
            int result = graph->setTexture(m_path.path, sub, ShaderRootNode::Cube);

            if(result > -1) {
                QString uv = "_uv0";
                const AbstractNodeGraph::Link *l = graph->findLink(this, ports.at(0)); // UV
                if(l) {
                    TextureSample *node = static_cast<TextureSample *>(l->sender);
                    if(node) {
                        int32_t type = 0;
                        uint32_t index = node->build(code, stack, graph, *l, depth, type);
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
                graph->reportMessage(this, "Missing texture");
                return -1;
            }
        }

        int32_t result = ShaderFunction::build(code, stack, graph, link, depth, size);

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
