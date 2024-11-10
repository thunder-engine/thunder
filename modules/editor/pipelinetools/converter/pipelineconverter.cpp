#include "pipelineconverter.h"

#include "pipelinetaskgraph.h"

#include <resources/pipeline.h>
#include <bson.h>

#include <QFile>

#define FORMAT_VERSION 11

PipelineConverterSettings::PipelineConverterSettings() {
    setType(MetaType::type<Pipeline *>());
    setVersion(FORMAT_VERSION);
}

QString PipelineConverterSettings::defaultIcon(QString) const {
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

    Pipeline pipeline;
    pipeline.loadUserData(data);

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( Engine::toVariant(&pipeline) );
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();

        return Success;
    }

    return InternalError;
}

AssetConverterSettings *PipelineConverter::createSettings() {
    return new PipelineConverterSettings;
}
