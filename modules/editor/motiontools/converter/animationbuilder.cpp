#include "animationbuilder.h"

#include <QFile>

#include <bson.h>

#include <resources/animationstatemachine.h>

#define FORMAT_VERSION 12

AnimationBuilderSettings::AnimationBuilderSettings() {
    setType(MetaType::type<AnimationStateMachine *>());
    setVersion(FORMAT_VERSION);
}

TString AnimationBuilderSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/machine.svg";
}

AssetConverter::ReturnCode AnimationControllerBuilder::convertFile(AssetConverterSettings *settings) {
    m_model.load(settings->source());
    QFile file(settings->absoluteDestination().data());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( m_model.object() );
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
        return Success;
    }
    return InternalError;
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() {
    return new AnimationBuilderSettings();
}

int AnimationControllerBuilder::version() {
    return FORMAT_VERSION;
}
