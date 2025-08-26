#include "pipelineconverter.h"

#include "pipelinetaskgraph.h"

#include <resources/pipeline.h>

#define FORMAT_VERSION 11

PipelineConverterSettings::PipelineConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList PipelineConverterSettings::typeNames() const {
    return { "Pipeline" };
}

TString PipelineConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/pipeline.svg";
}

int PipelineConverterSettings::version() {
    return FORMAT_VERSION;
}

AssetConverter::ReturnCode PipelineConverter::convertFile(AssetConverterSettings *settings) {
    VariantMap data;

    PipelineTaskGraph pipelineGraph;
    pipelineGraph.load(settings->source());
    if(pipelineGraph.buildGraph()) {
        if(settings->currentVersion() != settings->version()) {
            pipelineGraph.save(settings->source());
        }
        data = pipelineGraph.data();
    }

    if(data.empty()) {
         return InternalError;
    }

    Pipeline *pipeline = Engine::loadResource<Pipeline>(settings->destination());
    if(pipeline == nullptr) {
        pipeline = Engine::objectCreate<Pipeline>();
    }
    pipeline->loadUserData(data);
    Engine::setResource(pipeline, settings->destination());

    return settings->saveBinary(Engine::toVariant(pipeline));
}

AssetConverterSettings *PipelineConverter::createSettings() {
    return new PipelineConverterSettings;
}
