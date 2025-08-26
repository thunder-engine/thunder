#ifndef ANIMATIONCONTROLLERBUILDER_H
#define ANIMATIONCONTROLLERBUILDER_H

#include <editor/assetconverter.h>

#include "animationcontrollergraph.h"

class AnimationBuilderSettings : public AssetConverterSettings {
public:
    AnimationBuilderSettings();
private:
    StringList typeNames() const override;

    TString defaultIconPath(const TString &) const override;

};

class AnimationControllerBuilder : public AssetConverter {
public:
    static int version();

private:
    StringList suffixes() const override { return {"actl"}; }

    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Animation_Controller.actl"; }

private:
    AnimationControllerGraph m_model;
};

#endif // ANIMATIONCONTROLLERBUILDER_H
