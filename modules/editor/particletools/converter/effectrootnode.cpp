#include "effectrootnode.h"

#include <engine.h>

#include <components/frame.h>
#include <components/label.h>
#include <components/image.h>
#include <components/button.h>
#include <components/layout.h>
#include <components/checkbox.h>
#include <components/recttransform.h>

#include <components/foldout.h>

#include <pugixml.hpp>

#include "effectgraph.h"
#include "modules/custommodule.h"
#include "modules/spritemodule.h"

namespace {
    const char *gModules("modules");
    const char *gModule("module");
    const char *gType("type");
}

EffectRootNode::EffectRootNode() :
        m_emitterUpdateFold(nullptr),
        m_particleSpawnFold(nullptr),
        m_particleUpdateFold(nullptr),
        m_renderFold(nullptr),
        m_capacity(32),
        m_gpu(false),
        m_local(false),
        m_continuous(true) {

    setTypeName("EffectEmitter");

    addAttribute("s.deltaTime", MetaType::FLOAT);

    addAttribute("e.age", MetaType::FLOAT);
    addAttribute("e.spawnCounter", MetaType::FLOAT);
    addAttribute("e.transform", MetaType::MATRIX4);
}

EffectRootNode::~EffectRootNode() {
    removeAllModules();
}

Vector4 EffectRootNode::color() const {
    return Vector4(0.141f, 0.384f, 0.514f, 1.0f);
}

void EffectRootNode::toXml(pugi::xml_node &element) {
    GraphNode::toXml(element);

    pugi::xml_node modulesElement = element.append_child(gModules);
    for(auto it : getChildren()) {
        pugi::xml_node moduleElement = modulesElement.append_child(gModule);
        static_cast<EffectModule *>(it)->toXml(moduleElement);
    }
}

void EffectRootNode::fromXml(const pugi::xml_node &element) {
    GraphNode::fromXml(element);

    pugi::xml_node modulesElement = element.first_child();
    while(modulesElement) {
        if(std::string(modulesElement.name()) == gModules) {
            EffectGraph *graph = static_cast<EffectGraph *>(m_graph);

            pugi::xml_node moduleElement = modulesElement.first_child();
            while(moduleElement) {
                if(std::string(moduleElement.name()) == gModule) {
                    EffectModule *module = insertModule(moduleElement.attribute(gType).value());
                    if(module) {
                        module->fromXml(moduleElement);
                    }
                }

                moduleElement = moduleElement.next_sibling();
            }
        }
        modulesElement = modulesElement.next_sibling();
    }
}

Foldout *EffectRootNode::createFold(const TString &name, Actor *parent) {
    Actor *spawnActor = Engine::composeActor("Foldout", name, parent);
    Foldout *result = spawnActor->getComponent<Foldout>();
    result->setText(name);

    RectTransform *rect = result->rectTransform();
    rect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));

    return result;
}

Widget *EffectRootNode::widget() {
    Widget *result = GraphNode::widget();

    if(m_particleSpawnFold == nullptr) {
        m_emitterUpdateFold = createFold("Emitter Update", result->actor());
        m_particleSpawnFold = createFold("Particle Spawn", result->actor());
        m_particleUpdateFold = createFold("Particle Update", result->actor());
        m_renderFold = createFold("Render", result->actor());

        RectTransform *rect = result->rectTransform();
        Vector4 padding(rect->padding());
        padding.z += 10.0f;
        rect->setPadding(padding);

        Layout *layout = rect->layout();
        layout->addTransform(m_emitterUpdateFold->rectTransform());
        layout->addTransform(m_particleSpawnFold->rectTransform());
        layout->addTransform(m_particleUpdateFold->rectTransform());
        layout->addTransform(m_renderFold->rectTransform());

        for(auto it : getChildren()) {
            EffectModule *module = static_cast<EffectModule *>(it);
            switch(module->stage()) {
                case EffectModule::EmitterUpdate: {
                    m_emitterUpdateFold->insertWidget(-1, module->widget(m_emitterUpdateFold->actor()));
                } break;
                case EffectModule::ParticleSpawn: {
                    m_particleSpawnFold->insertWidget(-1, module->widget(m_particleSpawnFold->actor()));
                } break;
                case EffectModule::ParticleUpdate: {
                    m_particleUpdateFold->insertWidget(-1, module->widget(m_particleUpdateFold->actor()));
                } break;
                case EffectModule::Render: {
                    m_renderFold->insertWidget(-1, module->widget(m_renderFold->actor()));
                } break;
                default: break;
            }
        }
    }

    return result;
}

