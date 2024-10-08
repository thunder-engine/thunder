#ifndef PIPELINECONVERTER_H
#define PIPELINECONVERTER_H

#include <editor/assetconverter.h>

class PipelineConverterSettings : public AssetConverterSettings {
public:
    PipelineConverterSettings();

private:
    QString defaultIcon(QString) const override;

};

class PipelineConverter : public AssetConverter {
private:
    QStringList suffixes() const override { return {"pipeline"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;
    AssetConverterSettings *createSettings() override;

};

#endif // PIPELINECONVERTER_H
