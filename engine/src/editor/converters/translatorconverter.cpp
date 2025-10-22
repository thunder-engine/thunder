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

        while(!src.atEnd()) {
            QByteArray line = src.readLine();
            auto split = line.split(';');
            loc->setPair(split.first().constData(), split.last().constData());
        }
        src.close();

        settings->info().id = loc->uuid();

        return settings->saveBinary(Engine::toVariant(loc), settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *TranslatorConverter::createSettings() {
    return new TranslatorConverterSettings();
}
