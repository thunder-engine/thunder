#include "effectrootnode.h"

#include <QMetaProperty>

#include <engine.h>

#include <components/frame.h>
#include <components/label.h>
#include <components/image.h>
#include <components/button.h>
#include <components/layout.h>
#include <components/checkbox.h>
#include <components/recttransform.h>

#include <components/foldout.h>

#include "effectgraph.h"
#include "effectmodule.h"

namespace {
    const char *gModules("modules");
    const char *gModule("module");
    const char *gType("type");
    const char *gName("name");
    const char *gValue("value");
}

EffectRootNode::EffectRootNode() :
        m_spawnFold(nullptr),
        m_updateFold(nullptr),
        m_spawnRate(1.0f),
        m_capacity(32),
        m_gpu(false),
        m_local(false),
        m_continuous(true) {

    setTypeName("EffectRootNode");

    addEmitterAttribute("age", MetaType::FLOAT);
    addEmitterAttribute("deltaTime", MetaType::FLOAT);
    addEmitterAttribute("spawnCounter", MetaType::FLOAT);

    addParticleAttribute("age", MetaType::FLOAT);
    addParticleAttribute("lifetime", MetaType::FLOAT);
    addParticleAttribute("position", MetaType::VECTOR3);
    addParticleAttribute("rotation", MetaType::VECTOR3);
    addParticleAttribute("size", MetaType::VECTOR3);
    addParticleAttribute("color", MetaType::VECTOR4);
    addParticleAttribute("velocity", MetaType::VECTOR3);
}

EffectRootNode::~EffectRootNode() {

}

std::string EffectRootNode::meshPath() const {
    return m_meshPath;
}

std::string EffectRootNode::materialPath() const {
    return m_materialPath;
}

Vector4 EffectRootNode::color() const {
    return Vector4(0.141f, 0.384f, 0.514f, 1.0f);
}

QDomElement EffectRootNode::toXml(QDomDocument &xml) {
    QDomElement result = GraphNode::toXml(xml);

    QDomElement modulesElement = xml.createElement(gModules);
    for(auto it : m_modules) {
        const QMetaObject *meta = it->metaObject();

        QDomElement moduleElement = xml.createElement(gModule);
        moduleElement.setAttribute(gType, it->objectName());

        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property = meta->property(i);
            if(property.isUser(this)) {
                QDomElement valueElement = fromVariant(property.read(it), xml);
                valueElement.setAttribute(gName, property.name());

                moduleElement.appendChild(valueElement);
            }
        }

        for(auto property : it->dynamicPropertyNames()) {
            QDomElement valueElement = fromVariant(it->property(property), xml);
            valueElement.setAttribute(gName, qPrintable(property));

            moduleElement.appendChild(valueElement);
        }

        modulesElement.appendChild(moduleElement);
    }
    result.appendChild(modulesElement);

    return result;
}

void EffectRootNode::fromXml(const QDomElement &element) {
    GraphNode::fromXml(element);

    EffectGraph *graph = static_cast<EffectGraph *>(m_graph);

    QDomElement modulesElement = element.firstChildElement(gModules);
    if(!modulesElement.isNull()) {
        QDomElement moduleElement = modulesElement.firstChildElement(gModule);
        while(!moduleElement.isNull()) {
            QString modulePath = graph->modulePath(moduleElement.attribute(gType));

            EffectModule *function = addModule(modulePath.toStdString());

            if(function) {
                const QMetaObject *meta = function->metaObject();

                QDomElement valueElement = moduleElement.firstChildElement(gValue);
                while(!valueElement.isNull()) {
                    QString type = valueElement.attribute(gType);
                    QString name = valueElement.attribute(gName);

                    QVariant value = toVariant(valueElement.text(), type);
                    if(type == "EffectModule::Mode") {
                        const QMetaObject *meta = function->metaObject();
                        int index = meta->indexOfEnumerator("Mode");
                        if(index > -1) {
                            QMetaEnum metaEnum = meta->enumerator(index);
                            value = QVariant::fromValue(static_cast<EffectModule::Mode>(metaEnum.keyToValue(qPrintable(valueElement.text()))));
                        }
                    }
                    function->setProperty(qPrintable(name), value);

                    valueElement = valueElement.nextSiblingElement();
                }
            }

            moduleElement = moduleElement.nextSiblingElement();
        }
    }
}

