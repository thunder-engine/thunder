#include "effectmodule.h"

#include <components/image.h>
#include <components/frame.h>
#include <components/label.h>
#include <components/recttransform.h>
#include <components/checkbox.h>

#include <amath.h>

#include <QDomDocument>

#include "effectrootnode.h"

namespace {
    const char *gCheckBoxWidget("CheckBox");

    const char *gValue("value");
    const char *gMin("min");
    const char *gMax("max");

    const char *gRandom("random");
    const char *gType("type");
    const char *gName("name");

    const char *gMode("Mode");
};

static const std::map<std::string, int> locals = {
    {"float", 1},
    {"vec2",  2},
    {"vec3",  3},
    {"vec4",  4},
};

EffectModule::EffectModule() :
        m_effect(nullptr),
        m_stage(Spawn),
        m_enabled(true),
        m_checkBoxWidget(nullptr) {
}

Widget *EffectModule::widget(Object *parent) {
    if(m_checkBoxWidget == nullptr) {
        std::string moduleName = name();

        Actor *function = Engine::composeActor(gCheckBoxWidget, moduleName, parent);
        m_checkBoxWidget = function->getComponent<CheckBox>();
        RectTransform *checkBoxRect = m_checkBoxWidget->rectTransform();
        checkBoxRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
        checkBoxRect->setSize(Vector2(200.0f, 20.0f));

        bool res = Object::connect(m_checkBoxWidget, _SIGNAL(toggled(bool)), this, _SLOT(onModuleEnabled(bool)));

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
}
/*
bool EffectModule::event(QEvent *e) {
    if(e->type() == QEvent::DynamicPropertyChange) {
        QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
        if(ev && !signalsBlocked()) {
            QByteArray name(ev->propertyName());
            QVariant value = property(name);

            if(value.isValid()) {
                QByteArrayList list = name.split('/');
                name = list.last();

                int modeIndex = name.indexOf(gMode);
                if(modeIndex > -1) {
                    name = name.mid(0, modeIndex);
                    EffectModule::ParameterData *data = parameter(name.toStdString());
                    if(data) {
                        data->mode = value.value<SelectorData>();
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

                        emit updated();
                    }
                }
            }
        }

        return true;
    }

    return QObject::event(e);
}
*/
void EffectModule::load(const std::string &path) {
    QFile file(path.c_str());
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument doc;
        if(doc.setContent(&file)) {
            QDomElement function = doc.documentElement();

            std::string moduleName = QFileInfo(function.attribute(gName)).baseName().toStdString();
            setName(moduleName);

            static const QMap<QString, Stage> stages = {
                {"spawn", Stage::Spawn},
                {"update", Stage::Update},
                {"render", Stage::Render}
            };
            m_stage = stages.value(function.attribute("stage").toLower(), Stage::Spawn);

            QDomNode n = function.firstChild();
            while(!n.isNull()) {
                QDomElement element = n.toElement();

                if(element.tagName() == "options") {
                    std::string name = element.attribute(gName).toStdString();
                    std::vector<std::string> values;

                    QDomNode optionNode = element.firstChild();
                    while(!optionNode.isNull()) {
                        QDomElement optionElement = optionNode.toElement();
                        values.push_back(optionElement.attribute(gValue).toStdString());

                        optionNode = optionNode.nextSibling();
                    }

                    m_options[name] = values;
                } else if(element.tagName() == "params") { // parse inputs
                    QDomNode paramNode = element.firstChild();
                    while(!paramNode.isNull()) {
                        QDomElement paramElement = paramNode.toElement();

                        ParameterData data;

                        data.name = paramElement.attribute(gName).toStdString();
                        data.mode.type = paramElement.attribute("mode").toStdString();
                        data.mode.current = paramElement.attribute("defaultMode").toStdString();
                        data.max = data.min = EffectRootNode::toVariantHelper(paramElement.attribute("default").toStdString(), paramElement.attribute(gType).toStdString());
                        QString visible = paramElement.attribute("visible");
                        if(!visible.isEmpty()) {
                            data.visible = visible == "true";
                        }

                        addParameter(data);

                        paramNode = paramNode.nextSibling();
                    }
                } else if(element.tagName() == "operations") {
                    QDomNode operationNode = element.firstChild();
                    while(!operationNode.isNull()) {
                        QDomElement operationElement = operationNode.toElement();

                        static const QMap<QString, Operation> operations = {
                            {"set", Operation::Set},
                            {"add", Operation::Add},
                            {"sub", Operation::Subtract},
                            {"mul", Operation::Multiply},
                            {"div", Operation::Divide},
                        };

                        OperationData data;
                        data.operation = operations.value(operationElement.attribute("code").toLower(), Operation::Set);
                        data.result = operationElement.attribute("result").toStdString();
                        data.args.push_back(operationElement.attribute("arg0").toStdString());
                        data.args.push_back(operationElement.attribute("arg1").toStdString());

                        addOperation(data);

                        operationNode = operationNode.nextSibling();
                    }
                }else if(element.tagName() == "bindings") {
                    QDomNode bindNode = element.firstChild();
                    while(!bindNode.isNull()) {
                        QDomElement bindElement = bindNode.toElement();

                        int size = 1;
                        auto it = locals.find(bindElement.attribute(gType).toStdString());
                        if(it != locals.end()) {
                            size = it->second;
                        }

                        m_effect->addAttribute(bindElement.attribute(gName).toStdString(),
                                               size,
                                               bindElement.attribute("offset").toInt());

                        bindNode = bindNode.nextSibling();
                    }
                }

                n = n.nextSibling();
            }

            setRoot(m_effect);
        }
    }
}

