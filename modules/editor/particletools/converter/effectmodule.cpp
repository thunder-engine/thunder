#include "effectmodule.h"

#include <components/image.h>
#include <components/frame.h>
#include <components/label.h>
#include <components/recttransform.h>
#include <components/checkbox.h>

#include <amath.h>

#include <pugixml.hpp>

#include "effectrootnode.h"
#include "effectgraph.h"

namespace {
    const char *gCheckBoxWidget("CheckBox");

    const char *gValue("value");
    const char *gMin("min");
    const char *gMax("max");

    const char *gType("type");
    const char *gName("name");

    const char *gMode("Mode");
};

static const std::map<TString, int> locals = {
    {"float", 1},
    {"vec2",  2},
    {"vec3",  3},
    {"vec4",  4},
};

EffectModule::EffectModule() :
        m_effect(nullptr),
        m_stage(Spawn),
        m_enabled(true),
        m_blockUpdate(false),
        m_checkBoxWidget(nullptr) {
}

Widget *EffectModule::widget(Object *parent) {
    if(m_checkBoxWidget == nullptr) {
        TString moduleName = name();

        Actor *function = Engine::composeActor(gCheckBoxWidget, moduleName, parent);
        m_checkBoxWidget = function->getComponent<CheckBox>();
        RectTransform *checkBoxRect = m_checkBoxWidget->rectTransform();
        checkBoxRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
        checkBoxRect->setSize(Vector2(200.0f, 20.0f));

        Object::connect(m_checkBoxWidget, _SIGNAL(toggled(bool)), this, _SLOT(setEnabled(bool)));

        m_checkBoxWidget->setChecked(m_enabled);
        m_checkBoxWidget->setText(moduleName);
        m_checkBoxWidget->setMirrored(true);
    }

    return m_checkBoxWidget;
}

void EffectModule::setEnabled(bool enabled) {
    if(m_checkBoxWidget) {
        m_checkBoxWidget->blockSignals(true);
        m_checkBoxWidget->setChecked(enabled);
        m_checkBoxWidget->blockSignals(false);
    }

    m_enabled = enabled;

    EffectGraph *graph = static_cast<EffectGraph *>(m_effect->graph());
    graph->effectUpdated();
}

void EffectModule::setProperty(const char *name, const Variant &value) {
    if(value.isValid() && !m_blockUpdate) {
        QByteArray localName(name);

        QByteArrayList list = localName.split('/');
        localName = list.last();

        int modeIndex = localName.indexOf(gMode);
        if(modeIndex > -1) {
            localName = localName.mid(0, modeIndex);
            EffectModule::ParameterData *data = parameter(localName.toStdString());
            if(data) {
                data->mode = value.toInt();
            }

            setRoot(m_effect);
        } else {
            QByteArray prop = list.front();
            prop = prop.mid(0, prop.indexOf(gMode));

            EffectModule::ParameterData *data = parameter(prop.toStdString());

            if(data) {
                if(name == gMax) {
                    data->max = value;
                } else {
                    data->min = value;
                }

                EffectGraph *graph = static_cast<EffectGraph *>(m_effect->graph());
                graph->effectUpdated();
            }
        }
    }

    return Object::setProperty(name, value);
}

TString EffectModule::path() const {
    return m_path;
}

