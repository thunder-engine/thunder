#include "converters/translatorconverter.h"

#include <QFile>

#define FORMAT_VERSION 1

TranslatorConverterSettings::TranslatorConverterSettings() {
    setVersion(FORMAT_VERSION);
}

TString TranslatorConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/l10n.svg";
}

StringList TranslatorConverterSettings::typeNames() const {
    return { "Translator" };
}

AssetConverter::ReturnCode TranslatorConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
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

        return settings->saveBinary(Engine::toVariant(loc), settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *TranslatorConverter::createSettings() {
    return new TranslatorConverterSettings();
}
