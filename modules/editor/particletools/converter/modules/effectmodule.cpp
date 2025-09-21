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
    const char *gValue("value");
    const char *gMin("min");
    const char *gMax("max");

    const char *gType("type");
    const char *gName("name");

    const char *gMode("Mode");
};

EffectModule::EffectModule() :
        m_effect(nullptr),
        m_stage(ParticleSpawn),
        m_enabled(true),
        m_blockUpdate(false),
        m_checkBoxWidget(nullptr) {
}

Widget *EffectModule::widget(Object *parent) {
    if(m_checkBoxWidget == nullptr) {
        TString moduleName = name();

        Actor *function = Engine::composeActor<CheckBox>(moduleName, parent);
        m_checkBoxWidget = function->getComponent<CheckBox>();
        RectTransform *checkBoxRect = m_checkBoxWidget->rectTransform();
        checkBoxRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
        checkBoxRect->setSize(Vector2(200.0f, 20.0f));

        m_checkBoxWidget->setChecked(m_enabled);
        m_checkBoxWidget->setText(moduleName);
        m_checkBoxWidget->setMirrored(true);

        Object::connect(m_checkBoxWidget, _SIGNAL(toggled(bool)), this, _SLOT(setEnabled(bool)));
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

    graph->emitSignal(_SIGNAL(effectUpdated()));
}

void EffectModule::setProperty(const char *name, const Variant &value) {
    if(value.isValid() && !m_blockUpdate) {
        TString localName(name);

        StringList list = localName.split('/');
        localName = list.back();

        int modeIndex = localName.indexOf(gMode);
        if(modeIndex > -1) {
            localName = localName.mid(0, modeIndex);

            EffectRootNode::ParameterData *data = m_effect->parameter(localName, this);
            if(data) {
                data->mode = static_cast<EffectModule::Space>(value.toInt());
            }

            setRoot(m_effect);
        } else {
            TString prop = list.front();
            prop = prop.mid(0, prop.indexOf(gMode));

            EffectRootNode::ParameterData *data = m_effect->parameter(prop, this);
            if(data) {
                if(localName == gMax) {
                    data->max = value;
                } else {
                    data->min = value;
                }

                EffectGraph *graph = static_cast<EffectGraph *>(m_effect->graph());
                graph->emitSignal(_SIGNAL(effectUpdated()));
            }
        }
    }

    Object::setProperty(name, value);
}
void EffectModule::getOperations(std::vector<EffectModule::OperationData> &operations) const {

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

        pugi::xml_node valueElement = element.append_child(gValue);
        EffectRootNode::fromVariantHelper(valueElement, property.read(this), annotation);
        valueElement.append_attribute(gName) = property.name();
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
                valueElement.text().set(metaEnum.valueToKey(value.toInt()));
            } else {
                pugi::xml_node valueElement = element.append_child(gValue);
                EffectRootNode::fromVariantHelper(valueElement, value, annotation);

                valueElement.append_attribute(gName) = dynProp.data();
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

            for(auto &it : m_effect->parameters(this)) {
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

Vector4 toVector(const Variant &variant, const TString &comp = TString()) {
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

    static const QMap<char, uint8_t> maps = {
        {'x', 0},
        {'y', 1},
        {'z', 2},
        {'w', 3},
        {'r', 0},
        {'g', 1},
        {'b', 2},
        {'a', 3}
    };

    Vector4 local(result);
    for(int i = 0; i < comp.size(); i++) {
        result[i] = local[maps.value(comp.at(i))];
    }

    return result;
}

VariantList EffectModule::saveData() const {
    VariantList operations;

    std::vector<EffectModule::OperationData> ops;
    getOperations(ops);
    for(auto it : ops) {
        VariantList data;
        data.push_back(it.operation);

        data.push_back(it.result.space);
        data.push_back(it.result.size);
        data.push_back(it.result.offset);

        VariantList arguments;

        for(size_t arg = 0; arg < it.arguments.size(); arg++) {
            const VariableData &data = it.arguments.at(arg);

            VariantList argument;

            argument.push_back(data.space);
            argument.push_back(data.size);

            switch(data.space) {
                case EffectModule::Constant: {
                    for(int i = 0; i < data.size; i++) {
                        argument.push_back(data.min[i]);
                    }
                } break;
                case EffectModule::Random: {
                    for(int i = 0; i < data.size; i++) {
                        argument.push_back(data.min[i]);
                    }

                    for(int i = 0; i < data.size; i++) {
                        argument.push_back(data.max[i]);
                    }
                } break;
                default: {
                    argument.push_back(data.offset);
                } break;
            }

            arguments.push_back(argument);
        }

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

    for(auto &it : m_effect->parameters(this)) {
        if(!it.visible) {
            continue;
        }

        if(!it.modeType.isEmpty()) {
            TString type = it.name + gMode;

            setProperty(type.data(), it.mode);
            setDynamicPropertyInfo(type.data(), "enum=Space");

            if(it.mode == EffectModule::Random) {
                TString minName = type + "/" + gMin;
                TString maxName = type + "/" + gMax;

                setProperty(minName.data(), it.min);
                setProperty(maxName.data(), it.max);
                if(!it.type.isEmpty()) {
                    setDynamicPropertyInfo(minName.data(), annotationHelper(it.type));
                    setDynamicPropertyInfo(maxName.data(), annotationHelper(it.type));
                }

            } else if(it.mode == EffectModule::Constant) {
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

    graph->emitSignal(_SIGNAL(moduleChanged()));
}

const char *EffectModule::annotationHelper(const TString &type) const {
    if(type == "template") {
        return "editor=Asset";
    } else if(type == "color") {
        return "editor=Color";
    }

    return "";
}

int EffectModule::typeSize(uint32_t type) {
    switch(type) {
        case MetaType::FLOAT: return 1;
        case MetaType::VECTOR2: return 2;
        case MetaType::VECTOR3: return 3;
        case MetaType::VECTOR4: return 4;
        case MetaType::MATRIX4: return 16;
        default: break;
    }

    return 0;
}

MetaType::Type EffectModule::type(const TString &name) {
    static const std::map<TString, MetaType::Type> locals = {
        {"float", MetaType::FLOAT},
        {"vec2",  MetaType::VECTOR2},
        {"vec3",  MetaType::VECTOR3},
        {"vec4",  MetaType::VECTOR4},
        {"mat4",  MetaType::MATRIX4},
    };

    auto it = locals.find(name);
    if(it != locals.end()) {
        return it->second;
    }

    return MetaType::INVALID;
}

EffectModule::VariableData EffectModule::variable(const TString &name) const {
    VariableData result;

    MetaType::Type type = EffectModule::type(name);
    if(type != MetaType::INVALID) {
         result.size = typeSize(type);
         result.space = Space::_Local;
    } else {
        result.space = EffectRootNode::getSpace(name);
        result.offset = m_effect->attributeOffset(name);
        result.size = m_effect->attributeSize(name);
    }

    if(result.offset == -1 && !name.isEmpty()) {
        if(result.space != Space::None) {
            // Module uses invalid attribute
        } else {
            StringList split = name.split('.');
            const EffectRootNode::ParameterData *parameter = m_effect->parameterConst(split.front(), false);
            if(parameter) {
                result.size = EffectModule::typeSize(parameter->min.type());

                TString comp;
                if(split.size() > 1) {
                    comp = split.back();
                    result.size = comp.size();
                }
                result.min = toVector(parameter->min, comp);
                result.max = toVector(parameter->max, comp);

                result.space = parameter->mode;
            } else {
                Variant v = EffectRootNode::toVariantHelper(name, "auto");
                result.min = result.max = toVector(v);

                result.size = EffectModule::typeSize(v.type());
                result.space = EffectModule::Constant;
            }
        }
    }

    return result;
}
