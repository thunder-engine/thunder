#include "converters/builder.h"

#include "resources/text.h"

#include <QFile>
#include <QMap>
#include <QMetaProperty>
#include <QDir>
#include <QDirIterator>

const QString gFilesList("${filesList}");

const QString gRegisterComponents("${RegisterComponents}");
const QString gUnregisterComponents("${UnregisterComponents}");
const QString gComponentNames("${ComponentNames}");
const QString gIncludes("${Includes}");

const QRegExp gClass("A_REGISTER\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");

IBuilder::IBuilder() :
        m_Outdated(false) {

    m_Values["${Identifier_Prefix}"] = "com.tunderengine";
}

uint32_t IBuilder::type() const {
    return MetaType::type<Text *>();
}

uint8_t IBuilder::convertFile(IConverterSettings *) {
    m_Outdated = true;
    return 1;
}

void IBuilder::copyTemplate(const QString &src, const QString &dst, QStringMap &values) {
    QFile file(src);
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data(file.readAll());
        file.close();

        QMapIterator<QString, QString> it(values);
        while(it.hasNext()) {
            it.next();
            data.replace(it.key(), qPrintable(it.value()));
        }
        QDir dir;
        dir.mkpath(QFileInfo(dst).absolutePath());
        QFile gen(dst);
        if(gen.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
            gen.write(data);
            gen.close();
        }
    }
}

void IBuilder::generateLoader(const QString &dst) {
    QStringMap classes;
    // Generate plugin loader
    foreach(QString it, m_Sources) {
        QFile file(it);
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray data = file.readLine();
            bool valid      = true;
            while(!data.isEmpty()) {
                if(!valid && data.indexOf("*/") != -1) {
                    valid = true;
                }
                int comment = data.indexOf("/*");
                if(comment == -1) {
                    int comment = data.indexOf("//");
                    int index   = gClass.indexIn(QString(data));
                    if(valid && index != -1 && !gClass.cap(1).isEmpty() && (comment == -1 || comment > index)) {
                        classes[gClass.cap(1)] = it;
                    }
                } else if(data.indexOf("*/", comment + 2) == -1) {
                    valid   = false;
                }
                data = file.readLine();
            }
            file.close();
        }
    }

    m_Values[gRegisterComponents].clear();
    m_Values[gUnregisterComponents].clear();
    m_Values[gComponentNames].clear();

    QStringList includes;
    QMapIterator<QString, QString> it(classes);
    while(it.hasNext()) {
        it.next();
        includes << "#include \"" + it.value() + "\"\n";
        m_Values[gRegisterComponents].append(it.key() + "::registerClassFactory(m_pEngine);\n\t\t");
        m_Values[gUnregisterComponents].append(it.key() + "::unregisterClassFactory(m_pEngine);\n\t\t");
        m_Values[gComponentNames].append("result.push_back(\"" + it.key() + "\");\n\t\t");
    }
    includes.removeDuplicates();
    m_Values[gIncludes] = includes.join("");

    copyTemplate(dst + "/plugin.cpp", project() + "plugin.cpp", m_Values);
    copyTemplate(dst + "/application.cpp", project() + "application.cpp", m_Values);
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

    m_Values[gFilesList] = formatList(m_Sources);
}

bool IBuilder::isEmpty() const {
    return m_Sources.empty();
}

QString IBuilder::formatList(const QStringList &list) {
    bool first  = true;
    QString result;
    foreach(QString it, list) {
        result += QString("%2\n\t\t\t\"%1\"").arg(it).arg((!first) ? "," : "");
        first   = false;
    }
    return result;
}
