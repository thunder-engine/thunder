#ifndef CONTROLSCHEMECONVERTER_H
#define CONTROLSCHEMECONVERTER_H

#include <editor/assetconverter.h>
#include <resources/controlscheme.h>

class ControlScehemeConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"controlscheme"}; }
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Control_Scheme.controlscheme"; }
};

#endif //CONTROLSCHEMECONVERTER_H
