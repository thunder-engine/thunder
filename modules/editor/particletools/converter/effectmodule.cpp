#include "effectmodule.h"

#include <components/image.h>
#include <components/frame.h>
#include <components/label.h>
#include <components/recttransform.h>
#include <components/checkbox.h>

#include <amath.h>

#include <QVariant>
#include <QEvent>
#include <QColor>
#include <QMetaEnum>

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

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

class ModuleObserver : public Object {
    A_OBJECT(ModuleObserver, Object, Editor)

    A_METHODS(
        A_SLOT(ModuleObserver::onModuleEnabled)
    )

public:
    ModuleObserver() :
            m_module(nullptr) {

    }

    void setEffectModule(EffectModule *module) {
        m_module = module;
    }

private:
    void onModuleEnabled(bool enabled) {
        m_module->setEnabled(enabled);
    }

private:
    EffectModule *m_module;

};

EffectModule::EffectModule() :
        m_effect(nullptr),
        m_stage(Spawn),
        m_enabled(true),
        m_checkBoxWidget(nullptr),
        m_observer(new ModuleObserver) {

    m_observer->setEffectModule(this);
}

Widget *EffectModule::widget(Object *parent) {
    if(m_checkBoxWidget == nullptr) {
        std::string moduleName = objectName().toStdString();

        Actor *function = Engine::composeActor(gCheckBoxWidget, moduleName, parent);
        m_checkBoxWidget = function->getComponent<CheckBox>();
        RectTransform *checkBoxRect = m_checkBoxWidget->rectTransform();
        checkBoxRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
        checkBoxRect->setSize(Vector2(200.0f, 20.0f));

        bool res = Object::connect(m_checkBoxWidget, _SIGNAL(toggled(bool)), m_observer, _SLOT(onModuleEnabled(bool)));

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
    emit updated();
}

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

void EffectModule::load(const std::string &path) {
    QFile file(path.c_str());
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument doc;
        if(doc.setContent(&file)) {
            QDomElement function = doc.documentElement();

            QString moduleName = QFileInfo(function.attribute(gName)).baseName();
            setObjectName(moduleName);

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
                        data.mode.type = paramElement.attribute("mode");
                        data.mode.current = paramElement.attribute("defaultMode");
                        data.max = data.min = EffectRootNode::toVariant(paramElement.attribute("default"), paramElement.attribute(gType));
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
        QString type = valueElement.attribute(gType);
        QString name = valueElement.attribute(gName);
        QString value = valueElement.text();

        QVariant variant = EffectRootNode::toVariant(valueElement.text(), type);
        auto it = m_options.find(type.toStdString());
        if(it != m_options.end()) {
            SelectorData data;
            data.current = value;
            data.type = type;
            for(auto option : it->second) {
                data.values << option.c_str();
            }
            variant = QVariant::fromValue(data);
        }

        setProperty(qPrintable(name), variant);

        valueElement = valueElement.nextSiblingElement();
    }
}

Vector4 toVector(const QVariant &variant) {
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

                    if(attribute->mode.current.toLower() == gRandom) {
                        argSpace = Space::Random;
                    } else {
                        argSpace = Space::Constant;
                    }
                } else {
                    QVariant v = EffectRootNode::toVariant(argName.c_str(), "auto");
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
        setProperty(it, QVariant());
    }

    for(auto &it : m_parameters) {
        if(!it.visible) {
            continue;
        }

        if(!it.mode.type.isEmpty()) {
            if(it.mode.values.empty()) {
                std::string type = it.mode.type.toStdString();
                auto optIt = m_options.find(type);
                for(auto opt : optIt->second) {
                    it.mode.values << opt.c_str();
                }
            }

            if(it.mode.current.isEmpty()) {
                it.mode.current = it.mode.values.front();
            }

            std::string type = it.name + gMode;
            setProperty(type.c_str(), QVariant::fromValue(it.mode));

            if(it.mode.current.toLower() == gRandom) {
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

    emit moduleChanged();
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
