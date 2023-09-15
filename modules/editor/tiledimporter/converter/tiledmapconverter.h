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
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class TiledMapConverter : public AssetConverter {
public:
    static void parseTileset(const QDomElement &element, const QString &path, TileSet &tileSet);
    static void parseLayer(const QDomElement &element, int tileOffset, TileMap &tileMap);

private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"tmx"}; }
    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;

    ReturnCode convertTileSet(AssetConverterSettings *settings);
    ReturnCode convertTileMap(AssetConverterSettings *settings);

};

#endif // TILEDMAPCONVERTER_H
