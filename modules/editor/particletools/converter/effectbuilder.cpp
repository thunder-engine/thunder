#include "effectbuilder.h"

#include <actor.h>
#include <effectrender.h>

#include <mesh.h>
#include <material.h>
#include <visualeffect.h>

#include <url.h>
#include <file.h>
#include <json.h>

#include "effectrootnode.h"
#include "modules/effectmodule.h"

namespace  {
    // Old file format
    const char *gName("Name");
    const char *gMesh("Mesh");
    const char *gMaterial("Material");

    const char *gContinuous("Continuous");
    const char *gLocal("Local");
    const char *gGpu("Gpu");
    const char *gCapacity("Capacity");
    const char *gDistribution("Distribution");
    const char *gFunctions("Functions");
    const char *gClass("Class");
    const char *gType("Type");
    const char *gMin("Min");
    const char *gMax("Max");
};

#define FORMAT_VERSION 11

EffectBuilderSettings::EffectBuilderSettings() :
        m_thumbnailWarmup(1.0f) {
    setVersion(FORMAT_VERSION);
}

StringList EffectBuilderSettings::typeNames() const {
    return { MetaType::name<VisualEffect>() };
}

float EffectBuilderSettings::thumbnailWarmup() const {
    return m_thumbnailWarmup;
}
void EffectBuilderSettings::setThumbnailWarmup(float value) {
    if(m_thumbnailWarmup != value) {
        m_thumbnailWarmup = value;
        setModified();
    }
}

EffectBuilder::EffectBuilder() {

}

int EffectBuilder::version() {
    return FORMAT_VERSION;
}

void EffectBuilder::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/effect.svg");
    }
}

AssetConverter::ReturnCode EffectBuilder::convertFile(AssetConverterSettings *settings) {
    Url info(settings->source());
    if(info.suffix() == "efx" && settings->currentVersion() == 2) {
        convertOld(settings->source());
        m_graph.save(settings->source());
    } else {
        m_graph.load(settings->source());
    }

    uint32_t uuid = settings->info().id;
    if(uuid == 0) {
        uuid = Engine::generateUUID();
        settings->info().id = uuid;
    }

    VariantList result;

    VariantList object;

    object.push_back(VisualEffect::metaClass()->name()); // type
    object.push_back(uuid); // id
    object.push_back(0); // parent
    object.push_back(settings->destination()); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(m_graph.data()); // user data

    result.push_back(object);

    return settings->saveBinary(result, settings->absoluteDestination());
}

AssetConverterSettings *EffectBuilder::createSettings() {
    return new EffectBuilderSettings();
}

Actor *EffectBuilder::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    Actor *actor = Engine::composeActor<EffectRender>("");
    EffectRender *effect = actor->getComponent<EffectRender>();
    if(effect) {
        const EffectBuilderSettings *effectSettings = static_cast<const EffectBuilderSettings *>(settings);
        effect->setEffect(Engine::loadResource<VisualEffect>(guid));
        float warmup = effectSettings->thumbnailWarmup();
        const float frameStep = 1.0f / 60.0f;

        while(warmup > 0.0f) {
            effect->deltaUpdate(frameStep);
            warmup -= frameStep;
        }
    }
    return actor;
}

