#include "nativecodebuilder.h"

#include <url.h>
#include <log.h>

#include "editor/projectsettings.h"
#include "editor/pluginmanager.h"

#include <QFile>
#include <QRegularExpression>

namespace {
    const char *gSdkPath("${sdkPath}");
    const char *gIncludePaths("${includePaths}");
    const char *gLibraryPaths("${libraryPaths}");
    const char *gLibraries("${libraries}");

    const char *gFilesList("${FilesList}");

    const char *gLibrariesList("${LibrariesList}");
    const char *gEditorLibrariesList("${EditorLibrariesList}");

    const char *gRegisterModules("${RegisterModules}");
    const char *gModuleIncludes("${ModuleIncludes}");

    const char *gRegisterComponents("${RegisterComponents}");
    const char *gUnregisterComponents("${UnregisterComponents}");
    const char *gComponentNames("${ComponentNames}");
    const char *gIncludes("${Includes}");
}

NativeCodeBuilder::NativeCodeBuilder() {
    connect(&m_process, _SIGNAL(readyReadStandardOutput()), this, _SLOT(onReadOutput()) );
    connect(&m_process, _SIGNAL(readyReadStandardError()), this, _SLOT(onReadError()) );

    ProjectSettings *project = ProjectSettings::instance();

    TString idName = project->projectName().remove(' ').toLower();
    idName.remove('_');
    m_values["${idName}"] = idName;

    m_incPref = TString(12, ' ') + "\""; m_incSuff = "\""; m_incSep = ",";
    m_libPref = TString(12, ' ') + "\""; m_libSuff = "\""; m_libSep = ",";
    m_libsPref = TString(12, ' ') + "\""; m_libsSuff = "\""; m_libsSep = ",";
    m_filePref = TString(12, ' ') + "\""; m_fileSuff = "\""; m_fileSep = ",";

    TString sdk(project->sdkPath());

    m_incPath.push_back(sdk + "/include/engine");
    m_incPath.push_back(sdk + "/include/modules");
    m_incPath.push_back(sdk + "/include/next");
    m_incPath.push_back(sdk + "/include/next/math");
    m_incPath.push_back(sdk + "/include/next/core");

    m_libs.push_back("engine");
    m_libs.push_back("next");
    m_libs.push_back("physfs");
    m_libs.push_back("glfm");
    m_libs.push_back("bullet");
    m_libs.push_back("bullet3");
    m_libs.push_back("rendergl");
    m_libs.push_back("freetype");
    m_libs.push_back("uikit");
    m_libs.push_back("media");
    m_libs.push_back("angel");
    m_libs.push_back("angelscript");
}

bool NativeCodeBuilder::buildProject() {
    return false;
}

void NativeCodeBuilder::onBuildFinished(int exitCode) {
    if(exitCode == 0) {
        if(ProjectSettings::instance()->targetPath().isEmpty()) {
            PluginManager::instance()->reloadPlugin(m_artifact);
        }
    }

    aInfo() << name() << "Build finished";
    buildSuccessful(exitCode == 0);

    m_outdated = false;
}

void NativeCodeBuilder::onReadOutput() {
    parseLogs(m_process.readAllStandardOutput());
}

void NativeCodeBuilder::onReadError() {
    parseLogs(m_process.readAllStandardError());
}

void NativeCodeBuilder::generateProject() {
    aInfo() << name() << "Generating project";

    ProjectSettings *mgr = ProjectSettings::instance();

    m_project = mgr->generatedPath() + "/";

    m_values[gSdkPath] = mgr->sdkPath();

    m_values[gIncludePaths] = formatList(m_incPath, m_incPref, m_incSuff, m_incSep);
    m_values[gLibraryPaths] = formatList(m_libPath, m_libPref, m_libSuff, m_libSep);
    m_values[gLibraries] = formatList(m_libs, m_libsPref, m_libsSuff, m_libsSep);
    m_values[gFilesList] = formatList(StringList(m_sources.begin(), m_sources.end()), m_filePref, m_fileSuff, m_fileSep);

    const MetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        m_values[TString("${%1}").arg(property.name())] = property.read(mgr).toString();
    }

    generateLoader(mgr->templatePath(), mgr->modules());
}

void NativeCodeBuilder::generateLoader(const TString &dst, const StringList &modules) {
    std::map<TString, TString> classes;
    // Generate plugin loader
    for(const TString &it : m_sources) {
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
                    comment = data.indexOf("//");

                    static const QRegularExpression rx("A_OBJECT\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");
                    auto m = rx.globalMatch(data);
                    while(m.hasNext()) {
                        if(valid && comment == -1) {
                            QRegularExpressionMatch match = m.next();
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

    std::set<TString> includes;
    auto it = classes.begin();
    while(it != classes.end()) {
        includes.insert(TString("#include \"") + it->second + "\"\n");
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
    m_values[gIncludes] = TString::join(StringList(includes.begin(), includes.end()), "");

    for(auto &it : modules) {
        TString name(it);
        name.remove(' ');
        if(!name.isEmpty()) {
            m_values[gRegisterModules].append(TString(8, ' ') + "engine->addModule(new " + name + "(engine));\n");
            m_values[gModuleIncludes].append(TString("#include <") + name.toLower() + ".h>\n");
            m_values[gLibrariesList].append(TString(12, ' ') + "\"" + name.toLower() + "\",\n");
        }
    }

    TString name = ProjectSettings::instance()->projectName() + "-editor";
    for(auto &it : PluginManager::instance()->plugins()) {
        Url info(it);
        if(name != info.baseName()) {
            m_values[gEditorLibrariesList].append(TString(12, ' ') + "\"" + info.baseName() + "\",\n");
        }
    }

    updateTemplate(dst + "/plugin.cpp", project() + "plugin.cpp");
    updateTemplate(dst + "/application.cpp", project() + "application.cpp");
}

TString NativeCodeBuilder::formatList(const StringList &list, const TString &pref, const TString &suff, const TString &sep) const {
    TString result;
    for(int i = 0; i < list.size(); ++i) {
        result += pref + (*std::next(list.begin(), i)) + suff;
        if(i < (list.size() - 1)) {
            result += sep;
        }
        result += "\n";
    }
    return result;
}

void NativeCodeBuilder::parseLogs(const TString &log) {
    for(const TString &it : log.split("\r\n")) {
        if(!it.isEmpty()) {
            if(it.contains(" error ") || it.contains(" error:")) {
                aError() << name() << it;
            } else if(it.contains(" warning ") || it.contains(" warning:")) {
                aWarning() << name() << it;
            } else {
                aInfo() << name() << it;
            }
        }
    }
}
