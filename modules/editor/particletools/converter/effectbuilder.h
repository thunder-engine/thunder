#ifndef EFFECTBUILDER_H
#define EFFECTBUILDER_H

#include <editor/assetconverter.h>

#include "effectgraph.h"

class EffectBuilderSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(float thumbnailWarmup READ thumbnailWarmup WRITE setThumbnailWarmup DESIGNABLE true USER true)

public:
    EffectBuilderSettings();

    float thumbnailWarmup() const;
    void setThumbnailWarmup(float value);

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

private:
    float m_thumbnailWarmup;

};

class EffectBuilder : public AssetConverter {
    Q_OBJECT

public:
    EffectBuilder();

    static int version();

    QStringList suffixes() const override { return {"vfx", "efx"}; }

    EffectGraph &graph() { return m_graph; }

protected:
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const override;

    QString templatePath() const override { return ":/templates/VisualEffect.vfx"; }

    void convertOld(const QString &path);

signals:
    void effectUpdated();

private:
    EffectGraph m_graph;

};

#endif // EFFECTBUILDER_H
