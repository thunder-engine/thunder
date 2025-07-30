#include "converters/stylesheetconverter.h"

#include <QFile>

#include <bson.h>
#include <resources/stylesheet.h>

#define FORMAT_VERSION 1

StyleSheetConverterSettings::StyleSheetConverterSettings() {
    setType(MetaType::type<StyleSheet *>());
    setVersion(FORMAT_VERSION);
}

TString StyleSheetConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/css.svg";
}

AssetConverter::ReturnCode StyleSheetConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        StyleSheet *style = Engine::loadResource<StyleSheet>(settings->destination().toStdString());
        if(style == nullptr) {
            style = Engine::objectCreate<StyleSheet>();
        }

        QByteArray array = src.readAll();
        src.close();
        if(!array.isEmpty()) {
            style->setData(array.data());
        }

        QFile file(settings->absoluteDestination().data());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(style) );
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *StyleSheetConverter::createSettings() {
    return new StyleSheetConverterSettings();
}

TString StyleSheetConverter::templatePath() const {
    return ":/templates/StyleSheet.css";
}
