#ifndef PIPELINECONVERTER_H
#define PIPELINECONVERTER_H

#include <editor/assetconverter.h>

class PipelineConverterSettings : public AssetConverterSettings {
public:
    PipelineConverterSettings();

    static int version();

private:
    StringList typeNames() const override;

};

class PipelineConverter : public AssetConverter {
private:
    void init() override;
    StringList suffixes() const override { return {"pipeline"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;
    AssetConverterSettings *createSettings() override;

};

#endif // PIPELINECONVERTER_H
