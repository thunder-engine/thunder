#ifndef CONTROLSCHEMECONVERTER_H
#define CONTROLSCHEMECONVERTER_H

#include <editor/assetconverter.h>
#include <resources/controlscheme.h>

class ControlScehemeConverterSettings : public AssetConverterSettings {
public:
    ControlScehemeConverterSettings();
};

class ControlSchemeConverter : public AssetConverter {
    StringList suffixes() const override { return {"controlscheme"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Control_Scheme.controlscheme"; }
};

#endif //CONTROLSCHEMECONVERTER_H
