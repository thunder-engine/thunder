/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "projectsettings.h"

#include <QDir>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>

#include <log.h>
#include <json.h>
#include <file.h>
#include <url.h>
#include <os/uuid.h>

#include "config.h"

#include "editor/assetmanager.h"
#include "editor/nativecodebuilder.h"
#include "editor/editorplatform.h"
#include "editor/pluginmanager.h"

namespace {
    const char *gProjects("Projects");
    const char *gThumbnails("thumbnails");
    const char *gGenerated("generated");
    const char *gCache("cache");
    const char *gPlugins("plugins");
    const char *gModules("modules");
}

/*!
    \class ProjectSettings
    \brief Stores settings and paths for the current editor project.
    \inmodule Editor
*/

ProjectSettings::ProjectSettings() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#if defined(PLATFORM_MAC)
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    m_sdkPath = dir.absolutePath().toStdString();
    m_resourcePath = m_sdkPath + "/resources";
    m_templatePath = m_resourcePath + "/templates";

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QString path = settings.value(gProjects, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    m_myProjectsPath = path.toStdString();
}

/*!
    Initializes settings for a \a project and optional build \a target.
*/
void ProjectSettings::init(const TString &project, const TString &target) {
    m_projectPath = project;

    if(!target.isEmpty()) {
        File::mkPath(target);
    }
    m_targetPath = target;

    Url path(m_projectPath);

    m_projectName = path.baseName();

    m_contentPath = path.absoluteDir() + "/" + gContent;
    m_pluginsPath = path.absoluteDir() + "/" + gPlugins;
    m_cachePath = path.absoluteDir() + "/" + gCache;
    m_platformsPath = path.absoluteDir() + "/" + gPlatforms;

    m_importPath = m_cachePath + "/" + gImport;
    m_iconPath = m_cachePath + "/" + gThumbnails;
    m_generatedPath = m_cachePath + "/" + gGenerated;

    EditorPlatform::instance().setImportPath(m_importPath);

    File::mkPath(m_contentPath);
    File::mkPath(m_iconPath);
    File::mkPath(m_generatedPath);
    File::mkPath(m_pluginsPath);

    setCurrentPlatform();
    loadSettings();
}

/*!
    Loads platforms supported by the registered code builders.
*/
void ProjectSettings::loadPlatforms() {
    for(auto &it : Editor::assets()->builders()) {
        for(auto &platform : it->platforms()) {
            m_supportedPlatforms[platform] = it;
        }
    }
}

/*!
    Loads project settings from disk.
*/
void ProjectSettings::loadSettings() {
    blockSignals(true);

    File file(m_projectPath);
    if(file.open(File::Read)) {
        VariantMap object = Json::load(file.readAll()).toMap();
        file.close();

        m_projectId = Uuid::createUuid().toString();

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
                for(auto &platform : it->second.toList()) {
                    m_platforms.push_back(platform.toString());
                }
            }
        }
        {
            auto it = object.find(gModules);
            if(it != object.end()) {
                m_modules.clear();
                for(auto &module : it->second.toList()) {
                    m_modules.push_back(module.toString());
                }
            }
        }
        {
            auto it = object.find(gPlugins);
            if(it != object.end()) {
                VariantMap plugins = it->second.toMap();
                for(auto &plugin : plugins) {
                    m_plugins[plugin.first] = plugin.second.toBool();
                }
            }
        }
    }

    blockSignals(false);
}

/*!
    Saves project settings to disk.
*/
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

    for(const TString &name : dynamicPropertyNames()) {
        object[name] = property(name.data());
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
        for(auto &it : m_plugins) {
            plugins[it.first] = it.second;
        }
        object[gPlugins] = plugins;
    }

    File file(m_projectPath);
    if(file.open(File::Write)) {
        file.write(Json::save(object, 0));
        file.close();
    } else {
        aCritical() << "Unable to save the Project Settings.";
    }
}

/*!
    Returns project build artifacts.
*/
StringList ProjectSettings::artifacts() const {
    return m_artifacts;
}

