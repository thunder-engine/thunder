#include "converters/uiconverter.h"

#include <QFile>

#include <bson.h>
#include <resources/uidocument.h>

#define FORMAT_VERSION 1

UiConverterSettings::UiConverterSettings() {
    setType(MetaType::type<UiDocument *>());
    setVersion(FORMAT_VERSION);
}

QString UiConverterSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/ui.svg";
}

AssetConverter::ReturnCode UiConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        UiDocument document;
        QByteArray array = src.readAll();
        src.close();
        if(!array.isEmpty()) {
            document.setData(array.data());
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&document) );
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *UiConverter::createSettings() {
    return new UiConverterSettings();
}

TString UiConverter::templatePath() const {
    return ":/templates/UIDocument.ui";
}
