#include "animationbuilder.h"

#include <resources/animationstatemachine.h>

#define FORMAT_VERSION 13

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

    AnimationStateMachine *machine = Engine::loadResource<AnimationStateMachine>(settings->destination());
    if(machine == nullptr) {
        machine = Engine::objectCreate<AnimationStateMachine>(settings->destination());
    }

    uint32_t uuid = settings->info().id;
    if(uuid == 0) {
        uuid = Engine::generateUUID();
        settings->info().id = uuid;
    }

    if(machine->uuid() != uuid) {
        Engine::replaceUUID(machine, uuid);
    }

    ResourceSystem::loadResourceData(machine, m_model.data());

    return settings->saveBinary(machine, settings->absoluteDestination());
}

AssetConverterSettings *AnimationControllerBuilder::createSettings() {
    return new AnimationBuilderSettings();
}

int AnimationControllerBuilder::version() {
    return FORMAT_VERSION;
}
