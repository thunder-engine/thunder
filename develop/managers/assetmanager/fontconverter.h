#ifndef FONTCONVERTER_H
#define FONTCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/font.h>

class FontConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"ttf", "otf"}; }
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // FONTCONVERTER_H
