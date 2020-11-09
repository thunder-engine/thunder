#include "effectconverter.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <bson.h>
#include <json.h>

#include <components/actor.h>
#include <components/particlerender.h>

#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/particleeffect.h>

#define NAME        "Name"
#define MESH        "Mesh"
#define MATERIAL    "Material"
#define CONTINUOUS  "Continuous"
#define LOCAL       "Local"
#define GPU         "Gpu"
#define DISTRIBUTION "Distribution"
#define FUNCTIONS   "Functions"
#define CLASS       "Class"
#define TYPE        "Type"
#define MINIMUM     "Min"
#define MAXIMUM     "Max"

#define EMITTERS    "Emitters"

EffectConverterSettings::EffectConverterSettings() {
    setType(MetaType::type<ParticleEffect *>());
}

EffectConverter::EffectConverter() {
    qRegisterMetaType<EffectEmitter*>("EffectEmitter*");

    qRegisterMetaType<Lifetime*>("Lifetime");
    qRegisterMetaType<StartSize*>("StartSize");
    qRegisterMetaType<StartColor*>("StartColor");
    qRegisterMetaType<StartAngle*>("StartAngle");
    qRegisterMetaType<StartPosition*>("StartPosition");

    qRegisterMetaType<ScaleSize*>("ScaleSize");
    qRegisterMetaType<ScaleColor*>("ScaleColor");
    qRegisterMetaType<ScaleAngle*>("ScaleAngle");
    qRegisterMetaType<Velocity*>("Velocity");
}

uint8_t EffectConverter::convertFile(IConverterSettings *settings) {
    load(settings->source());

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data  = Bson::save( object() );
        file.write(reinterpret_cast<const char *>(&data[0]), data.size());
        file.close();
        return 0;
    }

    return 1;
}

IConverterSettings *EffectConverter::createSettings() const {
    return new EffectConverterSettings();
}

Actor *EffectConverter::createActor(const QString &guid) const {
    Actor *actor = Engine::objectCreate<Actor>("");
    ParticleRender *effect = static_cast<ParticleRender *>(actor->addComponent("ParticleRender"));
    if(effect) {
        effect->setEffect(Engine::loadResource<ParticleEffect>(guid.toStdString()));
    }
    return actor;
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

    QByteArray saveData = loadFile.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(saveData));

    QJsonArray nodes = doc.array();
    for(int i = 0; i < nodes.size(); ++i) {
        QJsonObject emitter = nodes[i].toObject();

        EffectEmitter *obj = new EffectEmitter(this);
        obj->blockSignals(true);
        obj->setObjectName(emitter[NAME].toString());

        obj->setMeshPath(emitter[MESH].toString());
        obj->setMaterialPath(emitter[MATERIAL].toString());

        obj->setDistribution(static_cast<float>(emitter[DISTRIBUTION].toDouble()));

        obj->setGpu(emitter[GPU].toBool());
        obj->setLocal(emitter[LOCAL].toBool());
        obj->setContinuous(emitter[CONTINUOUS].toBool());

        connect(obj, &EffectEmitter::updated, this, &EffectConverter::effectUpdated);

        obj->blockSignals(false);

        QJsonArray functions = emitter[FUNCTIONS].toArray();
        for(int m = 0; m < functions.size(); ++m) {
            QJsonObject function = functions[m].toObject();

            QString meta = function[CLASS].toString();

            EffectFunction *mod = createFunction(meta);
            if(mod) {
                mod->blockSignals(true);
                mod->setParent(obj);
                mod->setObjectName(meta);

                mod->setType(static_cast<EffectFunction::ModificatorType>(function[TYPE].toInt()));

                if(mod->type() <= EffectFunction::Range) {
                    {
                        QJsonObject min = function[MINIMUM].toObject();
                        QJsonArray v = min["Vector4"].toArray();
                        mod->setMin(Vector4(static_cast<float>(v.at(0).toDouble()),
                                            static_cast<float>(v.at(1).toDouble()),
                                            static_cast<float>(v.at(2).toDouble()),
                                            static_cast<float>(v.at(3).toDouble())));
                    }
                    if(function[TYPE].toInt() == EffectFunction::Range) {
                        QJsonObject max = function[MAXIMUM].toObject();
                        QJsonArray v = max["Vector4"].toArray();
                        mod->setMax(Vector4(static_cast<float>(v.at(0).toDouble()),
                                            static_cast<float>(v.at(1).toDouble()),
                                            static_cast<float>(v.at(2).toDouble()),
                                            static_cast<float>(v.at(3).toDouble())));
                    }
                } else {

                }
                mod->blockSignals(false);
            }
        }
    }
}

