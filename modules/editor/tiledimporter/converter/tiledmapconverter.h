#ifndef TILEDMAPCONVERTER_H
#define TILEDMAPCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/tileset.h>
#include <resources/tilemap.h>

#include <pugixml.hpp>

class TiledMapConverterSettings : public AssetConverterSettings {
public:
    TiledMapConverterSettings();

private:
    StringList typeNames() const override;

    TString defaultIconPath(const TString &) const override;

};

class TiledMapConverter : public AssetConverter {
public:
    static void parseTileset(const pugi::xml_node &parent, const QString &path, TileSet &tileSet);
    static void parseLayer(const pugi::xml_node &parent, int tileOffset, TileMap &tileMap);

private:
    StringList suffixes() const override { return {"tmx"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;
    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;

    ReturnCode convertTileSet(AssetConverterSettings *settings);
    ReturnCode convertTileMap(AssetConverterSettings *settings);

};

#endif // TILEDMAPCONVERTER_H
