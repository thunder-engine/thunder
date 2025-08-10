#include "animationbuilder.h"

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

    return settings->saveBinary(m_model.object());
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() {
    return new AnimationBuilderSettings();
}

int AnimationControllerBuilder::version() {
    return FORMAT_VERSION;
}
