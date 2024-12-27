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

#include "SelectorEdit.h"

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

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
        m_renderFold(nullptr),
        m_spawnRate(1.0f),
        m_capacity(32),
        m_gpu(false),
        m_local(false),
        m_continuous(true) {

    setTypeName("EffectRootNode");

    addAttribute("e.age", 1, 0);
    addAttribute("e.deltaTime", 1, 1);
    addAttribute("e.spawnCounter", 1, 2);

    addAttribute("p.age", 1, 0);
    addAttribute("p.lifetime", 1, 1);
    addAttribute("p.position", 3, 2);
    addAttribute("p.rotation", 3, 5);
    addAttribute("p.size", 3, 8);
    addAttribute("p.color", 4, 11);
    addAttribute("p.velocity", 3, 15);
}

EffectRootNode::~EffectRootNode() {
    removeAllModules();
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
            QVariant value = it->property(property);
            if(value.canConvert<SelectorData>()) {
                QDomElement valueElement = xml.createElement(gValue);

                SelectorData select = value.value<SelectorData>();
                valueElement.setAttribute(gName, qPrintable(property));
                valueElement.setAttribute(gType, select.type);
                valueElement.appendChild(xml.createTextNode(select.current));

                moduleElement.appendChild(valueElement);
            } else {
                QDomElement valueElement = fromVariant(value, xml);

                valueElement.setAttribute(gName, qPrintable(property));
                moduleElement.appendChild(valueElement);
            }
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
                function->fromXml(moduleElement);
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

        Actor *renderActor = Engine::composeActor("Foldout", "Render", result->actor());
        m_renderFold = renderActor->getComponent<Foldout>();
        m_renderFold->setText("Render");

        RectTransform *renderFoldRect = m_renderFold->rectTransform();
        renderFoldRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));

        RectTransform *rect = result->rectTransform();
        rect->setPadding(Vector4(0.0f, 0.0f, 10.0f, 0.0f));

        Layout *layout = rect->layout();
        layout->addTransform(spawnFoldRect);
        layout->addTransform(updateFoldRect);
        layout->addTransform(renderFoldRect);

        for(auto it : m_modules) {
            switch(it->stage()) {
                case EffectModule::Spawn: {
                    m_spawnFold->addWidget(it->widget(m_spawnFold->actor()));
                } break;
                case EffectModule::Update: {
                    m_updateFold->addWidget(it->widget(m_updateFold->actor()));
                } break;
                case EffectModule::Render: {
                    m_renderFold->addWidget(it->widget(m_renderFold->actor()));
                } break;
                default: break;
            }
        }
    }

    return result;
}

void EffectRootNode::addAttribute(const std::string &name, int size, int offset) {
    for(auto it : m_attributes) {
        if(it.name == name) {
            return;
        }
    }

    AttributeData data;
    data.name = name;
    data.size = size;
    data.offset = offset;

    m_attributes.push_back(data);
}

int EffectRootNode::attributeOffset(const std::string &name) {
    std::string local = name;

    int32_t offset = 0;
    QByteArrayList list = QByteArray(name.c_str()).split('.');
    if(list.size() > 1) {
        local = (list.at(0) + "." + list.at(1)).toStdString();
        if(list.size() == 3) {
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

            offset = maps.value(list.back().at(0), 0);
        }
    }

    for(auto it : m_attributes) {
        if(it.name == local) {
            return it.offset + offset;
        }
    }

    return -1;
}

int EffectRootNode::attributeSize(const std::string &name) {
    std::string local = name;

    int32_t size = 0;
    QByteArrayList list = QByteArray(name.c_str()).split('.');
    if(list.size() > 1) {
        local = (list.at(0) + "." + list.at(1)).toStdString();
        if(list.size() == 3) {
            size = list.back().size();
        }
    }

    for(auto it : m_attributes) {
        if(it.name == local) {
            return (size > 0) ? MIN(size, it.size) : it.size;
        }
    }

    return 0;
}

int EffectRootNode::getSpace(const std::string &name) {
    static const QMap<char, EffectModule::Space> spaces {
        {'s', EffectModule::System},
        {'e', EffectModule::Emitter},
        {'p', EffectModule::Particle},
        {'r', EffectModule::Renderable}
    };

    QByteArrayList list = QByteArray(name.c_str()).split('.');
    if(list.size() > 1) {
        return spaces.value(list.front().at(0), EffectModule::Particle);
    }

    return -1;
}

VariantList EffectRootNode::saveData() const {
    VariantList result;

    std::string meshPath;
    std::string materialPath;

    for(auto it : m_modules) {
        if(it->enabled() && it->stage() == EffectModule::Stage::Render) {
            EffectModule::ParameterData *data = it->parameter("material");
            if(data) {
                materialPath = data->min.value<Template>().path.toStdString();
            }

            data = it->parameter("mesh");
            if(data) {
                meshPath = data->min.value<Template>().path.toStdString();
            }
        }
    }

    result.push_back(meshPath);
    result.push_back(materialPath);

    result.push_back(isGpu());
    result.push_back(isLocal());
    result.push_back(isContinuous());

    result.push_back(capacity());
    result.push_back(spawnRate());

    int particleStride = 0;
    for(auto it : m_attributes) {
        if(getSpace(it.name) == EffectModule::Particle) {
            particleStride += it.size;
        }
    }

    result.push_back(particleStride);

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

EffectModule *EffectRootNode::addModule(const std::string &path) {
    EffectModule *module = new EffectModule;

    module->setParent(this);
    module->setRoot(this);

    module->load(path);

    connect(module, &EffectModule::updated, static_cast<EffectGraph *>(m_graph), &EffectGraph::effectUpdated);
    connect(module, &EffectModule::moduleChanged, static_cast<EffectGraph *>(m_graph), &EffectGraph::moduleChanged);

    m_modules.push_back(module);

    if(m_nodeWidget) {
        Widget *widget = module->widget(m_nodeWidget->actor());

        switch(module->stage()) {
            case EffectModule::Spawn: {
                m_spawnFold->addWidget(widget);
            } break;
            case EffectModule::Update: {
                m_updateFold->addWidget(widget);
            } break;
            case EffectModule::Render: {
                m_renderFold->addWidget(widget);
            } break;
            default: break;
        }
    }

    return module;
}

void EffectRootNode::removeModule(EffectModule *function) {
    m_modules.remove(function);

    Widget *widget = function->widget(nullptr);
    delete widget->actor();

    delete function;
}

void EffectRootNode::removeAllModules() {
    for(auto it : m_modules) {
        Widget *widget = it->widget(nullptr);
        delete widget->actor();

        delete it;
    }
    m_modules.clear();
}

int EffectRootNode::typeSize(const QVariant &value) {
    if(value.canConvert<Vector2>()) {
        return 2;
    } else if(value.canConvert<Vector3>()) {
        return 3;
    } else if(value.canConvert<Vector4>() || value.canConvert<QColor>()) {
        return 4;
    }
    return 1;
}
