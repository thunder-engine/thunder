#include "tiledsetconverter.h"

#include "tiledmapconverter.h"

#include <QFileInfo>

#include <cstring>

#include <bson.h>
#include <file.h>

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
    File file(settings->source());
    if(file.open(QIODevice::ReadOnly)) {
        TString buffer(file.readAll());
        file.close();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

        if(result) {
            pugi::xml_node ts = doc.document_element();
            if(std::string(ts.name()) == "tileset") {
                TileSet *tileSet = Engine::objectCreate<TileSet>();
                TiledMapConverter::parseTileset(ts, QFileInfo(settings->source().data()).path(), *tileSet);

                return settings->saveBinary(Engine::toVariant(tileSet));
            }
        }
    }

    return InternalError;
}

AssetConverterSettings *TiledSetConverter::createSettings() {
    return new TiledSetConverterSettings();
}
