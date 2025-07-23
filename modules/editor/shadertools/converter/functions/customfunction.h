#ifndef CUSTOMFUNCTION_H
#define CUSTOMFUNCTION_H

#include "function.h"

#include <QFileInfo>
#include <pugixml.hpp>

class CustomFunction : public ShaderNode {
    A_OBJECT(CustomFunction, ShaderNode, Graph)

public:
    CustomFunction() { }

    void exposeFunction(const TString &path) {
        QFile file(path.data());
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            pugi::xml_document doc;
            if(doc.load_string(file.readAll().data()).status == pugi::status_ok) {
                pugi::xml_node function = doc.document_element();

                m_funcName = QFileInfo(function.attribute("name").as_string()).baseName().toStdString();
                setTypeName(m_funcName);
                setName(m_funcName);

                pugi::xml_node element = function.first_child();
                while(element) {
                    std::string name(element.name());
                    if(name == "inputs") { // parse inputs
                        pugi::xml_node inputElement = element.first_child();
                        while(inputElement) {
                            TString inputName = inputElement.attribute("name").as_string();

                            uint32_t type = convertType(inputElement.attribute("type").as_string());

                            m_inputs.push_back(std::make_pair(inputName, type));

                            TString value = inputElement.attribute("embedded").as_string();
                            if(value.isEmpty()) {
                                value = inputElement.attribute("default").as_string();
                                if(!value.isEmpty()) {
                                    setProperty(inputName.data(), convertValue(type, value.data()));
                                }
                            } else {
                                setProperty(inputName.data(), value);
                            }

                            inputElement = inputElement.next_sibling();
                        }
                    } else if(name == "outputs") {
                        pugi::xml_node outputElement = element.first_child();
                        while(outputElement) {
                            TString outputName = outputElement.attribute("name").as_string();

                            uint32_t type = convertType(outputElement.attribute("type").as_string());

                            m_outputs.push_back(std::make_pair(outputName, type));

                            outputElement = outputElement.next_sibling();
                        }
                    } else if(name == "code") {
                        m_func = element.child_value();
                    }

                    element = element.next_sibling();
                }
            }

            ShaderNode::createParams();
        }
    }

    uint32_t convertType(const TString &type) const {
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
        QFile file(m_path.data());
        if(file.open(QIODevice::ReadOnly)) {
            m_func = file.readAll().toStdString();
            m_funcName = QFileInfo(m_path.data()).baseName().toStdString();

            ShaderNode::createParams();
        }
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            if(!m_func.isEmpty()) {
                static_cast<ShaderGraph *>(m_graph)->addFunction(m_funcName, m_func);

                if(link.oport->m_type != MetaType::INVALID) {
                    type = link.oport->m_type;
                }
                QStringList arguments = getArguments(code, stack, depth, type);

                QString expr = QString("%1(%2)").arg(m_funcName.data(), arguments.join(", "));
                if(m_graph->isSingleConnection(link.oport)) {
                    stack.push(expr);
                } else {
                    code.append(localValue(type, depth, expr));
                }
            }
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const TString &key, uint32_t &type) const override {
        Variant value = property(key.data());

        if(value.type() == MetaType::STRING) {
            return value.toString().data();
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

protected:
    TString m_path;

    TString m_func;
    TString m_funcName;

};

#endif // CUSTOMFUNCTION_H
