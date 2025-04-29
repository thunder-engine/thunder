#include "effectbuilder.h"

#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <bson.h>

#include <actor.h>
#include <effectrender.h>

#include <mesh.h>
#include <material.h>
#include <visualeffect.h>

#include "effectrootnode.h"
#include "effectmodule.h"

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

#define FORMAT_VERSION 10

EffectBuilderSettings::EffectBuilderSettings() :
        m_thumbnailWarmup(1.0f) {
    setType(MetaType::type<VisualEffect *>());
    setVersion(FORMAT_VERSION);
}

QString EffectBuilderSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/effect.svg";
}

float EffectBuilderSettings::thumbnailWarmup() const {
    return m_thumbnailWarmup;
}
void EffectBuilderSettings::setThumbnailWarmup(float value) {
    if(m_thumbnailWarmup != value) {
        m_thumbnailWarmup = value;
        emit updated();
    }
}

EffectBuilder::EffectBuilder() {
    connect(&m_graph, &EffectGraph::effectUpdated, this, &EffectBuilder::effectUpdated);
}

int EffectBuilder::version() {
    return FORMAT_VERSION;
}

AssetConverter::ReturnCode EffectBuilder::convertFile(AssetConverterSettings *settings) {
    QFileInfo info(settings->source());
    if(info.suffix() == "efx" && settings->currentVersion() == 2) {
        convertOld(settings->source());
        m_graph.save(settings->source());
    } else {
        m_graph.load(settings->source());
    }

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( m_graph.object() );
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();

        return Success;
    }
    return InternalError;
}

AssetConverterSettings *EffectBuilder::createSettings() {
    return new EffectBuilderSettings();
}

Actor *EffectBuilder::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    const EffectBuilderSettings *s = static_cast<const EffectBuilderSettings *>(settings);
    Actor *actor = Engine::composeActor(gEffectRender, "");
    EffectRender *effect = static_cast<EffectRender *>(actor->component(gEffectRender));
    if(effect) {
        effect->setEffect(Engine::loadResource<VisualEffect>(guid.toStdString()));
        float warmup = s->thumbnailWarmup();
        const float frameStep = 1.0f / 60.0f;

        while(warmup > 0.0f) {
            effect->deltaUpdate(frameStep);
            warmup -= frameStep;
        }
    }
    return actor;
}

void EffectBuilder::convertOld(const QString &path) {
    QFile loadFile(path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    m_graph.onNodesLoaded();
    EffectRootNode *root = m_graph.rootNode();
    root->removeAllModules();

    QJsonDocument doc(QJsonDocument::fromJson(loadFile.readAll()));

    QJsonArray nodes = doc.array();
    for(int i = 0; i < nodes.size(); ++i) {
        QJsonObject emitter = nodes[i].toObject();

        root->setObjectName(emitter[gName].toString());

        root->setGpu(emitter[gGpu].toBool());
        root->setLocal(emitter[gLocal].toBool());
        root->setContinuous(emitter[gContinuous].toBool());
        root->setSpawnRate(static_cast<float>(emitter[gDistribution].toDouble()));

        QJsonValue capacity = emitter.value(gCapacity);
        if(!capacity.isUndefined()) {
            root->setCapacity(capacity.toInt());
        }

        const static QMap<QString, QString> modulesMaper = {
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

        const static QMap<QString, QString> parametersMaper = {
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

        QMap<std::string, EffectModule *> modules;

        QJsonArray functions = emitter[gFunctions].toArray();
        for(int m = 0; m < functions.size(); ++m) {
            QJsonObject function = functions[m].toObject();

            QString origin = function[gClass].toString();

            QString classType = modulesMaper.value(origin);

            EffectModule *module = modules.value(classType.toStdString());
            if(module == nullptr) {
                module = root->addModule(m_graph.modulePath(classType).toStdString());
            }

            if(module) {
                modules[classType.toStdString()] = module;

                QString param = parametersMaper.value(origin);
                EffectModule::ParameterData *data = module->parameter(param.toStdString());
                if(data) {
                    int type = function[gType].toInt();

                    if(type >= Constant) {
                        data->mode.current = "Constant";

                        QJsonObject min = function[gMin].toObject();
                        QJsonArray minValue = min["Vector4"].toArray();
                        Vector4 v(static_cast<float>(minValue.at(0).toDouble()),
                                  static_cast<float>(minValue.at(1).toDouble()),
                                  static_cast<float>(minValue.at(2).toDouble()),
                                  static_cast<float>(minValue.at(3).toDouble()));

                        if(data->min.canConvert<Vector2>()) {
                            data->min = QVariant::fromValue(Vector2(v.x, v.y));
                        } else if(data->min.canConvert<Vector3>()) {
                            data->min = QVariant::fromValue(Vector3(v.x, v.y, v.z));
                        } else if(data->min.canConvert<Vector4>()) {
                            data->min = QVariant::fromValue(v);
                        } else {
                            data->min = v.x;
                        }

                        if(type == Random) {
                            data->mode.current = "Random";

                            QJsonObject max = function[gMax].toObject();
                            QJsonArray maxValue = max["Vector4"].toArray();
                            Vector4 v(static_cast<float>(maxValue.at(0).toDouble()),
                                      static_cast<float>(maxValue.at(1).toDouble()),
                                      static_cast<float>(maxValue.at(2).toDouble()),
                                      static_cast<float>(maxValue.at(3).toDouble()));

                            if(data->max.canConvert<Vector2>()) {
                                data->max = QVariant::fromValue(Vector2(v.x, v.y));
                            } else if(data->max.canConvert<Vector3>()) {
                                data->max = QVariant::fromValue(Vector3(v.x, v.y, v.z));
                            } else if(data->max.canConvert<Vector4>()) {
                                data->max = QVariant::fromValue(v);
                            } else {
                                data->max = v.x;
                            }
                        }
                    }
                }

                module->setRoot(root);
            }
        }

        root->addModule(m_graph.modulePath("UpdateState").toStdString());
        root->addModule(m_graph.modulePath("ResolveVelocity").toStdString());
        EffectModule *render = root->addModule(m_graph.modulePath("SpriteRender").toStdString());
        if(render) {
            EffectModule::ParameterData *data = render->parameter("material");
            if(data) {
                data->min = QVariant::fromValue(Template(emitter[gMaterial].toString(), MetaType::name<Material>()));
            }

            data = render->parameter("mesh");
            if(data) {
                data->min = QVariant::fromValue(Template(emitter[gMesh].toString(), MetaType::name<Mesh>()));
            }

            render->setRoot(root);
        }
    }
}
