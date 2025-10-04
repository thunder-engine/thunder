#include "custommodule.h"

#include "effectrootnode.h"

#include <QFile>

#include <url.h>

#include <pugixml.hpp>

namespace {
    const char *gType("type");
    const char *gName("name");
};

TString CustomModule::path() const {
    return m_path;
}

TString CustomModule::typeName() const {
    return name();
}

void CustomModule::getOperations(std::vector<OperationData> &operations) const {
    for(auto &it : m_operations) {
        OperationData data;
        data.operation = it.operation;
        data.result = variable(it.result);
        for(auto &arg : it.arguments) {
            data.arguments.push_back(variable(arg));
        }

        operations.push_back(data);
    }
}

void CustomModule::load(const TString &path) {
    m_path = path;

    QFile file(path.data());
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        pugi::xml_document doc;
        if(doc.load_string(file.readAll().data()).status == pugi::status_ok) {
            pugi::xml_node function = doc.document_element();

            TString moduleName = Url(function.attribute(gName).as_string()).baseName();
            setName(moduleName);

            int index = metaObject()->indexOfEnumerator("Stage");
            MetaEnum metaEnum = metaObject()->enumerator(index);

            m_stage = Stage::ParticleSpawn;
            int stage = metaEnum.keyToValue(function.attribute("stage").as_string());
            if(stage > -1) {
                m_stage = static_cast<Stage>(stage);
            }

            pugi::xml_node element = function.first_child();
            while(element) {
                std::string name(element.name());
                if(name == "params") { // parse inputs
                    pugi::xml_node paramElement = element.first_child();
                    while(paramElement) {
                        EffectRootNode::ParameterData data;

                        data.module = this;
                        data.name = paramElement.attribute(gName).as_string();
                        data.modeType = paramElement.attribute("mode").as_string();
                        data.type = paramElement.attribute(gType).as_string();
                        data.max = data.min = EffectRootNode::toVariantHelper(paramElement.attribute("default").as_string(), data.type);
                        TString visible = paramElement.attribute("visible").as_string();
                        if(!visible.isEmpty()) {
                            data.visible = visible == "true";
                        }

                        m_effect->addParameter(data);

                        paramElement = paramElement.next_sibling();
                    }
                } else if(name == "operations") {
                    pugi::xml_node operationElement = element.first_child();
                    while(operationElement) {
                        static const std::map<TString, Operation> operations = {
                            {"set", Operation::Set},
                            {"add", Operation::Add},
                            {"sub", Operation::Subtract},
                            {"mul", Operation::Multiply},
                            {"div", Operation::Divide},
                            {"mod", Operation::Mod},
                            {"min", Operation::Min},
                            {"max", Operation::Max},
                            {"floor", Operation::Floor},
                            {"ceil", Operation::Ceil}
                        };

                        OperationStr data;

                        data.operation = Operation::Set;
                        auto operationIt = operations.find(TString(operationElement.attribute("code").as_string()).toLower());
                        if(operationIt != operations.end()) {
                            data.operation = operationIt->second;
                        }

                        data.result = operationElement.attribute("result").as_string();
                        data.arguments.push_back(operationElement.attribute("arg0").as_string());

                        pugi::xml_attribute atrribute = operationElement.attribute("arg1");
                        if(atrribute) {
                            data.arguments.push_back(atrribute.as_string());
                        }

                        m_operations.push_back(data);

                        operationElement = operationElement.next_sibling();
                    }
                } else if(name == "attributes") {
                    pugi::xml_node attributes = element.first_child();
                    while(attributes) {
                        MetaType::Type type = MetaType::FLOAT;
                        MetaType::Type result = EffectModule::type(TString(attributes.attribute(gType).as_string()));
                        if(result != MetaType::INVALID) {
                            type = result;
                        }

                        m_effect->addAttribute(attributes.attribute(gName).as_string(), type);

                        attributes = attributes.next_sibling();
                    }
                }

                element = element.next_sibling();
            }

            setRoot(m_effect);
        }
    }
}
