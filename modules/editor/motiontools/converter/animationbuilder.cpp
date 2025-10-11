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

    Variant variant = m_model.object();

    uint32_t uuid = 0;
    VariantList objects = variant.value<VariantList>();
    for(auto &it : objects) {
        VariantList o  = it.value<VariantList>();
        if(o.size() >= 5) {
            auto i = o.begin();
            i++;
            uuid = static_cast<uint32_t>((*i).toInt());
            break;
        }
    }

    settings->info().id = uuid;

    return settings->saveBinary(variant, settings->absoluteDestination());
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() {
    return new AnimationBuilderSettings();
}

int AnimationControllerBuilder::version() {
    return FORMAT_VERSION;
}