/*!
    Sets project build \a artifacts.
*/
void ProjectSettings::setArtifacts(const StringList &artifacts) {
    m_artifacts = artifacts;
}

/*!
    Returns the project name.
*/
TString ProjectSettings::projectName() const {
    return m_projectName;
}

/*!
    Sets the project \a name.
*/
void ProjectSettings::setProjectName(const TString &name) {
    if(m_projectName != name) {
        m_projectName = name;
        saveSettings();
    }
}

/*!
    Returns the unique project identifier.
*/
TString ProjectSettings::projectId() const {
    return m_projectId;
}

/*!
    Sets the \a project identifier.
*/
void ProjectSettings::setProjectId(const TString &project) {
    if(m_projectId != project && !project.isEmpty()) {
        m_projectId = project;
        saveSettings();
    }
}

/*!
    Returns the project company name.
*/
TString ProjectSettings::projectCompany() const {
    return m_companyName;
}

/*!
    Sets the project company \a name.
*/
void ProjectSettings::setProjectCompany(const TString &name) {
    if(m_companyName != name) {
        m_companyName = name;
        saveSettings();
    }
}

/*!
    Returns the project version.
*/
TString ProjectSettings::projectVersion() const {
    return m_projectVersion;
}

/*!
    Sets the project \a version.
*/
void ProjectSettings::setProjectVersion(const TString &version) {
    if(m_projectVersion != version) {
        m_projectVersion = version;
        saveSettings();
    }
}

/*!
    Returns the first map opened for the project.
*/
TString ProjectSettings::firstMap() const {
    return m_firstMap;
}
/*!
    Sets the first map opened for the project.
*/
void ProjectSettings::setFirstMap(const TString &value) {
    if(m_firstMap != value) {
        m_firstMap = value;
        saveSettings();
    }
}

/*!
    Returns the SDK version used by the project.
*/
TString ProjectSettings::projectSdk() const {
    return m_projectSdk;
}

/*!
    Sets the \a sdk version used by the project.
*/
void ProjectSettings::setProjectSdk(const TString &sdk) {
    m_projectSdk = sdk;
}

/*!
    Returns the project file path.
*/
TString ProjectSettings::projectPath() const {
    return m_projectPath;
}

/*!
    Returns the build target path.
*/
TString ProjectSettings::targetPath() const {
    return m_targetPath;
}

/*!
    Returns the project content directory.
*/
TString ProjectSettings::contentPath() const {
    return m_contentPath;
}

/*!
    Returns the project cache directory.
*/
TString ProjectSettings::cachePath() const {
    return m_cachePath;
}

/*!
    Returns the imported resources directory.
*/
TString ProjectSettings::importPath() const {
    return m_importPath;
}

/*!
    Returns the generated asset icon directory.
*/
TString ProjectSettings::iconPath() const {
    return m_iconPath;
}

/*!
    Returns the generated source directory.
*/
TString ProjectSettings::generatedPath() const {
    return m_generatedPath;
}

/*!
    Returns the project plugin directory.
*/
TString ProjectSettings::pluginsPath() const {
    return m_pluginsPath;
}

/*!
    Returns the project platforms directory.
*/
TString ProjectSettings::platformsPath() const {
    return m_platformsPath;
}

/*!
    Returns the installed SDK directory.
*/
TString ProjectSettings::sdkPath() const {
    return m_sdkPath;
}

/*!
    Returns the editor resource directory.
*/
TString ProjectSettings::resourcePath() const {
    return m_resourcePath;
}

/*!
    Returns the project template directory.
*/
TString ProjectSettings::templatePath() const {
    return m_templatePath;
}

/*!
    Returns the directory containing user projects.
*/
TString ProjectSettings::myProjectsPath() const {
    return m_myProjectsPath;
}

/*!
    Returns the modules enabled for the project, including automatic dependencies.
*/
StringList ProjectSettings::modules() const {
    std::set<TString> result;
    result.insert(m_autoModules.begin(), m_autoModules.end());
    result.insert(m_modules.begin(), m_modules.end());
    NativeCodeBuilder *builder = currentBuilder();
    if(builder) {
        switch(builder->defaultRhi()) {
        case NativeCodeBuilder::OpenGL: result.insert("RenderGL"); break;
        case NativeCodeBuilder::Vulkan: result.insert("RenderVK"); break;
        case NativeCodeBuilder::Metal: result.insert("RenderMT"); break;
        default: break;
        }
    }

    return StringList(result.begin(), result.end());
}

