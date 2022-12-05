#include "editor/codebuilder.h"

#include "resources/text.h"

#include <editor/pluginmanager.h>
#include <editor/projectmanager.h>

#include <QFile>
#include <QMap>
#include <QMetaProperty>
#include <QDir>
#include <QDirIterator>

namespace  {
    const char *gFilesList("{FilesList}");
    const char *gLibrariesList("{LibrariesList}");
    const char *gEditorLibrariesList("{EditorLibrariesList}");

    const char *gRegisterModules("{RegisterModules}");
    const char *gModuleIncludes("{ModuleIncludes}");

    const char *gRegisterComponents("{RegisterComponents}");
    const char *gUnregisterComponents("{UnregisterComponents}");
    const char *gComponentNames("{ComponentNames}");
    const char *gIncludes("{Includes}");
}

static const QRegExp gClass("A_REGISTER\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");

BuilderSettings::BuilderSettings() {
    setType(MetaType::type<Text *>());
}

QStringList BuilderSettings::typeNames() const {
    return { "Code" };
}

QString BuilderSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/code.svg";
}

bool BuilderSettings::isCode() const {
    return true;
}

CodeBuilder::CodeBuilder() :
        m_outdated(false) {

    m_values["${Identifier_Prefix}"] = "com.tunderengine";
}

AssetConverter::ReturnCode CodeBuilder::convertFile(AssetConverterSettings *) {
    makeOutdated();
    return Skipped;
}

AssetConverterSettings *CodeBuilder::createSettings() const {
    return new BuilderSettings();
}

void CodeBuilder::renameAsset(AssetConverterSettings *settings, const QString &oldName, const QString &newName) {
    QFile file(settings->source());
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data = file.readAll();
        file.close();

        static const QStringList templates = {
            "class %1",
            "%1()",
            "(%1"
        };

        foreach(auto it, templates) {
            data.replace(it.arg(oldName), qPrintable(it.arg(newName)));
        }

        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(data);
            file.close();
        }
    }
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
    foreach(QString it, m_sources) {
        QFile file(it);
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray data = file.readLine();
            bool valid = true;
            while(!data.isNull()) {
                if(!valid && data.indexOf("*/") != -1) {
                    valid = true;
                }
                int comment = data.indexOf("/*");
                if(comment == -1) {
                    int comment = data.indexOf("//");
                    int index = gClass.indexIn(QString(data));
                    if(valid && index != -1 && !gClass.cap(1).isEmpty() && (comment == -1 || comment > index)) {
                        classes[gClass.cap(1)] = it;
                    }
                } else if(data.indexOf("*/", comment + 2) == -1) {
                    valid = false;
                }
                data = file.readLine();
            }
            file.close();
        }
    }

    m_values[gRegisterComponents].clear();
    m_values[gUnregisterComponents].clear();
    m_values[gComponentNames].clear();
    m_values[gRegisterModules].clear();
    m_values[gModuleIncludes].clear();
    m_values[gLibrariesList].clear();
    m_values[gEditorLibrariesList].clear();
    {
        QStringList includes;
        QMapIterator<QString, QString> it(classes);
        while(it.hasNext()) {
            it.next();
            includes << "#include \"" + it.value() + "\"\n";
            m_values[gRegisterComponents].append(it.key() + "::registerClassFactory(m_engine);\n");
            m_values[gUnregisterComponents].append(it.key() + "::unregisterClassFactory(m_engine);\n");
            m_values[gComponentNames].append("\"        \\\"" + it.key() + "\\\"" + (it.hasNext() ? "," : "") + "\"\n");
        }
        includes.removeDuplicates();
        m_values[gIncludes] = includes.join("");
    }
    {
        for(auto &it : modules) {
            m_values[gRegisterModules].append(QString("engine->addModule(new %1(engine));\n").arg(it));
            m_values[gModuleIncludes].append(QString("#include <%1.h>\n").arg(it.toLower()));
            m_values[gLibrariesList].append(QString("\t\t\t\"%1\",\n").arg(it.toLower()));
        }

        QString name = ProjectManager::instance()->projectName() + "-editor";
        for(auto &it : PluginManager::instance()->plugins()) {
            QFileInfo info(it);
            if(name != info.baseName()) {
                m_values[gEditorLibrariesList].append(QString("\t\t\t\"%1\",\n").arg(info.baseName()));
            }
        }
    }

    updateTemplate(dst + "/plugin.cpp", project() + "plugin.cpp", m_values);
    updateTemplate(dst + "/application.cpp", project() + "application.cpp", m_values);
}

const QString CodeBuilder::persistentAsset() const {
    return "";
}

const QString CodeBuilder::persistentUUID() const {
    return "";
}

QStringList CodeBuilder::platforms() const {
    return QStringList();
}

QString CodeBuilder::project() const {
    return m_project;
}

QStringList CodeBuilder::sources() const {
    return m_sources;
}

void CodeBuilder::rescanSources(const QString &path) {
    m_sources.clear();
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(suffixes().contains(info.completeSuffix(), Qt::CaseInsensitive)) {
            m_sources.push_back(info.absoluteFilePath());
        }
    }
    m_sources.removeDuplicates();

    m_values[gFilesList] = formatList(m_sources);
}

bool CodeBuilder::isEmpty() const {
    return m_sources.empty();
}

bool CodeBuilder::isPackage(const QString &platform) const {
    Q_UNUSED(platform);
    return false;
}

void CodeBuilder::makeOutdated() {
    m_outdated = true;
}

bool CodeBuilder::isOutdated() const {
    return m_outdated;
}

QString CodeBuilder::formatList(const QStringList &list) const {
    QString result;
    for(int i = 0; i < list.size(); ++i) {
        result += QString("\t\t\t\"%1\"\n%2").arg(list.at(i), (i < (list.size() - 1)) ? "," : "");
    }
    return result;
}

QAbstractItemModel *CodeBuilder::classMap() const {
    return nullptr;
}
