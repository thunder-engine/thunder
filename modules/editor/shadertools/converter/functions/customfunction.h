#ifndef CUSTOMFUNCTION_H
#define CUSTOMFUNCTION_H

#include "function.h"

#include <QFile>

#include <url.h>
#include <pugixml.hpp>

class CustomFunction : public ShaderNode {
    A_OBJECT(CustomFunction, ShaderNode, Graph)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    CustomFunction() { }

    void exposeFunction(const TString &path) {
        QFile file(path.data());
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray byteData = file.readAll();
            pugi::xml_document doc;
            if(doc.load_string(byteData.data()).status == pugi::status_ok) {
                pugi::xml_node function = doc.document_element();

                m_funcName = Url(function.attribute("name").as_string()).baseName();
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

    int32_t build(TString &code, std::stack<TString> &stack, const GraphLink &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            if(!m_func.isEmpty()) {
                static_cast<ShaderGraph *>(m_graph)->addFragmentFunction(m_funcName, m_func);

                int l_type = 0;
                std::vector<TString> args = getArguments(code, stack, depth, l_type);

                if(link.oport->m_type != MetaType::INVALID) {
                    type = link.oport->m_type;
                }

                TString expr = TString("%1(%2)").arg(m_funcName, TString::join(StringList(args.begin(), args.end()), ", "));
                if(m_graph->isSingleConnection(link.oport)) {
                    stack.push(expr);
                } else {
                    code.append(localValue(type, depth, expr));
                }
            }
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    TString defaultValue(const TString &key, uint32_t &type) const override {
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
                return TString::number(value.toFloat());
            }
            case MetaType::VECTOR2: {
                Vector2 v = value.value<Vector2>();
                return TString("vec2(%1, %2)").arg(TString::number(v.x), TString::number(v.y));
            }
            case MetaType::VECTOR3: {
                Vector3 v = value.value<Vector3>();
                return TString("vec3(%1, %2, %3)").arg(TString::number(v.x), TString::number(v.y), TString::number(v.z));
            }
            case MetaType::VECTOR4: {
                Vector4 v = value.value<Vector4>();
                return TString("vec4(%1, %2, %3, %4)").arg(TString::number(v.x), TString::number(v.y), TString::number(v.z), TString::number(v.w));
            }
            default: break;
        }

        return TString();
    }

protected:
    TString m_path;

    TString m_func;
    TString m_funcName;

};

#endif // CUSTOMFUNCTION_H
