#ifndef PIPELINECONVERTER_H
#define PIPELINECONVERTER_H

#include <editor/assetconverter.h>

class PipelineConverterSettings : public AssetConverterSettings {
public:
    PipelineConverterSettings();

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class PipelineConverter : public AssetConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"pipeline"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

};

#endif // PIPELINECONVERTER_H
