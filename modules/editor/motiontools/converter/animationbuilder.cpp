#include "animationbuilder.h"

#include <resources/animationstatemachine.h>

#define FORMAT_VERSION 12

AnimationBuilderSettings::AnimationBuilderSettings() {
    setVersion(FORMAT_VERSION);
}

StringList AnimationBuilderSettings::typeNames() const {
    return { MetaType::name<AnimationStateMachine>() };
}

void AnimationControllerBuilder::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/machine.svg");
    }
}

AssetConverter::ReturnCode AnimationControllerBuilder::convertFile(AssetConverterSettings *settings) {
    m_model.load(settings->source());

    uint32_t uuid = settings->info().id;
    if(uuid == 0) {
        uuid = Engine::generateUUID();
        settings->info().id = uuid;
    }

    VariantList result;

    VariantList object;

    object.push_back(AnimationStateMachine::metaClass()->name()); // type
    object.push_back(uuid); // id
    object.push_back(0); // parent
    object.push_back(""); // name

    object.push_back(VariantMap()); // properties
    object.push_back(VariantList()); // links

    object.push_back(m_model.data()); // user data

    result.push_back(object);

    return settings->saveBinary(result, settings->absoluteDestination());
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() {
    return new AnimationBuilderSettings();
}

int AnimationControllerBuilder::version() {
    return FORMAT_VERSION;
}