void EffectRootNode::addAttribute(const TString &name, MetaType::Type type) {
    int offset = 0;
    for(auto it : m_attributes) {
        if(it.name == name) {
            return;
        }
        if(it.name.front() == name.front()) {
            offset += it.size;
        }
    }

    AttributeData data;
    data.name = name;
    data.offset = offset;

    switch(type) {
        case MetaType::VECTOR2: data.size = 2; break;
        case MetaType::VECTOR3: data.size = 3; break;
        case MetaType::VECTOR4: data.size = 4; break;
        case MetaType::MATRIX4: data.size = 16; break;
        default: data.size = 1; break;
    }

    m_attributes.push_back(data);
}

int EffectRootNode::attributeOffset(const TString &name) {
    TString local = name;

    int32_t offset = 0;
    StringList list = name.split('.');
    if(list.size() > 1) {
        local = list.front() + "." + *std::next(list.begin(), 1);
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

int EffectRootNode::attributeSize(const TString &name) {
    TString local = name;

    int32_t size = 0;
    StringList list = name.split('.');
    if(list.size() > 1) {
        local = list.front() + "." + *std::next(list.begin(), 1);
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

void EffectRootNode::addParameter(const ParameterData &data) {
    m_parameters.push_back(data);
}

const EffectRootNode::ParameterData *EffectRootNode::parameterConst(const TString &name, bool enabledOnly) const {
    for(auto &it : m_parameters) {
        if(it.name == name && (!enabledOnly || it.module->enabled())) {
            return &it;
        }
    }
    return nullptr;
}

EffectRootNode::ParameterData *EffectRootNode::parameter(const TString &name, EffectModule *module) {
    for(auto &it : m_parameters) {
        if(it.name == name && it.module == module) {
            return &it;
        }
    }
    return nullptr;
}

std::vector<EffectRootNode::ParameterData> EffectRootNode::parameters(EffectModule *owner) const {
    std::vector<EffectRootNode::ParameterData> result;

    for(auto &it : m_parameters) {
        if(it.module == owner) {
            result.push_back(it);
        }
    }

    return result;
}

EffectModule::Space EffectRootNode::getSpace(const TString &name) {
    static const QMap<char, EffectModule::Space> spaces {
        {'s', EffectModule::_System},
        {'e', EffectModule::_Emitter},
        {'p', EffectModule::_Particle},
        {'r', EffectModule::_Renderable}
    };

    StringList list = name.split('.');
    if(list.size() > 1) {
        return spaces.value(list.front().at(0), EffectModule::_Particle);
    }

    return EffectModule::None;
}

VariantList EffectRootNode::saveData() const {
    VariantList result;

    result.push_back(isGpu());
    result.push_back(isLocal());
    result.push_back(isContinuous());

    result.push_back(capacity());

    std::vector<int> strides;
    strides.resize(EffectModule::_Renderable);

    int particleStride = 0;
    for(auto &it : m_attributes) {
        int space = getSpace(it.name);
        if(space < EffectModule::_Renderable) {
            strides[space] += it.size;
        }
    }

    for(auto &it : strides) {
        result.push_back(it);
    }

    VariantList emitterSpawnOperations;
    VariantList emitterUpdateOperations;

    VariantList particleSpawnOperations;
    VariantList particleUpdateOperations;

    VariantList renderOperations;

    VariantList renderables;

    for(const auto it : getChildren()) {
        EffectModule *module = static_cast<EffectModule *>(it);
        if(!module->enabled()) {
            continue;
        }

        VariantList data = module->saveData();

        switch(module->stage()) {
            case EffectModule::EmitterSpawn: {
                emitterSpawnOperations.insert(emitterSpawnOperations.end(), data.begin(), data.end());
            } break;
            case EffectModule::EmitterUpdate: {
                emitterUpdateOperations.insert(emitterUpdateOperations.end(), data.begin(), data.end());
            } break;
            case EffectModule::ParticleSpawn: {
                particleSpawnOperations.insert(particleSpawnOperations.end(), data.begin(), data.end());
            } break;
            case EffectModule::ParticleUpdate: {
                particleUpdateOperations.insert(particleUpdateOperations.end(), data.begin(), data.end());
            } break;
            case EffectModule::Render: {
                renderOperations.insert(renderOperations.end(), data.begin(), data.end());
            } break;
            default: break;
        }

        RenderableModule *renderable = dynamic_cast<RenderableModule *>(module);
        if(renderable) {
            VariantList rend;

            rend.push_back(renderable->type());
            rend.push_back(Engine::reference(renderable->mesh()));
            rend.push_back(Engine::reference(renderable->material()));

            renderables.push_back(rend);
        }
    }

    result.push_back(renderables);

    result.push_back(emitterSpawnOperations);
    result.push_back(emitterUpdateOperations);

    result.push_back(particleSpawnOperations);
    result.push_back(particleUpdateOperations);

    result.push_back(renderOperations);

    return result;
}

void EffectRootNode::setGpu(bool value) {
    if(m_gpu != value) {
        m_gpu = value;

        EffectGraph *g = static_cast<EffectGraph *>(graph());
        g->emitSignal(_SIGNAL(effectUpdated()));
    }
}

void EffectRootNode::setLocal(bool value) {
    if(m_local != value) {
        m_local = value;

        EffectGraph *g = static_cast<EffectGraph *>(graph());
        g->emitSignal(_SIGNAL(effectUpdated()));
    }
}

void EffectRootNode::setContinuous(bool value) {
    if(m_continuous != value) {
        m_continuous = value;

        EffectGraph *g = static_cast<EffectGraph *>(graph());
        g->emitSignal(_SIGNAL(effectUpdated()));
    }
}

void EffectRootNode::setCapacity(int value) {
    if(m_capacity != value) {
        m_capacity = value;

        EffectGraph *g = static_cast<EffectGraph *>(graph());
        g->emitSignal(_SIGNAL(effectUpdated()));
    }
}

EffectModule *EffectRootNode::insertModule(const TString &type, int index) {
    EffectModule *module = dynamic_cast<EffectModule *>(Engine::objectCreate(type));
    CustomModule *custom = nullptr;
    if(module == nullptr) {
        custom = Engine::objectCreate<CustomModule>();
        module = custom;
    }

    module->setRoot(this);

    if(custom) {
        custom->load(static_cast<EffectGraph *>(m_graph)->modulePath(type));
    }

    if(index == -1) {
        int stage = module->stage();
        for(auto &it : getChildren()) {
            EffectModule *m = dynamic_cast<EffectModule *>(it);
            if(m && m->stage() <= stage) {
                index++;
            }
        }
    }

    module->setParent(this, index + 1);

    if(m_nodeWidget) {
        Widget *widget = module->widget(m_nodeWidget->actor());

        switch(module->stage()) {
            case EffectModule::EmitterUpdate: {
                m_emitterUpdateFold->insertWidget(index, widget);
            } break;
            case EffectModule::ParticleSpawn: {
                m_particleSpawnFold->insertWidget(index, widget);
            } break;
            case EffectModule::ParticleUpdate: {
                m_particleUpdateFold->insertWidget(index, widget);
            } break;
            case EffectModule::Render: {
                m_renderFold->insertWidget(index, widget);
            } break;
            default: break;
        }
    }

    return module;
}

int EffectRootNode::moduleIndex(EffectModule *module) {
    int result = -1;
    for(auto it : getChildren()) {
        result++;
        if(it == module) {
            break;
        }
    }
    return result;
}

void EffectRootNode::removeModule(EffectModule *module) {
    Widget *widget = module->widget(nullptr);
    delete widget->actor();

    delete module;
}

void EffectRootNode::removeAllModules() {
    for(auto it : getChildren()) {
        Widget *widget = static_cast<EffectModule *>(it)->widget(nullptr);
        delete widget->actor();

        delete it;
    }
}