Widget *EffectRootNode::widget() {
    Widget *result = GraphNode::widget();

    if(m_spawnFold == nullptr) {
        Actor *spawnActor = Engine::composeActor("Foldout", "Spawn", result->actor());
        m_spawnFold = spawnActor->getComponent<Foldout>();
        m_spawnFold->setText("Particle Spawn");

        RectTransform *spawnFoldRect = m_spawnFold->rectTransform();
        spawnFoldRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));

        Actor *updateActor = Engine::composeActor("Foldout", "Update", result->actor());
        m_updateFold = updateActor->getComponent<Foldout>();
        m_updateFold->setText("Particle Update");

        RectTransform *updateFoldRect = m_updateFold->rectTransform();
        updateFoldRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));

        RectTransform *rect = result->rectTransform();
        rect->setPadding(Vector4(0.0f, 0.0f, 10.0f, 0.0f));

        Layout *layout = rect->layout();
        layout->addTransform(spawnFoldRect);
        layout->addTransform(updateFoldRect);

        for(auto it : m_modules) {
            switch(it->stage()) {
                case EffectModule::Spawn: {
                    m_spawnFold->addWidget(it->widget(m_spawnFold->actor()));
                } break;
                case EffectModule::Update: {
                    m_spawnFold->addWidget(it->widget(m_spawnFold->actor()));
                } break;
                default: break;
            }
        }
    }

    return result;
}

void EffectRootNode::addEmitterAttribute(const std::string &name, int32_t type) {
    for(auto it : m_emitterAttributes) {
        if(it.first == name) {
            return;
        }
    }

    m_emitterAttributes.push_back(std::make_pair(name, typeSize(type)));
}

int EffectRootNode::emitterAttributeOffset(const std::string &name) {
    int result = 0;
    for(auto it : m_emitterAttributes) {
        if(it.first == name) {
            return result;
        }
        result += it.second;
    }
    return -1;
}

int EffectRootNode::emitterAttributeSize(const std::string &name) {
    for(auto it : m_emitterAttributes) {
        if(it.first == name) {
            return it.second;
        }
    }

    return 0;
}

void EffectRootNode::addParticleAttribute(const std::string &name, int32_t type) {
    for(auto it : m_particleAttributes) {
        if(it.first == name) {
            return;
        }
    }

    m_particleAttributes.push_back(std::make_pair(name, typeSize(type)));
}

int EffectRootNode::particleAttributeOffset(const std::string &name) {
    int result = 0;
    for(auto it : m_particleAttributes) {
        if(it.first == name) {
            return result;
        }
        result += it.second;
    }
    return -1;
}

int EffectRootNode::particleAttributeSize(const std::string &name) {
    for(auto it : m_particleAttributes) {
        if(it.first == name) {
            return it.second;
        }
    }

    return 0;
}

VariantList EffectRootNode::saveData() const {
    VariantList result;

    result.push_back(meshPath());
    result.push_back(materialPath());

    result.push_back(isGpu());
    result.push_back(isLocal());
    result.push_back(isContinuous());

    int stride = 0;
    for(auto it : m_particleAttributes) {
        stride += it.second;
    }

    result.push_back(capacity());
    result.push_back(stride);
    result.push_back(spawnRate());

    VariantList spawnOperations;
    VariantList updateOperations;
    VariantList renderOperations;

    for(const auto module : m_modules) {
        if(!module->enabled()) {
            continue;
        }

        VariantList data = module->saveData();

        switch(module->stage()) {
            case EffectModule::Spawn: {
                spawnOperations.insert(spawnOperations.end(), data.begin(), data.end());
            } break;
            case EffectModule::Update: {
                updateOperations.insert(updateOperations.end(), data.begin(), data.end());
            } break;
            case EffectModule::Render: {
                renderOperations.insert(renderOperations.end(), data.begin(), data.end());
            } break;
            default: break;
        }
    }

    result.push_back(spawnOperations);
    result.push_back(updateOperations);
    result.push_back(renderOperations);

    return result;
}

void EffectRootNode::onShowMenu() const {
    EffectGraph *graph = static_cast<EffectGraph *>(m_graph);
    graph->showFunctionsMenu();
}

EffectModule *EffectRootNode::addModule(const std::string &path) {
    EffectModule *module = new EffectModule;

    module->load(path);

    connect(module, &EffectModule::updated, static_cast<EffectGraph *>(m_graph), &EffectGraph::effectUpdated);
    connect(module, &EffectModule::moduleChanged, static_cast<EffectGraph *>(m_graph), &EffectGraph::moduleChanged);

    m_modules.push_back(module);

    module->setParent(this);
    module->setRoot(this);

    if(m_nodeWidget) {
        Widget *widget = module->widget(m_nodeWidget->actor());

        switch(module->stage()) {
            case EffectModule::Spawn: {
                m_spawnFold->addWidget(widget);
            } break;
            case EffectModule::Update: {
                m_updateFold->addWidget(widget);
            } break;
            default: break;
        }

        //RectTransform *rect = m_nodeWidget->rectTransform();
        //Layout *layout = rect->layout();
        //layout->invalidate();
        //layout->update();
    }

    return module;
}

void EffectRootNode::removeModule(EffectModule *function) {
    m_modules.remove(function);

    Widget *widget = function->widget(nullptr);
    delete widget->actor();

    delete function;
}

int EffectRootNode::typeSize(int type) {
    switch(type) {
        case MetaType::VECTOR2: return 2;
        case MetaType::VECTOR3: return 3;
        case MetaType::VECTOR4: return 4;
        default: break;
    }
    return 1;
}
