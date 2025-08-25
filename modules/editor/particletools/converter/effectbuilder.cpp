#include "effectbuilder.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <actor.h>
#include <effectrender.h>

#include <mesh.h>
#include <material.h>
#include <visualeffect.h>

#include "effectrootnode.h"
#include "modules/effectmodule.h"

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

namespace  {
    const char *gEffectRender("EffectRender");

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
    setType(MetaType::type<VisualEffect *>());
    setVersion(FORMAT_VERSION);
}

TString EffectBuilderSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/effect.svg";
}

float EffectBuilderSettings::thumbnailWarmup() const {
    return m_thumbnailWarmup;
}
void EffectBuilderSettings::setThumbnailWarmup(float value) {
    if(m_thumbnailWarmup != value) {
        m_thumbnailWarmup = value;
    }
}

EffectBuilder::EffectBuilder() {

}

int EffectBuilder::version() {
    return FORMAT_VERSION;
}

AssetConverter::ReturnCode EffectBuilder::convertFile(AssetConverterSettings *settings) {
    QFileInfo info(settings->source().data());
    if(info.suffix() == "efx" && settings->currentVersion() == 2) {
        convertOld(settings->source());
        m_graph.save(settings->source());
    } else {
        m_graph.load(settings->source());
    }

    return settings->saveBinary(m_graph.object());
}

AssetConverterSettings *EffectBuilder::createSettings() {
    return new EffectBuilderSettings();
}

Actor *EffectBuilder::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    const EffectBuilderSettings *s = static_cast<const EffectBuilderSettings *>(settings);
    Actor *actor = Engine::composeActor(gEffectRender, "");
    EffectRender *effect = static_cast<EffectRender *>(actor->component(gEffectRender));
    if(effect) {
        effect->setEffect(Engine::loadResource<VisualEffect>(guid));
        float warmup = s->thumbnailWarmup();
        const float frameStep = 1.0f / 60.0f;

        while(warmup > 0.0f) {
            effect->deltaUpdate(frameStep);
            warmup -= frameStep;
        }
    }
    return actor;
}

void EffectBuilder::convertOld(const TString &path) {
    QFile loadFile(path.data());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    m_graph.onNodesLoaded();
    EffectRootNode *root = static_cast<EffectRootNode *>(m_graph.defaultNode());
    root->removeAllModules();

    QJsonDocument doc(QJsonDocument::fromJson(loadFile.readAll()));

    QJsonArray nodes = doc.array();
    for(int i = 0; i < nodes.size(); ++i) {
        QJsonObject emitter = nodes[i].toObject();

        root->setName(emitter[gName].toString().toStdString());

        root->setGpu(emitter[gGpu].toBool());
        root->setLocal(emitter[gLocal].toBool());
        root->setContinuous(emitter[gContinuous].toBool());

        QJsonValue capacity = emitter.value(gCapacity);
        if(!capacity.isUndefined()) {
            root->setCapacity(capacity.toInt());
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

        QJsonArray functions = emitter[gFunctions].toArray();
        for(int m = 0; m < functions.size(); ++m) {
            QJsonObject function = functions[m].toObject();

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
                        QJsonObject min = function[gMin].toObject();
                        QJsonArray minValue = min["Vector4"].toArray();
                        Vector4 v(static_cast<float>(minValue.at(0).toDouble()),
                                  static_cast<float>(minValue.at(1).toDouble()),
                                  static_cast<float>(minValue.at(2).toDouble()),
                                  static_cast<float>(minValue.at(3).toDouble()));

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
                            QJsonObject max = function[gMax].toObject();
                            QJsonArray maxValue = max["Vector4"].toArray();
                            Vector4 v(static_cast<float>(maxValue.at(0).toDouble()),
                                      static_cast<float>(maxValue.at(1).toDouble()),
                                      static_cast<float>(maxValue.at(2).toDouble()),
                                      static_cast<float>(maxValue.at(3).toDouble()));

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
                Object *object = Engine::loadResource(emitter[gMaterial].toString().toStdString());
                uint32_t type = MetaType::type("Material") + 1;
                data->min = Variant(type, &object);
            }

            data = root->parameter("mesh", render);
            if(data) {
                Object *object = Engine::loadResource(emitter[gMesh].toString().toStdString());
                uint32_t type = MetaType::type("Mesh") + 1;
                data->min = Variant(type, &object);
            }

            render->setRoot(root);
        }
    }
}