/*!
    Returns the supported or selected project platforms.
*/
StringList ProjectSettings::platforms() const {
    StringList list;
    for(auto &it : m_supportedPlatforms) {
        list.push_back(it.first.data());
    }
    return (m_platforms.empty()) ? list : m_platforms;
}

/*!
    Returns the map of project plugins and their enabled states.
*/
std::map<TString, bool> &ProjectSettings::plugins() {
    return m_plugins;
}

/*!
    Selects the current platform and updates its import directory.
*/
void ProjectSettings::setCurrentPlatform(const TString &platform) {
    if(platform.isEmpty()) {
#if defined(PLATFORM_WINDOWS)
        m_currentPlatform = "windows";
#elif defined(PLATFORM_MAC)
        m_currentPlatform = "macos";
#elif defined(PLATFORM_LINUX)
        m_currentPlatform = "linux";
#endif
    } else {
        m_currentPlatform = platform;
    }

    m_importPath = m_cachePath + (platform.isEmpty() ? "" : TString("/") + m_currentPlatform) + "/" + gImport;
    EditorPlatform::instance().setImportPath(m_importPath);

    File::mkPath(m_importPath);
}

/*!
    Returns the name of the current platform.
*/
TString ProjectSettings::currentPlatformName() const {
    return m_currentPlatform;
}

/*!
    Returns the code builder registered for the specified or current platform.
*/
NativeCodeBuilder *ProjectSettings::currentBuilder(const TString &platform) const {
    TString key(platform.isEmpty() ? m_currentPlatform : platform);
    auto it = m_supportedPlatforms.find(key);
    if(it != m_supportedPlatforms.end()) {
        return dynamic_cast<NativeCodeBuilder *>(it->second);
    }
    return nullptr;
}

/*!
    Reports component types used by the project for automatic module discovery.
*/
void ProjectSettings::reportTypes(const std::set<TString> &types) {
    TString projectModule = TString("Module") + projectName();
    for(auto &it : types) {
        TString name = Editor::plugins()->getModuleName(it);
        if(!name.isEmpty() && name != projectModule) {
            if(std::find(m_autoModules.begin(), m_autoModules.end(), name) == m_autoModules.end()) {
                m_autoModules.push_back(name);
            }
        }
    }
}

Variant ProjectSettings::property(const char *name) const {
    TString str(name);
    if(str == gModules) {
        return getModules();
    } else if(str == gPlatforms) {
        return getPlatforms();
    }

    return Object::property(name);
}

void ProjectSettings::setProperty(const char *name, const Variant &value) {
    TString str(name);
    if(str == gModules) {
        setModules(value.toList());
    } else if(str == gPlatforms) {
        setPlatforms(value.toList());
    }

    Object::setProperty(name, value);
}

VariantList ProjectSettings::getModules() const {
    VariantList result;
    for(auto &it : m_modules) {
        result.push_back(it);
    }
    return result;
}

void ProjectSettings::setModules(const VariantList &modules) {
    StringList list;
    for(auto &it : modules) {
        TString module(it.toString());
        if(std::find(list.begin(), list.end(), module) == list.end()) {
            list.push_back(module);
        }
    }

    if(list != m_modules) {
        m_modules = list;
        saveSettings();
    }
}

VariantList ProjectSettings::getPlatforms() const {
    VariantList result;
    for(auto &it : m_platforms) {
        result.push_back(it);
    }
    return result;
}

void ProjectSettings::setPlatforms(const VariantList &platforms) {
    StringList list;
    for(auto &it : platforms) {
        TString platform(it.toString());
        if(std::find(list.begin(), list.end(), platform) == list.end()) {
            list.push_back(platform);
        }
    }
    if(list != m_platforms) {
        m_platforms = list;
        saveSettings();
    }
}
