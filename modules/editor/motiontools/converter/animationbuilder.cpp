#include "animationbuilder.h"

#include <resources/animationstatemachine.h>

#define FORMAT_VERSION 12

AnimationBuilderSettings::AnimationBuilderSettings() {
    setVersion(FORMAT_VERSION);
}

StringList AnimationBuilderSettings::typeNames() const {
    return { "AnimationStateMachine" };
}

TString AnimationBuilderSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/machine.svg";
}

AssetConverter::ReturnCode AnimationControllerBuilder::convertFile(AssetConverterSettings *settings) {
    m_model.load(settings->source());

    return settings->saveBinary(m_model.object(), settings->absoluteDestination());
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() {
    return new AnimationBuilderSettings();
}

int AnimationControllerBuilder::version() {
    return FORMAT_VERSION;
}
