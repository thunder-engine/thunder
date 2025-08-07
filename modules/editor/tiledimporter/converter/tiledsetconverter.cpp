#include "tiledsetconverter.h"

#include "tiledmapconverter.h"

#include <QFileInfo>

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
        QByteArray buffer(file.readAll());
        file.close();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

        if(result) {
            pugi::xml_node ts = doc.document_element();
            if(std::string(ts.name()) == "tileset") {
                TileSet *tileSet = Engine::objectCreate<TileSet>();
                TiledMapConverter::parseTileset(ts, QFileInfo(settings->source().data()).path(), *tileSet);

                QFile file(settings->absoluteDestination().data());
                if(file.open(QIODevice::WriteOnly)) {
                    ByteArray data = Bson::save( Engine::toVariant(tileSet) );
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
