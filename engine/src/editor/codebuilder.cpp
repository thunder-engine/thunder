#include "editor/codebuilder.h"

#include "resources/text.h"

#include <QFile>
#include <QMap>
#include <QMetaProperty>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

const QString gFilesList("{FilesList}");

const QString gRegisterModules("{RegisterModules}");
const QString gModuleIncludes("{ModuleIncludes}");

const QString gRegisterComponents("{RegisterComponents}");
const QString gUnregisterComponents("{UnregisterComponents}");
const QString gComponentNames("{ComponentNames}");
const QString gIncludes("{Includes}");

const QRegExp gClass("A_REGISTER\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");

BuilderSettings::BuilderSettings() {
    setType(MetaType::type<Text *>());
}

QString BuilderSettings::typeName() const {
    return "Code";
}

CodeBuilder::CodeBuilder() :
        m_Outdated(false) {

    m_Values["${Identifier_Prefix}"] = "com.tunderengine";
}

uint8_t CodeBuilder::convertFile(AssetConverterSettings *) {
    m_Outdated = true;
    return 1;
}

AssetConverterSettings *CodeBuilder::createSettings() const {
    return new BuilderSettings();
}

void CodeBuilder::updateTemplate(const QString &src, const QString &dst, QStringMap &values) {
    QFile file(dst);
    if(!file.exists()) {
        file.setFileName(src);
    }

    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data = file.readLine();

        QByteArray out;

        int begin = -1;
        int row = 0;
        while(!data.isNull()) {
            int index = -1;
            if(begin > -1) {
                index = data.indexOf(QString("//-"));
                if(index != -1) {
                    begin = -1;
                    out += data;
                }
            } else {
                QMapIterator<QString, QString> it(values);
                while(it.hasNext()) {
                    it.next();
                    if(it.key().at(0) == '$') {
                        data.replace(it.key(), qPrintable(it.value()));
                    }
                }

                out += data;
            }

            index = data.indexOf(QString("//+"));
            if(index != -1) {
                begin = row;

                QString value = values.value(data.mid(index + 3).trimmed());
                if(!value.isEmpty()) {
                    out += value;
                }
            }

            data = file.readLine();
            row++;
        }
        file.close();

        QDir dir;
        dir.mkpath(QFileInfo(dst).absolutePath());

        file.setFileName(dst);
        if(file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
            file.write(out);
            file.close();
        }
    }
}

void CodeBuilder::generateLoader(const QString &dst, const QStringList &modules) {
    QStringMap classes;
    // Generate plugin loader
    foreach(QString it, m_Sources) {
        QFile file(it);
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray data = file.readLine();
            bool valid      = true;
            while(!data.isNull()) {
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
    m_Values[gRegisterModules].clear();
    {
        QStringList includes;
        QMapIterator<QString, QString> it(classes);
        while(it.hasNext()) {
            it.next();
            includes << "#include \"" + it.value() + "\"\n";
            m_Values[gRegisterComponents].append(it.key() + "::registerClassFactory(m_pEngine);\n");
            m_Values[gUnregisterComponents].append(it.key() + "::unregisterClassFactory(m_pEngine);\n");
            m_Values[gComponentNames].append("\"        \\\"" + it.key() + "\\\"" + (it.hasNext() ? "," : "") + "\"\n");
        }
        includes.removeDuplicates();
        m_Values[gIncludes] = includes.join("");
    }
    {
        for(QString it : modules) {
            m_Values[gRegisterModules].append(QString("engine->addModule(new %1(engine));\n").arg(it));
            m_Values[gModuleIncludes].append(QString("#include <%1.h>\n").arg(it.toLower()));
        }
    }

    updateTemplate(dst + "/plugin.cpp", project() + "plugin.cpp", m_Values);
    updateTemplate(dst + "/application.cpp", project() + "application.cpp", m_Values);
}

const QString CodeBuilder::persistentAsset() const {
    return "";
}

const QString CodeBuilder::persistentUUID() const {
    return "";
}

QString CodeBuilder::project() const {
    return m_Project;
}

QStringList CodeBuilder::sources() const {
    return m_Sources;
}

void CodeBuilder::rescanSources(const QString &path) {
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

bool CodeBuilder::isEmpty() const {
    return m_Sources.empty();
}

bool CodeBuilder::isOutdated() const {
    return m_Outdated;
}

QString CodeBuilder::formatList(const QStringList &list) const {
    bool first  = true;
    QString result;
    for(int i = 0; i < list.size(); ++i) {
        result += QString("\t\t\t\"%1\"\n%2").arg(list.at(i)).arg((i < (list.size() - 1)) ? "," : "");
        first   = false;
    }
    return result;
}


QAbstractItemModel *CodeBuilder::classMap() const {
    return nullptr;
}
