#include "projectsettings.h"

#include <QUuid>
#include <QDir>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>

#include <log.h>
#include <json.h>

#include "config.h"

#include <components/actor.h>
#include <resources/map.h>

#include <editor/assetmanager.h>
#include <editor/codebuilder.h>
#include <editor/editorplatform.h>

namespace {
    const char *gProjects("Projects");

    const char *gModulesFile("/modules.txt");
};

ProjectSettings *ProjectSettings::m_pInstance = nullptr;

ProjectSettings::ProjectSettings() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#if defined(Q_OS_MAC)
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    m_sdkPath = dir.absolutePath().toStdString();
    m_resourcePath = m_sdkPath + "/resources";
    m_templatePath = m_resourcePath + "/editor/templates";

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QString path = settings.value(gProjects, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    m_myProjectsPath = path.toStdString();
}

ProjectSettings *ProjectSettings::instance() {
    if(!m_pInstance) {
        m_pInstance = new ProjectSettings;
    }
    return m_pInstance;
}

void ProjectSettings::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void ProjectSettings::init(const TString &project, const TString &target) {
    m_projectPath = project;

    if(!target.isEmpty()) {
        QDir dir;
        dir.mkpath(target.data());
    }
    m_targetPath = target;

    Url path(m_projectPath);

    m_projectName = path.baseName();

    m_contentPath = path.absoluteDir() + "/" + gContent;
    m_pluginsPath = path.absoluteDir() + "/" + gPlugins;
    m_cachePath = path.absoluteDir() + "/" + gCache;

    m_importPath = m_cachePath + "/" + gImport;
    m_iconPath = m_cachePath + "/" + gThumbnails;
    m_generatedPath = m_cachePath + "/" + gGenerated;

    m_manifestFile = path.absoluteDir() + "/" + gPlatforms + "/android/AndroidManifest.xml";

    EditorPlatform::instance().setImportPath(m_importPath);

    QDir dir;
    dir.mkpath(m_contentPath.data());
    dir.mkpath(m_iconPath.data());
    dir.mkpath(m_generatedPath.data());
    dir.mkpath(m_pluginsPath.data());

    File file(m_generatedPath + gModulesFile);
    if(file.open(File::ReadOnly)) {
        for(auto &it : TString(file.readAll()).split('\n')) {
            m_autoModules.insert(it);
        }
        file.close();
    }

    setCurrentPlatform();
}

void ProjectSettings::loadPlatforms() {
    for(auto &it : AssetManager::instance()->builders()) {
        for(auto &platform : it->platforms()) {
            m_supportedPlatforms[platform] = it;
        }
    }
}

void ProjectSettings::loadSettings() {
    blockSignals(true);

    File file(m_projectPath);
    if(file.open(File::ReadOnly)) {
        VariantMap object = Json::load(file.readAll()).toMap();
        file.close();

        m_projectId = QUuid::createUuid().toString().toStdString();

        const MetaObject *meta = metaObject();
        for(const auto &it : object) {
            TString name = it.first;
            name.remove('_');
            name[0] = std::tolower(name[0]);

            setProperty(name.data(), it.second);
        }
        {
            auto it = object.find(gPlatforms);
            if(it != object.end()) {
                m_platforms.clear();
                for(auto platform : it->second.toList()) {
                    m_platforms.push_back(platform.toString());
                }
            }
        }
        {
            auto it = object.find(gModules);
            if(it != object.end()) {
                for(auto module : it->second.toList()) {
                    m_modules.insert(module.toString());
                }
            }
        }
        {
            auto it = object.find(gPlugins);
            if(it != object.end()) {
                VariantMap plugins = it->second.toMap();
                for(auto plugin : plugins) {
                    m_plugins[plugin.first] = plugin.second.toBool();
                }
            }
        }

        m_autoModules.insert("RenderGL");
    }

    blockSignals(false);
}

void ProjectSettings::saveSettings() {
    if(isSignalsBlocked()) {
        return;
    }

    const MetaObject *meta = metaObject();

    VariantMap object;

    bool success = true;
    StringList req;
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);

        TString name = property.name();
        Variant value = property.read(this);

        MetaType type = MetaType::table(value.userType());
        if(value.type() == QMetaType::QVariantList) {
            // Just skip the list type for now
        } else {
            TString str = value.toString();

            object[name] = str;

            if(str.isEmpty()) {
                success = false;
                req.push_back(name.replace('_', ' '));
            }
        }
    }
    if(!success) {
        aCritical() << "The required settings was not specified:" << TString::join(req, ", ")
                    << "Please specify them in the Project Settings.";
    }

    VariantList platforms;
    for(auto &it : m_platforms) {
        platforms.push_back(it);
    }
    object[gPlatforms] = platforms;

    VariantList modules;
    for(auto &it : m_modules) {
        modules.push_back(it);
    }
    object[gModules] = modules;
    if(!m_plugins.empty()) {
        VariantMap plugins;
        for(auto it : m_plugins) {
            plugins[it.first] = it.second;
        }
        object[gPlugins] = plugins;
    }

    File file(m_projectPath);
    if(file.open(File::WriteOnly)) {
        file.write(Json::save(object, 0));
        file.close();
    } else {
        aCritical() << "Unable to save the Project Settings.";
    }
}