void EffectModule::fromXml(const QDomElement &element) {
    QDomElement valueElement = element.firstChildElement(gValue);
    while(!valueElement.isNull()) {
        std::string type = valueElement.attribute(gType).toStdString();
        std::string name = valueElement.attribute(gName).toStdString();
        std::string value = valueElement.text().toStdString();

        Variant variant = EffectRootNode::toVariantHelper(value, type);
        auto it = m_options.find(type);
        if(it != m_options.end()) {
            SelectorData data;
            data.current = value;
            data.type = type;
            for(auto option : it->second) {
                data.values.push_back(option);
            }
            //variant = Variant::fromValue(data);
        }

        setProperty(name.c_str(), variant);

        valueElement = valueElement.nextSiblingElement();
    }
}

Vector4 toVector(const Variant &variant) {
    Vector4 result;

    if(variant.canConvert<Vector2>()) {
        result = variant.value<Vector2>();
    } else if(variant.canConvert<Vector3>()) {
        result = variant.value<Vector3>();
    } else if(variant.canConvert<Vector4>()) {
        result = variant.value<Vector4>();
    } else if(variant.userType() == QMetaType::QColor) {
        QColor col = variant.value<QColor>();
        result = Vector4(col.redF(), col.greenF(), col.blueF(), col.alphaF());
    } else {
        result.x = variant.toFloat();
    }

    return result;
}

VariantList EffectModule::saveData() const {
    VariantList operations;
    for(auto it : m_operations) {
        VariantList data;
        data.push_back(it.operation);

        int32_t returnSpace = Particle;

        int32_t returnOffset = 0;
        int32_t returnSize = 0;

        auto regIt = locals.find(it.result);
        if(regIt != locals.end()) {
             returnSize = regIt->second;
             returnSpace = Local;
        } else {
            returnSpace = EffectRootNode::getSpace(it.result);
            returnOffset = m_effect->attributeOffset(it.result);
            returnSize = m_effect->attributeSize(it.result);
        }

        VariantList arguments;

        for(int arg = 0; arg < it.args.size(); arg++) {
            VariantList argument;

            std::string argName = it.args.at(arg);

            int32_t argSpace = EffectRootNode::getSpace(argName);
            int32_t argOffset = m_effect->attributeOffset(argName);
            int32_t argSize = m_effect->attributeSize(argName);

            Vector4 min;
            Vector4 max;

            auto regIt = locals.find(argName);
            if(regIt != locals.end()) {
                argSpace = Local;
                argOffset = 0;
                argSize = regIt->second;
            } else if(argOffset == -1 && !argName.empty()) {
                const ParameterData *attribute = parameterConst(argName);
                if(attribute) {
                    min = toVector(attribute->min);
                    max = toVector(attribute->max);

                    argSize = EffectRootNode::typeSize(attribute->min);

                    std::string data = attribute->mode.current;
                    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c){ return std::tolower(c); });
                    if(data == gRandom) {
                        argSpace = Space::Random;
                    } else {
                        argSpace = Space::Constant;
                    }
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

void EffectModule::addParameter(const ParameterData &data) {
    m_parameters.push_back(data);
}

void EffectModule::addOperation(const OperationData &data) {
    m_operations.push_back(data);
}

void EffectModule::setRoot(EffectRootNode *effect) {
    m_effect = effect;

    blockSignals(true);
    for(auto it : dynamicPropertyNames()) {
        setProperty(it.c_str(), Variant());
    }

    for(auto &it : m_parameters) {
        if(!it.visible) {
            continue;
        }

        if(!it.mode.type.empty()) {
            if(it.mode.values.empty()) {
                auto optIt = m_options.find(it.mode.type);
                for(auto opt : optIt->second) {
                    it.mode.values.push_back(opt);
                }
            }

            if(it.mode.current.empty()) {
                it.mode.current = it.mode.values.front();
            }

            std::string type = it.name + gMode;
            //setProperty(type.c_str(), Variant::fromValue(it.mode));

            std::string data = it.mode.current;
            std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c){ return std::tolower(c); });
            if(data == gRandom) {
                std::string minName = type + "/" + gMin;
                std::string maxName = type + "/" + gMax;

                setProperty(minName.c_str(), it.min);
                setProperty(maxName.c_str(), it.max);
            } else {
                std::string name = type + "/" + gValue;

                setProperty(name.c_str(), it.min);
            }
        } else {
            setProperty(it.name.c_str(), it.min);
        }
    }
    blockSignals(false);
}

EffectModule::ParameterData *EffectModule::parameter(const std::string &name) {
    for(auto &it : m_parameters) {
        if(it.name == name) {
            return &it;
        }
    }
    return nullptr;
}

const EffectModule::ParameterData *EffectModule::parameterConst(const std::string &name) const {
    for(auto &it : m_parameters) {
        if(it.name == name) {
            return &it;
        }
    }
    return nullptr;
}