void EffectModule::load(const TString &path) {
    m_path = path;

    QFile file(path.data());
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        pugi::xml_document doc;
        if(doc.load_string(file.readAll().data()).status == pugi::status_ok) {
            pugi::xml_node function = doc.document_element();

            TString moduleName = QFileInfo(function.attribute(gName).as_string()).baseName().toStdString();
            setName(moduleName);

            static const QMap<TString, Stage> stages = {
                {"spawn", Stage::Spawn},
                {"update", Stage::Update},
                {"render", Stage::Render}
            };
            m_stage = stages.value(TString(function.attribute("stage").as_string()).toLower(), Stage::Spawn);

            pugi::xml_node element = function.first_child();
            while(element) {
                std::string name(element.name());
                if(name == "params") { // parse inputs
                    pugi::xml_node paramElement = element.first_child();
                    while(paramElement) {
                        ParameterData data;

                        data.name = paramElement.attribute(gName).as_string();
                        data.modeType = paramElement.attribute("mode").as_string();
                        data.type = paramElement.attribute(gType).as_string();
                        data.max = data.min = EffectRootNode::toVariantHelper(paramElement.attribute("default").as_string(), data.type);
                        TString visible = paramElement.attribute("visible").as_string();
                        if(!visible.isEmpty()) {
                            data.visible = visible == "true";
                        }

                        m_parameters.push_back(data);

                        paramElement = paramElement.next_sibling();
                    }
                } else if(name == "operations") {
                    pugi::xml_node operationElement = element.first_child();
                    while(operationElement) {
                        static const QMap<TString, Operation> operations = {
                            {"set", Operation::Set},
                            {"add", Operation::Add},
                            {"sub", Operation::Subtract},
                            {"mul", Operation::Multiply},
                            {"div", Operation::Divide},
                        };

                        OperationData data;
                        data.operation = operations.value(TString(operationElement.attribute("code").as_string()).toLower(), Operation::Set);
                        data.result = operationElement.attribute("result").as_string();
                        data.args.push_back(operationElement.attribute("arg0").as_string());
                        data.args.push_back(operationElement.attribute("arg1").as_string());

                        m_operations.push_back(data);

                        operationElement = operationElement.next_sibling();
                    }
                } else if(name == "bindings") {
                    pugi::xml_node bindElement = element.first_child();
                    while(bindElement) {
                        int size = 1;
                        auto it = locals.find(bindElement.attribute(gType).as_string());
                        if(it != locals.end()) {
                            size = it->second;
                        }

                        m_effect->addAttribute(bindElement.attribute(gName).as_string(),
                                               size,
                                               bindElement.attribute("offset").as_int());

                        bindElement = bindElement.next_sibling();
                    }
                }

                element = element.next_sibling();
            }

            setRoot(m_effect);
        }
    }
}

void EffectModule::toXml(pugi::xml_node &element) {
    element.append_attribute(gType) = name().data();

    const MetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);

        TString annotation;
        if(property.table()->annotation) {
            annotation = property.table()->annotation;
        }

        pugi::xml_node valueElement = EffectRootNode::fromVariantHelper(property.read(this), annotation);
        valueElement.append_attribute(gName) = property.name();

        element.append_copy(valueElement);
    }

    auto dynamicProperties = dynamicPropertyNames();

    MetaEnum metaEnum = meta->enumerator(meta->indexOfEnumerator("Space"));
    for(auto dynProp : dynamicProperties) {
        if(dynProp.front() != '_') {
            TString annotation = dynamicPropertyInfo(dynProp.data());

            Variant value = property(dynProp.data());
            if(annotation == "enum=Space") {
                pugi::xml_node valueElement = element.append_child(gValue);

                valueElement.append_attribute(gName) = dynProp.data();
                valueElement.append_attribute(gType) = dynProp.data();
                valueElement.set_value(metaEnum.valueToKey(value.toInt()));
            } else {
                pugi::xml_node valueElement = EffectRootNode::fromVariantHelper(value, annotation);

                valueElement.append_attribute(gName) = dynProp.data();
                element.append_copy(valueElement);
            }
        }
    }
}

void EffectModule::fromXml(const pugi::xml_node &element) {
    const MetaObject *meta = metaObject();

    MetaEnum metaEnum = meta->enumerator(meta->indexOfEnumerator("Space"));

    pugi::xml_node valueElement = element.first_child();
    while(valueElement) {
        if(std::string(valueElement.name()) == gValue) {
            TString type = valueElement.attribute(gType).value();
            TString name = valueElement.attribute(gName).value();
            TString value = valueElement.child_value();

            Variant variant = EffectRootNode::toVariantHelper(value, type);

            for(auto it : m_parameters) {
                if(it.modeType == name) {
                    int enumValue = metaEnum.keyToValue(value.data());
                    variant = Variant::fromValue(enumValue);

                    break;
                }
            }

            setProperty(name.data(), variant);
        }

        valueElement = valueElement.next_sibling();
    }

    setRoot(m_effect);
}

Vector4 toVector(const Variant &variant) {
    Vector4 result;

    switch(variant.type()) {
        case MetaType::FLOAT: {
            result.x = variant.toFloat();
        } break;
        case MetaType::VECTOR2: {
            result = variant.toVector2();
        } break;
        case MetaType::VECTOR3: {
            result = variant.toVector3();
        } break;
        case MetaType::VECTOR4: {
            result = variant.toVector4();
        } break;
        default: break;
    }

    return result;
}

