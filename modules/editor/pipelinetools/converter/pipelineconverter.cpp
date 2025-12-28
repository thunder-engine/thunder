#include "pipelineconverter.h"

#include "pipelinetaskgraph.h"

#include <resources/pipeline.h>

#define FORMAT_VERSION 11

PipelineConverterSettings::PipelineConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList PipelineConverterSettings::typeNames() const {
    return { MetaType::name<Pipeline>() };
}

int PipelineConverterSettings::version() {
    return FORMAT_VERSION;
}

void PipelineConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/pipeline.svg");
    }
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
        pipeline = Engine::objectCreate<Pipeline>(settings->destination());
    }

    uint32_t uuid = settings->info().id;
    if(uuid == 0) {
        uuid = Engine::generateUUID();
        settings->info().id = uuid;
    }

    if(pipeline->uuid() != uuid) {
        Engine::replaceUUID(pipeline, uuid);
    }

    pipeline->loadUserData(data);

    return settings->saveBinary(Engine::toVariant(pipeline), settings->absoluteDestination());
}

AssetConverterSettings *PipelineConverter::createSettings() {
    return new PipelineConverterSettings;
}
