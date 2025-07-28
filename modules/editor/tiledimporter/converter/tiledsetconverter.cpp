#include "tiledsetconverter.h"

#include "tiledmapconverter.h"

#include <QFileInfo>
#include <QUuid>
#include <QDir>

#include <QDomDocument>

#include <cstring>

#include <bson.h>
#include <tilemap.h>
#include <sprite.h>

#include <projectsettings.h>

#define FORMAT_VERSION 1

TiledSetConverterSettings::TiledSetConverterSettings() {
    setType(MetaType::type<TileSet *>());
    setVersion(FORMAT_VERSION);
}

TString TiledSetConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/tileset.svg";
}

AssetConverter::ReturnCode TiledSetConverter::convertFile(AssetConverterSettings *settings) {
    QFile file(settings->source().data());
    if(file.open(QIODevice::ReadOnly)) {
        QDomDocument doc;
        if(doc.setContent(&file)) {

            QDomElement ts = doc.documentElement();
            if(ts.nodeName() == "tileset") {
                TileSet tileSet;
                TiledMapConverter::parseTileset(ts, QFileInfo(settings->source().data()).path(), tileSet);

                QFile file(settings->absoluteDestination().data());
                if(file.open(QIODevice::WriteOnly)) {
                    ByteArray data = Bson::save( Engine::toVariant(&tileSet) );
                    file.write(reinterpret_cast<const char *>(data.data()), data.size());
                    file.close();

                    return Success;
                }
            }
        }
    }

    return InternalError;
}

AssetConverterSettings *TiledSetConverter::createSettings() {
    return new TiledSetConverterSettings();
}


