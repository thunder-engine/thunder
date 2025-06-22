#ifndef CUSTOMFUNCTION_H
#define CUSTOMFUNCTION_H

#include "function.h"

#include <QFileInfo>

class CustomFunction : public ShaderNode {
    A_OBJECT(CustomFunction, ShaderNode, Graph)

public:
    CustomFunction() { }

    void exposeFunction(const std::string &path) {
        QFile file(path.c_str());
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QDomDocument doc;
            if(doc.setContent(&file)) {
                QDomElement function = doc.documentElement();

                m_funcName = QFileInfo(function.attribute("name")).baseName().toStdString();
                setTypeName(m_funcName);
                setName(m_funcName);

                QDomNode n = function.firstChild();
                while(!n.isNull()) {
                    QDomElement element = n.toElement();

                    if(element.tagName() == "inputs") { // parse inputs
                        QDomNode inputNode = element.firstChild();
                        while(!inputNode.isNull()) {
                            QDomElement inputElement = inputNode.toElement();

                            QString inputName = inputElement.attribute("name");

                            uint32_t type = convertType(inputElement.attribute("type"));

                            m_inputs.push_back(make_pair(inputName.toStdString(), type));

                            QString value = inputElement.attribute("embedded");
                            if(value.isEmpty()) {
                                value = inputElement.attribute("default");
                                if(!value.isEmpty()) {
                                    setProperty(qPrintable(inputName), convertValue(type, value));
                                }
                            } else {
                                setProperty(qPrintable(inputName), value.toStdString());
                            }

                            inputNode = inputNode.nextSibling();
                        }
                    } else if(element.tagName() == "outputs") {
                        QDomNode outputNode = element.firstChild();
                        while(!outputNode.isNull()) {
                            QDomElement outputElement = outputNode.toElement();

                            QString outputName = outputElement.attribute("name");

                            uint32_t type = convertType(outputElement.attribute("type"));

                            m_outputs.push_back(make_pair(outputName.toStdString(), type));

                            outputNode = outputNode.nextSibling();
                        }
                    } else if(element.tagName() == "code") {
                        m_func = element.text().toStdString();
                    }

                    n = n.nextSibling();
                }

                ShaderNode::createParams();
            }
        }
    }

    uint32_t convertType(const QString &type) const {
        if(type == "int") {
            return MetaType::INTEGER;
        } else if(type == "float") {
            return MetaType::FLOAT;
        } else if(type == "vec2") {
            return MetaType::VECTOR2;
        } else if(type == "vec3") {
            return MetaType::VECTOR3;
        } else if(type == "vec4") {
            return MetaType::VECTOR4;
        } else if(type == "image") {
            return MetaType::STRING;
        }

        return MetaType::INVALID;
    }

    Variant convertValue(uint32_t type, const QString &value) {
        QStringList values = value.simplified().split(',');

        switch(type) {
            case MetaType::INTEGER: return values[0].toInt();
            case MetaType::FLOAT: return values[0].toFloat();
            case MetaType::VECTOR2: return Vector2(values[0].toFloat(), values[1].toFloat());
            case MetaType::VECTOR3: return Vector3(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());
            case MetaType::VECTOR4: return Vector4(values[0].toFloat(), values[1].toFloat(),
                                                   values[2].toFloat(), values[3].toFloat());
            case MetaType::STRING:  return Variant::fromValue(Engine::loadResource<Texture>(value.toStdString()));
            default: break;
        }

        return Variant();
    }

    void createParams() override {
        QFile file(m_path.c_str());
        if(file.open(QIODevice::ReadOnly)) {
            m_func = file.readAll().toStdString();
            m_funcName = QFileInfo(m_path.c_str()).baseName().toStdString();

            ShaderNode::createParams();
        }
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            if(!m_func.empty()) {
                static_cast<ShaderGraph *>(m_graph)->addFunction(m_funcName, m_func);

                if(link.oport->m_type != MetaType::INVALID) {
                    type = link.oport->m_type;
                }
                QStringList arguments = getArguments(code, stack, depth, type);

                QString expr = QString("%1(%2)").arg(m_funcName.c_str(), arguments.join(", "));
                if(m_graph->isSingleConnection(link.oport)) {
                    stack.push(expr);
                } else {
                    code.append(localValue(type, depth, expr));
                }
            }
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const std::string &key, uint32_t &type) const override {
        Variant value = property(key.c_str());

        if(value.type() == QVariant::String) {
            return value.toString().c_str();
        }

        type = MetaType::INVALID;
        for(auto &it : m_inputs) {
            if(it.first == key) {
                type = it.second;
                break;
            }
        }

        switch(type) {
            case MetaType::FLOAT: {
                return QString::number(value.toFloat());
            }
            case MetaType::VECTOR2: {
                Vector2 v = value.value<Vector2>();
                return QString("vec2(%1, %2)").arg(v.x).arg(v.y);
            }
            case MetaType::VECTOR3: {
                Vector3 v = value.value<Vector3>();
                return QString("vec3(%1, %2, %3)").arg(v.x).arg(v.y).arg(v.z);
            }
            case MetaType::VECTOR4: {
                Vector4 v = value.value<Vector4>();
                return QString("vec4(%1, %2, %3, %4)").arg(v.x).arg(v.y).arg(v.z).arg(v.w);
            }
            default: break;
        }

        return QString();
    }
/*
    bool event(QEvent *e) override {
        if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
            QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
            QVariant value = property(qPrintable(ev->propertyName()));
            if(value.isValid()) {
                emit updated();
            }
        }
        return false;
    }
*/

protected:
    std::string m_path;

    std::string m_func;
    std::string m_funcName;

};

#endif // CUSTOMFUNCTION_H
