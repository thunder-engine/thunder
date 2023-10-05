#include "animationbuilder.h"

#include <QFile>

#include <bson.h>

#include <resources/animationstatemachine.h>

#define FORMAT_VERSION 1

AnimationBuilderSettings::AnimationBuilderSettings() {
    setType(MetaType::type<AnimationStateMachine *>());
    setVersion(FORMAT_VERSION);
}

QString AnimationBuilderSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/machine.svg";
}

AssetConverter::ReturnCode AnimationControllerBuilder::convertFile(AssetConverterSettings *settings) {
    m_model.load(settings->source());
    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( m_model.object() );
        file.write((const char *)&data[0], data.size());
        file.close();
        return Success;
    }
    return InternalError;
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() const {
    return new AnimationBuilderSettings();
}
