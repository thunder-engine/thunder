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

    const char *gMode("Mode");
};

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)

class ModuleObserver : public Object {
    A_REGISTER(ModuleObserver, Object, Editor)

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

            QByteArrayList list = name.split('/');
            name = list.last();

            if(value.isValid()) {
                int modeIndex = name.indexOf(gMode);
                if(modeIndex > -1) {
                    name = name.mid(0, modeIndex);
                    EffectModule::ParameterData *data = parameter(name.toStdString());
                    if(data) {
                        data->mode = value.value<Mode>();
                    }

                    setRoot(m_effect);
                } else {
                    QByteArray prop = list.front();
                    prop = prop.mid(0, prop.indexOf(gMode));

                    EffectModule::ParameterData *data = parameter(prop.toStdString());

                    int dataType = 0;

                    Vector4 *v = nullptr;

                    if(data) {
                        dataType = data->dataType;
                        v = &data->min;
                        if(name == gMax) {
                            v = &data->max;
                        }
                    }

                    if(v) {
                        switch (dataType) {
                            case MetaType::FLOAT: {
                                v->x = value.toFloat();
                            } break;
                            case MetaType::VECTOR2: {
                                *v = value.value<Vector2>();
                            } break;
                            case MetaType::VECTOR3: {
                                *v = value.value<Vector3>();
                            } break;
                            case MetaType::VECTOR4: {
                                QColor color = value.value<QColor>();
                                *v = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
                            } break;
                            default: break;
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

            QString moduleName = QFileInfo(function.attribute("name")).baseName();
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

                if(element.tagName() == "params") { // parse inputs
                    QDomNode paramNode = element.firstChild();
                    while(!paramNode.isNull()) {
                        QDomElement paramElement = paramNode.toElement();

                        ParameterData data;

                        static const QMap<QString, MetaType::Type> types = {
                            {"float", MetaType::FLOAT},
                            {"vec2", MetaType::VECTOR2},
                            {"vec3", MetaType::VECTOR3},
                            {"vec4", MetaType::VECTOR4}
                        };

                        static const QMap<QString, int32_t> modes = {
                            {"const", Mode::Constant},
                            {"range", Mode::Random}
                        };

                        data.name = paramElement.attribute("name").toStdString();
                        data.dataType = types.value(paramElement.attribute("type").toLower(), MetaType::FLOAT);
                        data.mode = modes.value(paramElement.attribute("mode").toLower(), -1);

                        addParameter(data);

                        paramNode = paramNode.nextSibling();
                    }
                } else if(element.tagName() == "operations") {
                    QDomNode operationNode = element.firstChild();
                    while(!operationNode.isNull()) {
                        QDomElement operationElement = operationNode.toElement();

                        OperationData data;

                        static const QMap<QString, Operation> operations = {
                            {"set", Operation::Set},
                            {"add", Operation::Add},
                            {"subtract", Operation::Subtract},
                            {"multiply", Operation::Multiply},
                            {"divide", Operation::Divide},
                        };

                        data.operation = operations.value(operationElement.attribute("code").toLower(), Operation::Set);
                        data.result = operationElement.attribute("result").toStdString();
                        data.arg1 = operationElement.attribute("arg1").toStdString();
                        data.arg2 = operationElement.attribute("arg2").toStdString();

                        addOperation(data);

                        operationNode = operationNode.nextSibling();
                    }
                }

                n = n.nextSibling();
            }
        }
    }
}

VariantList EffectModule::saveData() const {
    static const QMap<char, Space> spaces {
        {'s', EffectModule::System},
        {'e', EffectModule::Emitter},
        {'p', EffectModule::Particle},
        {'r', EffectModule::Renderable}
    };

    VariantList operations;
    for(auto it : m_operations) {
        VariantList data;
        data.push_back(it.operation);

        int32_t s = Particle;
        QByteArrayList list = QByteArray(it.result.c_str()).split('.');
        if(list.size() > 1) {
            s = spaces.value(list.front().at(0), Space::Particle);
        }

        int32_t offset = 0;
        int32_t size = 0;
        int32_t mode = -1;

        Vector4 min;
        Vector4 max;
        switch(s) {
            case Particle: {
                std::string name = list.back().toStdString();
                offset = m_effect->particleAttributeOffset(name);
                size = m_effect->particleAttributeSize(name);
                const ParameterData *attribute = parameterConst(name);
                if(attribute) {
                    mode = attribute->mode;
                    min = attribute->min;
                    max = attribute->max;
                }
            } break;
            default: break;
        }

        data.push_back(s);
        data.push_back(offset);
        data.push_back(size);

        data.push_back(mode);

        switch(mode) {
            case Constant: {
                for(int i = 0; i < size; i++) {
                    data.push_back(min[i]);
                }
            } break;
            case Random: {
                for(int i = 0; i < size; i++) {
                    data.push_back(min[i]);
                }

                for(int i = 0; i < size; i++) {
                    data.push_back(max[i]);
                }
            } break;
            default: {
                int32_t s = Particle;
                QByteArrayList list = QByteArray(it.arg1.c_str()).split('.');
                if(list.size() > 1) {
                    s = spaces.value(list.front().at(0), Space::Particle);
                }

                std::string name = list.back().toStdString();

                int32_t offset = 0;
                int32_t size = 0;
                switch(s) {
                    case Particle: {
                        offset = m_effect->particleAttributeOffset(name);
                        size = m_effect->particleAttributeSize(name);
                    } break;
                    case Emitter: {
                        offset = m_effect->emitterAttributeOffset(name);
                        size = m_effect->emitterAttributeSize(name);
                    } break;
                    default: break;
                }

                data.push_back(s);
                data.push_back(offset);
                data.push_back(size);
            } break;
        }

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

    for(auto it : m_parameters) {
        if(it.mode >= Constant) {
            m_effect->addParticleAttribute(it.name, it.dataType);

            setProperty((it.name + gMode).c_str(), QVariant::fromValue(static_cast<Mode>(it.mode)));
            setMode(it);
        }
    }
    blockSignals(false);

    emit moduleChanged();
}

void EffectModule::setMode(const ParameterData &data) {
    switch(data.mode) {
        case Constant: {
            std::string name = data.name + gMode + "/" + gValue;

            switch(data.dataType) {
                case MetaType::FLOAT: {
                    setProperty(name.c_str(), data.min.x);
                } break;
                case MetaType::VECTOR2: {
                    setProperty(name.c_str(), QVariant::fromValue(Vector2(data.min)));
                } break;
                case MetaType::VECTOR3: {
                    setProperty(name.c_str(), QVariant::fromValue(Vector3(data.min)));
                } break;
                case MetaType::VECTOR4: {
                    setProperty(name.c_str(), QColor::fromRgbF(data.min.x, data.min.y, data.min.z, data.min.w));
                } break;
                default: break;
            }
        } break;
        case Random: {
            std::string minName = data.name + gMode + "/" + gMin;
            std::string maxName = data.name + gMode + "/" + gMax;

            switch(data.dataType) {
                case MetaType::FLOAT: {
                    setProperty(minName.c_str(), data.min.x);
                    setProperty(maxName.c_str(), data.max.x);
                } break;
                case MetaType::VECTOR2: {
                    setProperty(minName.c_str(), QVariant::fromValue(Vector2(data.min)));
                    setProperty(maxName.c_str(), QVariant::fromValue(Vector2(data.max)));
                } break;
                case MetaType::VECTOR3: {
                    setProperty(minName.c_str(), QVariant::fromValue(Vector3(data.min)));
                    setProperty(maxName.c_str(), QVariant::fromValue(Vector3(data.max)));
                } break;
                case MetaType::VECTOR4: {
                    setProperty(minName.c_str(), QColor::fromRgbF(data.min.x, data.min.y, data.min.z, data.min.w));
                    setProperty(maxName.c_str(), QColor::fromRgbF(data.max.x, data.max.y, data.max.z, data.max.w));
                } break;
                default: break;
            }
        } break;
        default: break;
    }
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
