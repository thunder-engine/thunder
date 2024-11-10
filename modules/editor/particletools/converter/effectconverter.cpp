#include "effectconverter.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <bson.h>

#include <actor.h>
#include <effectrender.h>

#include <mesh.h>
#include <material.h>
#include <particleeffect.h>

#include "effectbuilder.h"
#include "effectrootnode.h"
#include "effectmodule.h"

namespace {
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

#define FORMAT_VERSION 2

EffectConverter::EffectConverter() {

}

AssetConverter::ReturnCode EffectConverter::convertFile(AssetConverterSettings *settings) {
    load(settings->source());

    if(settings->currentVersion() != FORMAT_VERSION) {
        //save(settings->source());
    }

    return InternalError;
}

AssetConverterSettings *EffectConverter::createSettings() {
    return new EffectBuilderSettings();
}

void EffectConverter::load(const QString &path) {
    foreach(QObject *it, children()) {
        delete it;
    }

    QFile loadFile(path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    QJsonDocument doc(QJsonDocument::fromJson(loadFile.readAll()));

    QJsonArray nodes = doc.array();
    for(int i = 0; i < nodes.size(); ++i) {
        QJsonObject emitter = nodes[i].toObject();

        EffectEmitter *obj = new EffectEmitter(this);
        obj->blockSignals(true);
        obj->setObjectName(emitter[gName].toString());

        obj->setMeshPath(emitter[gMesh].toString());
        obj->setMaterialPath(emitter[gMaterial].toString());

        QJsonValue capacity = emitter.value(gCapacity);
        if(!capacity.isUndefined()) {
            obj->setCapacity(capacity.toInt());
        }

        obj->setDistribution(static_cast<float>(emitter[gDistribution].toDouble()));

        obj->setGpu(emitter[gGpu].toBool());
        obj->setLocal(emitter[gLocal].toBool());
        obj->setContinuous(emitter[gContinuous].toBool());

        connect(obj, &EffectEmitter::updated, this, &EffectConverter::effectUpdated);

        obj->blockSignals(false);

        QJsonArray functions = emitter[gFunctions].toArray();
        for(int m = 0; m < functions.size(); ++m) {
            QJsonObject function = functions[m].toObject();

            QString meta = function[gClass].toString();

            EffectModule *module = createModule(meta);
            if(module) {
                module->blockSignals(true);
                module->setParent(obj);
                module->setObjectName(meta);

                int type = function[gType].toInt();
                //module->setMode(type);

                if(type <= EffectModule::Random) {
                    //QJsonObject min = function[gMin].toObject();
                    //QJsonArray minValue = min["Vector4"].toArray();
                    //module->setMin(Vector4(static_cast<float>(minValue.at(0).toDouble()),
                    //                       static_cast<float>(minValue.at(1).toDouble()),
                    //                       static_cast<float>(minValue.at(2).toDouble()),
                    //                       static_cast<float>(minValue.at(3).toDouble())));

                    if(type == EffectModule::Random) {
                        QJsonObject max = function[gMax].toObject();
                        QJsonArray maxValue = max["Vector4"].toArray();
                        //module->setMax(Vector4(static_cast<float>(maxValue.at(0).toDouble()),
                        //                       static_cast<float>(maxValue.at(1).toDouble()),
                        //                       static_cast<float>(maxValue.at(2).toDouble()),
                        //                       static_cast<float>(maxValue.at(3).toDouble())));
                    }
                }

                module->blockSignals(false);
            }
        }
    }
}

EffectModule *EffectConverter::createModule(const QString &path) {
    const int type = QMetaType::type( qPrintable(path + "*") );
    const QMetaObject *meta = QMetaType::metaObjectForType(type);
    if(meta) {
        QObject *object = meta->newInstance();
        EffectModule *module = dynamic_cast<EffectModule *>(object);
        if(module) {
            connect(module, SIGNAL(updated()), this, SIGNAL(effectUpdated()));
            return module;
        }
    }
    return nullptr;
}
