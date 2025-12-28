#include "converters/translatorconverter.h"

#include <QFile>

#define FORMAT_VERSION 1

TranslatorConverterSettings::TranslatorConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList TranslatorConverterSettings::typeNames() const {
    return { MetaType::name<Translator>() };
}

void TranslatorConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/l10n.svg");
    }
}

AssetConverter::ReturnCode TranslatorConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QFile::ReadOnly)) {
        Translator *loc = Engine::loadResource<Translator>(settings->destination());
        if(loc == nullptr) {
            loc = Engine::objectCreate<Translator>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(loc->uuid() != uuid) {
            Engine::replaceUUID(loc, uuid);
        }

        while(!src.atEnd()) {
            QByteArray line = src.readLine();
            auto split = line.split(';');
            loc->setPair(split.first().constData(), split.last().constData());
        }
        src.close();

        return settings->saveBinary(Engine::toVariant(loc), settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *TranslatorConverter::createSettings() {
    return new TranslatorConverterSettings();
}
