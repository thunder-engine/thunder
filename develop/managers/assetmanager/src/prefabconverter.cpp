#include "prefabconverter.h"

#include "components/actor.h"

#include <QFile>

#include <bson.h>
#include <json.h>

uint8_t PrefabConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        Variant actor = Json::load(src.readAll().toStdString());
        src.close();

        Actor *prefab = static_cast<Actor *>(Engine::toObject(actor));
        Prefab *fab = Engine::objectCreate<Prefab>("");
        fab->setActor(prefab);

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(fab));
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }
    return 1;
}