TString ProjectSettings::artifact() const {
    return m_artifact;
}

void ProjectSettings::setArtifact(const TString &value) {
    m_artifact = value;
}

TString ProjectSettings::projectName() const {
    return m_projectName;
}

void ProjectSettings::setProjectName(const TString &value) {
    if(m_projectName != value) {
        m_projectName = value;
        saveSettings();
    }
}

TString ProjectSettings::projectId() const {
    return m_projectId;
}

void ProjectSettings::setProjectId(const TString &value) {
    if(m_projectId != value && !value.isEmpty()) {
        m_projectId = value;
        saveSettings();
    }
}

TString ProjectSettings::projectCompany() const {
    return m_companyName;
}

void ProjectSettings::setProjectCompany(const TString &value) {
    if(m_companyName != value) {
        m_companyName = value;
        saveSettings();
    }
}

TString ProjectSettings::projectVersion() const {
    return m_projectVersion;
}

void ProjectSettings::setProjectVersion(const TString &value) {
    if(m_projectVersion != value) {
        m_projectVersion = value;
        saveSettings();
    }
}

TString ProjectSettings::firstMap() const {
    return m_firstMap;
}
void ProjectSettings::setFirstMap(const TString &value) {
    if(m_firstMap != value) {
        m_firstMap = value;
        saveSettings();
    }
}

TString ProjectSettings::projectSdk() const {
    return m_projectSdk;
}

void ProjectSettings::setProjectSdk(const TString &sdk) {
    m_projectSdk = sdk;
}

TString ProjectSettings::projectPath() const {
    return m_projectPath;
}

TString ProjectSettings::targetPath() const {
    return m_targetPath;
}

TString ProjectSettings::contentPath() const {
    return m_contentPath;
}

TString ProjectSettings::cachePath() const {
    return m_cachePath;
}

TString ProjectSettings::importPath() const {
    return m_importPath;
}

TString ProjectSettings::iconPath() const {
    return m_iconPath;
}

TString ProjectSettings::generatedPath() const {
    return m_generatedPath;
}

TString ProjectSettings::pluginsPath() const {
    return m_pluginsPath;
}

TString ProjectSettings::manifestFile() const {
    return m_manifestFile;
}

TString ProjectSettings::sdkPath() const {
    return m_sdkPath;
}

TString ProjectSettings::resourcePath() const {
    return m_resourcePath;
}

TString ProjectSettings::templatePath() const {
    return m_templatePath;
}

TString ProjectSettings::myProjectsPath() const {
    return m_myProjectsPath;
}

StringList ProjectSettings::modules() const {
    std::set<TString> result = m_autoModules;
    result.insert(m_modules.begin(), m_modules.end());
    return StringList(result.begin(), result.end());
}

StringList ProjectSettings::platforms() const {
    StringList list;
    for(auto it : m_supportedPlatforms) {
        list.push_back(it.first.data());
    }
    return (m_platforms.empty()) ? list : m_platforms;
}

std::map<TString, bool> &ProjectSettings::plugins() {
    return m_plugins;
}

void ProjectSettings::setCurrentPlatform(const TString &platform) {
    m_currentPlatform = (platform.isEmpty()) ?  "desktop" : platform;

    m_importPath = m_cachePath + (platform.isEmpty() ? "" : TString("/") + m_currentPlatform) + TString("/") + gImport;
    EditorPlatform::instance().setImportPath(m_importPath);

    QDir dir;
    dir.mkpath(m_importPath.data());
}

TString ProjectSettings::currentPlatformName() const {
    return m_currentPlatform;
}

CodeBuilder *ProjectSettings::currentBuilder(const TString &platform) const {
    TString key(platform.isEmpty() ? m_currentPlatform : platform);
    auto it = m_supportedPlatforms.find(key);
    if(it != m_supportedPlatforms.end()) {
        return it->second;
    }
    return nullptr;
}

void ProjectSettings::reportModules(const std::set<TString> &modules) {
    m_autoModules.insert(modules.begin(), modules.end());

    File file(m_generatedPath + gModulesFile);
    if(file.open(File::WriteOnly)) {
        file.write(TString::join(StringList(m_autoModules.begin(), m_autoModules.end()), "\n"));
        file.close();
    }
}

VariantList ProjectSettings::getModules() const {
    VariantList result;
    for(auto &it : m_modules) {
        result.push_back(it);
    }
    return result;
}

void ProjectSettings::setModules(VariantList modules) {
    m_modules.clear();
    for(auto &it : modules) {
        m_modules.insert(it.toString());
    }
    saveSettings();
}

VariantList ProjectSettings::getPlatforms() const {
    VariantList result;
    for(auto &it : m_platforms) {
        result.push_back(it);
    }
    return result;
}

void ProjectSettings::setPlatforms(VariantList platforms) {
    m_platforms.clear();
    for(auto &it : platforms) {
        m_platforms.push_back(it.toString());
    }
    saveSettings();
}
