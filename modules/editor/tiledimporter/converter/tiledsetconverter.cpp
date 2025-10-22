#include "tiledsetconverter.h"

#include "tiledmapconverter.h"

#include <cstring>

#include <bson.h>
#include <file.h>

#include <tilemap.h>
#include <sprite.h>

#include <projectsettings.h>

#define FORMAT_VERSION 1

TiledSetConverterSettings::TiledSetConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList TiledSetConverterSettings::typeNames() const {
    return { MetaType::name<TileSet>() };
}

void TiledSetConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/tileset.svg");
    }
}

AssetConverter::ReturnCode TiledSetConverter::convertFile(AssetConverterSettings *settings) {
    File file(settings->source());
    if(file.open(File::ReadOnly)) {
        TString buffer(file.readAll());
        file.close();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

        if(result) {
            pugi::xml_node ts = doc.document_element();
            if(std::string(ts.name()) == "tileset") {
                TileSet *tileSet = Engine::loadResource<TileSet>(settings->destination());
                if(tileSet == nullptr) {
                    tileSet = Engine::objectCreate<TileSet>(settings->destination());
                }

                TiledMapConverter::parseTileset(ts, settings->source(), *tileSet);

                settings->info().id = tileSet->uuid();

                return settings->saveBinary(Engine::toVariant(tileSet), settings->absoluteDestination());
            }
        }
    }

    return InternalError;
}

AssetConverterSettings *TiledSetConverter::createSettings() {
    return new TiledSetConverterSettings();
}