void EffectBuilder::convertOld(const TString &path) {
    File loadFile(path.data());
    if(!loadFile.open(File::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    m_graph.onNodesLoaded();
    EffectRootNode *root = static_cast<EffectRootNode *>(m_graph.defaultNode());
    root->removeAllModules();

    Variant doc = Json::load(loadFile.readAll());

    for(auto &node : doc.toList()) {
        VariantMap emitter = node.toMap();

        root->setName(emitter[gName].toString().toStdString());

        root->setGpu(emitter[gGpu].toBool());
        root->setLocal(emitter[gLocal].toBool());
        root->setContinuous(emitter[gContinuous].toBool());

        auto it = emitter.find(gCapacity);
        if(it != emitter.end()) {
            root->setCapacity(it->second.toInt());
        }

        const static std::map<std::string, std::string> modulesMaper = {
            {"Lifetime", "InitializeParticle"},
            {"StartPosition", "InitializeParticle"},
            {"StartSize", "InitializeParticle"},
            {"StartColor", "InitializeParticle"},
            {"StartAngle", "InitializeParticle"},

            {"ScaleSize", "SizeScale"},
            {"ScaleColor", "ColorScale"},
            {"ScaleAngle", "RotationRate"},
            {"Velocity", "AddVelocity"}
        };

        const static std::map<std::string, std::string> parametersMaper = {
            {"Lifetime", "lifetime"},
            {"StartPosition", "position"},
            {"StartSize", "size"},
            {"StartColor", "color"},
            {"StartAngle", "rotation"},

            {"ScaleSize", "sizeScale"},
            {"ScaleColor", "colorScale"},
            {"ScaleAngle", "rotationRate"},
            {"Velocity", "velocitySpeed"},
        };

        enum Types {
            Constant,
            Random
        };

        std::map<std::string, EffectModule *> modules;

        for(auto &functionIt : emitter[gFunctions].toList()) {
            VariantMap function = functionIt.toMap();

            std::string origin = function[gClass].toString().toStdString();

            auto modulesMaperIt = modulesMaper.find(origin);
            std::string classType = modulesMaperIt->second;

            EffectModule *module = modules[classType];
            if(module == nullptr) {
                module = root->insertModule(m_graph.modulePath(classType));
            }

            if(module) {
                modules[classType] = module;

                auto parametersMaperIt = parametersMaper.find(origin);
                std::string param = parametersMaperIt->second;
                EffectRootNode::ParameterData *data = root->parameter(param, module);
                if(data) {
                    if(function[gType].toInt() == Constant) {
                        data->mode = EffectModule::Constant;
                    } else {
                        data->mode = EffectModule::Random;
                    }

                    if(data->mode >= EffectModule::Constant) {
                        VariantMap min = function[gMin].toMap();

                        Vector4 v;
                        int i = 0;
                        for(auto &minIt : min["Vector4"].toList()) {
                            v[i] = minIt.toFloat();
                            i++;
                        }

                        if(data->min.canConvert<Vector2>()) {
                            data->min = Vector2(v.x, v.y);
                        } else if(data->min.canConvert<Vector3>()) {
                            data->min = Vector3(v.x, v.y, v.z);
                        } else if(data->min.canConvert<Vector4>()) {
                            data->min = v;
                        } else {
                            data->min = v.x;
                        }

                        if(data->mode == EffectModule::Random) {
                            VariantMap max = function[gMax].toMap();

                            Vector4 v;
                            int i = 0;
                            for(auto &minIt : min["Vector4"].toList()) {
                                v[i] = minIt.toFloat();
                                i++;
                            }

                            if(data->max.canConvert<Vector2>()) {
                                data->max = Vector2(v.x, v.y);
                            } else if(data->max.canConvert<Vector3>()) {
                                data->max = Vector3(v.x, v.y, v.z);
                            } else if(data->max.canConvert<Vector4>()) {
                                data->max = v;
                            } else {
                                data->max = v.x;
                            }
                        }
                    }
                }

                module->setRoot(root);
            }
        }

        root->insertModule(m_graph.modulePath("UpdateState"));
        root->insertModule(m_graph.modulePath("ResolveVelocity"));
        EffectModule *render = root->insertModule(m_graph.modulePath("SpriteRender"));
        if(render) {
            EffectRootNode::ParameterData *data = root->parameter("material", render);
            if(data) {
                Resource *object = Engine::loadResource(emitter[gMaterial].toString().toStdString());
                uint32_t type = MetaType::type("Material") + 1;
                data->min = Variant(type, &object);
            }

            data = root->parameter("mesh", render);
            if(data) {
                Resource *object = Engine::loadResource(emitter[gMesh].toString().toStdString());
                uint32_t type = MetaType::type("Mesh") + 1;
                data->min = Variant(type, &object);
            }

            render->setRoot(root);
        }
    }
}
