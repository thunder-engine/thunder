#include "converters/builder.h"

#include <QFile>
#include <QMap>
#include <QMetaProperty>
#include <QDir>
#include <QDirIterator>

IBuilder::IBuilder() :
        m_Outdated(false) {

}

uint8_t IBuilder::convertFile(IConverterSettings *) {
    m_Outdated = true;
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

void IBuilder::rescanSources(const QString &path) {
    m_Sources.clear();
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(suffixes().contains(info.completeSuffix(), Qt::CaseInsensitive)) {
            m_Sources.push_back(info.absoluteFilePath());
        }
    }
    m_Sources.removeDuplicates();
}