void EffectConverter::save(const QString &path) {
    QFile saveFile(path);
    if(!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    QJsonArray nodes;
    foreach(auto *it, children()) {
        const EffectEmitter *e = dynamic_cast<const EffectEmitter *>(it);
        if(e) {
            QJsonObject emitter;

            emitter[NAME] = e->objectName();
            emitter[MESH] = e->meshPath();
            emitter[MATERIAL] = e->materialPath();

            emitter[DISTRIBUTION] = static_cast<double>(e->distribution());

            emitter[GPU] = e->isGpu();
            emitter[LOCAL] = e->isLocal();
            emitter[CONTINUOUS] = e->isContinuous();


            QJsonArray functions;
            foreach(auto *obj, it->children()) {
                const EffectFunction *mod = dynamic_cast<const EffectFunction *>(obj);
                if(mod) {
                    QJsonObject function;
                    function[CLASS] = mod->metaObject()->className();
                    function[TYPE] = mod->type();
                    if(mod->type() <= EffectFunction::Range) {
                        {
                            QJsonObject min;
                            QJsonArray vector;
                            Vector4 v = mod->min();
                            vector.push_back(static_cast<double>(v.x));
                            vector.push_back(static_cast<double>(v.y));
                            vector.push_back(static_cast<double>(v.z));
                            vector.push_back(static_cast<double>(v.w));

                            min["Vector4"] = vector;
                            function[MINIMUM] = min;
                        }
                        if(mod->type() == EffectFunction::Range) {
                            QJsonObject max;
                            QJsonArray vector;
                            Vector4 v = mod->max();
                            vector.push_back(static_cast<double>(v.x));
                            vector.push_back(static_cast<double>(v.y));
                            vector.push_back(static_cast<double>(v.z));
                            vector.push_back(static_cast<double>(v.w));

                            max["Vector4"] = vector;
                            function[MAXIMUM] = max;
                        }
                    } else {

                    }
                    functions.push_back(function);
                }
            }
            emitter[FUNCTIONS] = functions;
            nodes.push_back(emitter);
        }
    }
    QJsonDocument doc(nodes);
    saveFile.write(doc.toJson());
}

Variant EffectConverter::data() const {
    VariantMap user;

    VariantList emitters;
    foreach(QObject *child, children()) {
        EffectEmitter *e = dynamic_cast<EffectEmitter *>(child);
        if(e) {
            VariantList emitter;

            emitter.push_back( e->meshPath().toStdString() );
            emitter.push_back( e->materialPath().toStdString() );

            emitter.push_back(e->isGpu());
            emitter.push_back(e->isLocal());
            emitter.push_back(e->isContinuous());

            emitter.push_back(e->distribution());

            VariantList modificators;

            foreach(QObject *mod, e->children()) {
                EffectFunction *m = dynamic_cast<EffectFunction *>(mod);
                if(m) {
                    VariantList modificator;

                    int32_t type = m->type();
                    modificator.push_back(m->classType());

                    VariantList data;
                    data.push_back(type);

                    if(type <= EffectFunction::Range) {
                        data.push_back(m->min());
                        if(type == EffectFunction::Range) {
                            data.push_back(m->max());
                        }
                    } else {

                    }
                    modificator.push_back(data);
                    modificators.push_back(modificator);
                }
            }
            emitter.push_back(modificators);
            emitters.push_back(emitter);
        }
    }
    user[EMITTERS] = emitters;

    return user;
}

EffectEmitter *EffectConverter::createEmitter() {
    EffectEmitter *obj = new EffectEmitter(this);
    QString base = "Emitter";
    uint32_t it = 1;
    while(findChild<EffectEmitter *>(base + QString::number(it), Qt::FindDirectChildrenOnly) != nullptr) {
        it++;
    }
    obj->setObjectName(base + QString::number(it));
    obj->setMeshPath(".embedded/sphere.fbx/Sphere001"); // .embedded/plane.fbx/Plane001
    connect(obj, &EffectEmitter::updated, this, &EffectConverter::effectUpdated);
    emit effectUpdated();
    return obj;
}

void EffectConverter::deleteEmitter(QString name) {
    EffectEmitter *emitter  = findChild<EffectEmitter *>(name, Qt::FindDirectChildrenOnly);
    if(emitter) {
        emitter->setParent(nullptr);
        emitter->deleteLater();
        emitter = nullptr;
    }
    emit effectUpdated();
}

EffectFunction *EffectConverter::createFunction(const QString &name, const QString &path) {
    EffectEmitter *emitter  = findChild<EffectEmitter *>(name, Qt::FindDirectChildrenOnly);
    if(emitter) {
        EffectFunction *mod = createFunction(path);
        if(mod) {
            mod->blockSignals(true);

            mod->setParent(emitter);
            mod->setObjectName(path);

            mod->blockSignals(false);

            emit effectUpdated();
            return mod;
        }
    }
    return nullptr;
}

void EffectConverter::deleteFunction(const QString &name, const QString &path) {
    EffectEmitter *emitter  = findChild<EffectEmitter *>(name, Qt::FindDirectChildrenOnly);
    if(emitter) {
        EffectFunction *mod = emitter->findChild<EffectFunction *>(path, Qt::FindDirectChildrenOnly);
        if(mod) {
            mod->setParent(nullptr);
            mod->deleteLater();
            mod = nullptr;
        }
    }
    emit effectUpdated();
}

EffectFunction *EffectConverter::createFunction(const QString &path) {
    const QByteArray className = qPrintable(path + "*");
    const int type = QMetaType::type( className );
    const QMetaObject *meta = QMetaType::metaObjectForType(type);
    if(meta) {
        QObject *object = meta->newInstance();
        EffectFunction *function = dynamic_cast<EffectFunction *>(object);
        if(function) {
            connect(function, SIGNAL(updated()), this, SIGNAL(effectUpdated()));
            return function;
        }
    }
    return nullptr;
}

Variant EffectConverter::object() const {
    VariantList result;

    VariantList object;

    object.push_back(ParticleEffect::metaClass()->name()); // type
    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(ParticleEffect::metaClass()->name()); // name

    object.push_back(VariantMap()); // properties

    object.push_back(VariantList()); // links
    object.push_back(data()); // user data

    result.push_back(object);

    return result;
}