VariantList EffectModule::saveData() const {
    VariantList operations;
    for(auto it : m_operations) {
        VariantList data;
        data.push_back(it.operation);

        int32_t returnSpace = _Particle;

        int32_t returnOffset = 0;
        int32_t returnSize = 0;

        auto regIt = locals.find(it.result);
        if(regIt != locals.end()) {
             returnSize = regIt->second;
             returnSpace = _Local;
        } else {
            returnSpace = EffectRootNode::getSpace(it.result);
            returnOffset = m_effect->attributeOffset(it.result);
            returnSize = m_effect->attributeSize(it.result);
        }

        VariantList arguments;

        for(size_t arg = 0; arg < it.args.size(); arg++) {
            VariantList argument;

            TString argName = it.args.at(arg);

            int32_t argSpace = EffectRootNode::getSpace(argName);
            int32_t argOffset = m_effect->attributeOffset(argName);
            int32_t argSize = m_effect->attributeSize(argName);

            Vector4 min;
            Vector4 max;

            auto regIt = locals.find(argName);
            if(regIt != locals.end()) {
                argSpace = _Local;
                argOffset = 0;
                argSize = regIt->second;
            } else if(argOffset == -1 && !argName.isEmpty()) {
                const ParameterData *attribute = parameterConst(argName);
                if(attribute) {
                    min = toVector(attribute->min);
                    max = toVector(attribute->max);

                    argSize = EffectRootNode::typeSize(attribute->min);

                    argSpace = attribute->mode;
                } else {
                    Variant v = EffectRootNode::toVariantHelper(argName, "auto");
                    min = max = toVector(v);

                    argSize = EffectRootNode::typeSize(v);
                    argSpace = Space::Constant;
                }
            }

            argument.push_back(argSpace);

            switch(argSpace) {
                case Constant: {
                    argument.push_back(argSize);
                    for(int i = 0; i < argSize; i++) {
                        argument.push_back(min[i]);
                    }
                } break;
                case Random: {
                    argument.push_back(argSize);
                    for(int i = 0; i < argSize; i++) {
                        argument.push_back(min[i]);
                    }

                    for(int i = 0; i < argSize; i++) {
                        argument.push_back(max[i]);
                    }
                } break;
                default: {
                    argument.push_back(argSize);
                    argument.push_back(argOffset);
                } break;
            }


            arguments.push_back(argument);
       }

        data.push_back(returnSpace);
        data.push_back(returnOffset);
        data.push_back(returnSize);

        data.push_back(arguments);

        operations.push_back(data);
    }

    return operations;
}

void EffectModule::setRoot(EffectRootNode *effect) {
    m_effect = effect;

    m_blockUpdate = true;
    auto names = dynamicPropertyNames();
    for(auto it : names) {
        setProperty(it.data(), Variant());
    }

    for(auto &it : m_parameters) {
        if(!it.visible) {
            continue;
        }

        if(!it.modeType.isEmpty()) {
            TString type = it.name + gMode;

            setProperty(type.data(), it.mode);
            setDynamicPropertyInfo(type.data(), "enum=Space");

            if(it.mode == Space::Random) {
                TString minName = type + "/" + gMin;
                TString maxName = type + "/" + gMax;

                setProperty(minName.data(), it.min);
                setProperty(maxName.data(), it.max);
                if(!it.type.isEmpty()) {
                    setDynamicPropertyInfo(minName.data(), annotationHelper(it.type));
                    setDynamicPropertyInfo(maxName.data(), annotationHelper(it.type));
                }

            } else if(it.mode == Space::Constant) {
                TString name = type + "/" + gValue;

                setProperty(name.data(), it.min);
                if(!it.type.isEmpty()) {
                    setDynamicPropertyInfo(name.data(), annotationHelper(it.type));
                }
            }
        } else {
            setProperty(it.name.data(), it.min);
            if(!it.type.isEmpty()) {
                setDynamicPropertyInfo(it.name.data(), annotationHelper(it.type));
            }
        }
    }
    m_blockUpdate = false;

    EffectGraph *graph = static_cast<EffectGraph *>(m_effect->graph());
    graph->moduleChanged();
}

EffectModule::ParameterData *EffectModule::parameter(const TString &name) {
    for(auto &it : m_parameters) {
        if(it.name == name) {
            return &it;
        }
    }
    return nullptr;
}

const EffectModule::ParameterData *EffectModule::parameterConst(const TString &name) const {
    for(auto &it : m_parameters) {
        if(it.name == name) {
            return &it;
        }
    }
    return nullptr;
}

const char *EffectModule::annotationHelper(const TString &type) const {
    if(type == "template") {
        return "editor=Asset";
    } else if(type == "color") {
        return "editor=Color";
    }

    return "";
}
