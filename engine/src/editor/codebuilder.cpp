#include "editor/codebuilder.h"

#include "resources/text.h"

#include "editor/pluginmanager.h"
#include "editor/projectsettings.h"
#include "editor/assetmanager.h"

#include <QFile>
#include <QMap>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>

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

BuilderSettings::BuilderSettings(CodeBuilder *builder) :
        m_builder(builder) {
    setType(MetaType::type<Text *>());
}

QStringList BuilderSettings::typeNames() const {
    return { "Code" };
}

QString BuilderSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/code.svg";
}

CodeBuilder *BuilderSettings::builder() const {
    return m_builder;
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

void CodeBuilder::buildSuccessful() {
    AssetManager::instance()->onBuildSuccessful(this);
}

AssetConverterSettings *CodeBuilder::createSettings() {
    return new BuilderSettings(this);
}

void CodeBuilder::renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName) {
    QFile file(settings->source());
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QString data = file.readAll();
        file.close();

        static const QStringList templates = {
            "class %1",
            "%1()",
            "(%1"
        };

        for(auto it : templates) {
            data.replace(it.arg(oldName.data()), it.arg(newName.data()));
        }

        if(file.open(QFile::WriteOnly | QFile::Text)) {
            file.write(qPrintable(data));
            file.close();
        }
    }
}

void CodeBuilder::updateTemplate(const TString &src, const TString &dst) {
    QFile file(dst.data());
    if(!file.exists()) {
        file.setFileName(src.data());
    }

    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QString data = file.readLine();

        TString out;

        int begin = -1;
        int row = 0;
        while(!data.isNull()) {
            int index = -1;
            if(begin > -1) {
                index = data.indexOf(QByteArray("//-"));
                if(index != -1) {
                    begin = -1;
                    out += data.toStdString();
                }
            } else {
                auto it = m_values.begin();
                while(it != m_values.end()) {
                    if(it->first.at(0) == '$') {
                        data.replace(it->first.data(), it->second.data());
                    }
                    it++;
                }

                out += data.toStdString();
            }

            index = data.indexOf(QByteArray("//+"));
            if(index != -1) {
                begin = row;

                TString key = data.mid(index + 3).trimmed().toStdString();
                TString value = m_values[key];
                if(!value.isEmpty()) {
                    out += value;
                }
            }

            data = file.readLine();
            row++;
        }
        file.close();

        QDir dir;
        dir.mkpath(QFileInfo(dst.data()).absolutePath());

        file.setFileName(dst.data());
        if(file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
            file.write(out.data());
            file.close();
        }
    }
}

void CodeBuilder::generateLoader(const TString &dst, const StringList &modules) {
    std::map<TString, TString> classes;
    // Generate plugin loader
    for(TString it : m_sources) {
        QFile file(it.data());
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            QString data = file.readLine();
            bool valid = true;
            while(!data.isNull()) {
                if(!valid && data.indexOf("*/") != -1) {
                    valid = true;
                }
                int comment = data.indexOf("/*");
                if(comment == -1) {
                    int comment = data.indexOf("//");

                    static const QRegularExpression rx("A_OBJECT\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");
                    auto m = rx.globalMatch(data);
                    while(m.hasNext()) {
                        QRegularExpressionMatch match = m.next();

                        if(valid && comment == -1) {
                            TString className = match.captured(1).toStdString();

                            classes[className] = it;
                        }
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
        auto it = classes.begin();
        while(it != classes.end()) {
            includes << QString("#include \"") + it->second.data() + "\"\n";
            m_values[gRegisterComponents].append(TString(4, ' ') + it->first + "::registerClassFactory(m_engine);\n");
            m_values[gUnregisterComponents].append(TString(4, ' ') + it->first + "::unregisterClassFactory(m_engine);\n");
            TString value = TString("\"        \\\"") + it->first + "\\\"";
            it++;
            if(it != classes.end()) {
                value += ",";
            }
            value += "\"\n";

            m_values[gComponentNames].append(value);
        }
        includes.removeDuplicates();
        m_values[gIncludes] = includes.join("").toStdString();
    }
    {
        for(auto &it : modules) {
            TString name(it);
            name.remove(' ');
            if(!name.isEmpty()) {
                m_values[gRegisterModules].append(TString(8, ' ') + "engine->addModule(new " + name + "(engine));\n");
                m_values[gModuleIncludes].append(TString("#include <") + name.toLower() + ".h>\n");
                m_values[gLibrariesList].append(TString(12, ' ') + "\"" + name.toLower() + "\",\n");
            }
        }

        TString name = ProjectSettings::instance()->projectName().toStdString() + "-editor";
        for(auto &it : PluginManager::instance()->plugins()) {
            Url info(it.toStdString());
            if(name != info.baseName()) {
                m_values[gEditorLibrariesList].append(TString(12, ' ') + "\"" + info.baseName() + "\",\n");
            }
        }
    }

    updateTemplate(dst + "/plugin.cpp", project() + "plugin.cpp");
    updateTemplate(dst + "/application.cpp", project() + "application.cpp");
}

const TString CodeBuilder::persistentAsset() const {
    return "";
}

const TString CodeBuilder::persistentUUID() const {
    return "";
}

StringList CodeBuilder::platforms() const {
    return StringList();
}

TString CodeBuilder::project() const {
    return m_project;
}

StringList CodeBuilder::sources() const {
    StringList list;
    for(auto it : m_sources) {
        list.push_back(it);
    }
    return list;
}

void CodeBuilder::rescanSources(const TString &path) {
    m_sources.clear();

    QStringList suff;
    for(auto it : suffixes()) {
        suff.push_back(it.data());
    }

    QDirIterator it(path.data(), QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(suff.contains(info.completeSuffix(), Qt::CaseInsensitive)) {
            m_sources.insert(info.absoluteFilePath().toStdString());
        }
    }

    m_values[gFilesList] = formatList(StringList(m_sources.begin(), m_sources.end()));
}

bool CodeBuilder::isEmpty() const {
    return m_sources.empty();
}

bool CodeBuilder::isBundle(const TString &platform) const {
    Q_UNUSED(platform);
    return false;
}

void CodeBuilder::makeOutdated() {
    m_outdated = true;
}

bool CodeBuilder::isOutdated() const {
    return m_outdated;
}

TString CodeBuilder::formatList(const StringList &list) const {
    TString result;
    for(int i = 0; i < list.size(); ++i) {
        result += TString(12, ' ') + "\"" + (*std::next(list.begin(), i)) + "\"";
        if(i < (list.size() - 1)) {
            result += ',';
        }
        result += "\n";
    }
    return result;
}

QAbstractItemModel *CodeBuilder::classMap() const {
    return nullptr;
}
