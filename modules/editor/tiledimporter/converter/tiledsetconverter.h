#ifndef TILEDSETCONVERTER_H
#define TILEDSETCONVERTER_H

#include <editor/assetconverter.h>

class TiledSetConverterSettings : public AssetConverterSettings {
public:
    TiledSetConverterSettings();

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class TiledSetConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"tsx"}; }
    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    ReturnCode convertTileSet(AssetConverterSettings *settings);

};

#endif // TILEDSETCONVERTER_H
