#ifndef EFFECTBUILDER_H
#define EFFECTBUILDER_H

#include <editor/assetconverter.h>

#include "effectgraph.h"

class EffectBuilderSettings : public AssetConverterSettings {
    A_OBJECT(EffectBuilderSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTY(float, thumbnailWarmup, EffectBuilderSettings::thumbnailWarmup, EffectBuilderSettings::setThumbnailWarmup)
    )

public:
    EffectBuilderSettings();

    float thumbnailWarmup() const;
    void setThumbnailWarmup(float value);

private:
    StringList typeNames() const override;

    TString defaultIconPath(const TString &) const override;

private:
    float m_thumbnailWarmup;

};

class EffectBuilder : public AssetConverter {
public:
    EffectBuilder();

    static int version();

    StringList suffixes() const override { return {"vfx", "efx"}; }

    EffectGraph &graph() { return m_graph; }

protected:
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;

    TString templatePath() const override { return ":/templates/VisualEffect.vfx"; }

    void convertOld(const TString &path);

private:
    EffectGraph m_graph;

};

#endif // EFFECTBUILDER_H
