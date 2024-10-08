#ifndef TILEDSETCONVERTER_H
#define TILEDSETCONVERTER_H

#include <editor/assetconverter.h>

class TiledSetConverterSettings : public AssetConverterSettings {
public:
    TiledSetConverterSettings();

private:
    QString defaultIcon(QString) const override;

};

class TiledSetConverter : public AssetConverter {
    QStringList suffixes() const override { return {"tsx"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;

    ReturnCode convertTileSet(AssetConverterSettings *settings);

};

#endif // TILEDSETCONVERTER_H
