#include "converters/translatorconverter.h"

#include <QFile>

#include <bson.h>
#include <file.h>

#define FORMAT_VERSION 1

TranslatorConverterSettings::TranslatorConverterSettings() {
    setType(MetaType::type<Translator *>());
    setVersion(FORMAT_VERSION);
}

TString TranslatorConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/l10n.svg";
}

AssetConverter::ReturnCode TranslatorConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        Translator *loc = Engine::loadResource<Translator>(settings->destination());
        if(loc == nullptr) {
            loc = Engine::objectCreate<Translator>();
        }

        while(!src.atEnd()) {
            QByteArray line = src.readLine();
            auto split = line.split(';');
            loc->setPair(split.first().constData(), split.last().constData());
        }
        src.close();

        File file(settings->absoluteDestination());
        if(file.open(File::WriteOnly)) {
            file.write(Bson::save( Engine::toVariant(loc) ));
            file.close();

            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *TranslatorConverter::createSettings() {
    return new TranslatorConverterSettings();
}
