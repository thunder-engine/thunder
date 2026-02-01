#ifndef ANIMATIONCONTROLLERBUILDER_H
#define ANIMATIONCONTROLLERBUILDER_H

#include <editor/assetconverter.h>

#include "animationcontrollergraph.h"

class AnimationBuilderSettings : public AssetConverterSettings {
    A_OBJECT(AnimationBuilderSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTYEX(TString, previewAsset, AnimationBuilderSettings::previewAsset, AnimationBuilderSettings::setPreviewAsset, "editor=Asset,type=Prefab")
    )

public:
    AnimationBuilderSettings();

    TString previewAsset() const;
    void setPreviewAsset(TString &asset);

private:
    StringList typeNames() const override;

private:
    TString m_previewAsset;

};

class AnimationControllerBuilder : public AssetConverter {
public:
    static int version();

private:
    void init() override;

    StringList suffixes() const override { return {"actl"}; }

    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Animation_Controller.actl"; }

private:
    AnimationControllerGraph m_model;
};

#endif // ANIMATIONCONTROLLERBUILDER_H
