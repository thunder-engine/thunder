#include "converters/builder.h"

#include <QFile>
#include <QMap>
#include <QMetaProperty>
#include <QDir>
#include <QDirIterator>

IBuilder::IBuilder() {

}

uint8_t IBuilder::convertFile(IConverterSettings *) {
    return 1;
}

void IBuilder::copyTemplate(const QString &src, const QString &dst, StringMap &values) {
    QFile file(src);
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data(file.readAll());
        file.close();

        QMapIterator<QString, QString> it(values);
        while(it.hasNext()) {
            it.next();
            data.replace(it.key(), qPrintable(it.value()));
        }
        QFile gen(dst);
        if(gen.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
            gen.write(data);
            gen.close();
        }
    }
}

QStringList IBuilder::rescanSources(const QString &path) const {
    QStringList result;
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(suffixes().contains(info.completeSuffix(), Qt::CaseInsensitive)) {
            result.push_back(info.absoluteFilePath());
        }
    }
    result.removeDuplicates();
    return result;
}
