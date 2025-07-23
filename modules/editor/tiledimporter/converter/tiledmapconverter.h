#ifndef TILEDMAPCONVERTER_H
#define TILEDMAPCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/tileset.h>
#include <resources/tilemap.h>

class QDomElement;

class TiledMapConverterSettings : public AssetConverterSettings {
public:
    TiledMapConverterSettings();

private:
    QString defaultIconPath(const QString &) const override;

};

class TiledMapConverter : public AssetConverter {
public:
    static void parseTileset(const QDomElement &element, const QString &path, TileSet &tileSet);
    static void parseLayer(const QDomElement &element, int tileOffset, TileMap &tileMap);

private:
    StringList suffixes() const override { return {"tmx"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;
    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;

    ReturnCode convertTileSet(AssetConverterSettings *settings);
    ReturnCode convertTileMap(AssetConverterSettings *settings);

};

#endif // TILEDMAPCONVERTER_H
