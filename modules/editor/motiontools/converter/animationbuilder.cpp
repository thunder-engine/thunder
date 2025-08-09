#include "animationbuilder.h"

#include <bson.h>
#include <file.h>

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
    File file(settings->absoluteDestination());
    if(file.open(File::WriteOnly)) {
        file.write(Bson::save( m_model.object() ));
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
