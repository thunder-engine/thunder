#include "editor/baseassetprovider.h"

#include <os/filesystemwatcher.h>

#include "editor/projectsettings.h"
#include "editor/assetmanager.h"
#include "editor/codebuilder.h"

#include "config.h"

BaseAssetProvider::BaseAssetProvider() :
        m_dirWatcher(nullptr) {

    FileSystemWatcher::registerClassFactory(&Engine::instance());

    m_dirWatcher = Engine::objectCreate<FileSystemWatcher>();

    connect(m_dirWatcher, _SIGNAL(directoryChanged(TString)), this, _SLOT(onDirectoryChanged(TString)));
    connect(m_dirWatcher, _SIGNAL(fileChanged(TString)), this, _SLOT(onFileChanged(TString)));
}

BaseAssetProvider::~BaseAssetProvider() {
    delete m_dirWatcher;
}

void BaseAssetProvider::init(bool force) {
    StringList paths = m_dirWatcher->directories();
    if(!paths.empty()) {
        m_dirWatcher->removePaths(paths);
    }

    ProjectSettings *mgr = ProjectSettings::instance();
    TString resourcePath(ProjectSettings::instance()->resourcePath());

    onDirectoryChangedForce(resourcePath + "/engine/materials",force);
    onDirectoryChangedForce(resourcePath + "/engine/textures", force);
    onDirectoryChangedForce(resourcePath + "/engine/meshes",   force);
    onDirectoryChangedForce(resourcePath + "/engine/pipelines",force);
    onDirectoryChangedForce(resourcePath + "/engine/fonts",    force);
#ifndef BUILDER
    onDirectoryChangedForce(resourcePath + "/editor/materials",force);
    onDirectoryChangedForce(resourcePath + "/editor/gizmos",   force);
    onDirectoryChangedForce(resourcePath + "/editor/meshes",   force);
    onDirectoryChangedForce(resourcePath + "/editor/textures", force);
#endif
    onDirectoryChangedForce(mgr->contentPath(), force);
}

void BaseAssetProvider::onFileChanged(const TString &path) {
    onFileChangedForce(path);

    AssetManager::instance()->fileChanged(path.data());
    AssetManager::instance()->reimport();
}

void BaseAssetProvider::onFileChangedForce(const TString &path, bool force) {
    if(File::exists(path) && Url(path).suffix() != gMetaExt) {
        AssetManager *mgr = AssetManager::instance();
        AssetConverterSettings *settings = mgr->fetchSettings(path);
        if(settings) {
            if(force || settings->isOutdated()) {
                mgr->pushToImport(settings);
            } else {
                if(!settings->isCode()) {
                    mgr->registerAsset(path, settings->info());
                    for(const TString &it : settings->subKeys()) {
                        mgr->registerAsset(path + "/" + it, settings->subItem(it));
                    }
                }
            }
        }
    }
}

void BaseAssetProvider::onDirectoryChanged(const TString &path) {
    onDirectoryChangedForce(path);

    AssetManager::instance()->directoryChanged(path.data());
}

void BaseAssetProvider::onDirectoryChangedForce(const TString &path, bool force) {
    m_dirWatcher->addPath(path);

    for(auto &item : File::list(path)) {
        if(Url(item).suffix() == gMetaExt) {
            continue;
        }
        if(File::isDir(item)) {
            m_dirWatcher->addPath(item);
        } else {
            onFileChangedForce(item, force);
        }
    }
}

void BaseAssetProvider::removeResource(const TString &source) {
    if(source.isEmpty()) {
        return;
    }

    ProjectSettings *project = ProjectSettings::instance();
    AssetManager *asset = AssetManager::instance();
    TString src(project->contentPath() + "/" + source);

    Engine::unloadResource(source);

    TString uuid = asset->unregisterAsset(source);
    if(!uuid.isEmpty()) {
        File::remove(project->importPath() + "/" + uuid);
        File::remove(project->iconPath() + "/" + uuid + ".png");
    }

    File::remove(src + "." + gMetaExt);
    File::remove(src);

    CodeBuilder *builder = nullptr;
    BuilderSettings *settings = dynamic_cast<BuilderSettings *>(asset->fetchSettings(src));
    if(settings) {
        builder = settings->builder();
    }
    if(builder) {
        builder->rescanSources(project->contentPath());
        if(!builder->isEmpty()) {
            builder->makeOutdated();
            builder->buildProject();
        }
    }

    asset->dumpBundle();
}

void BaseAssetProvider::renameResource(const TString &oldName, const TString &newName) {
    AssetManager *asset = AssetManager::instance();

    ResourceSystem::Dictionary &indices(Engine::resourceSystem()->indices());

    if(File::isDir(oldName)) {
        StringList dirs = m_dirWatcher->directories();
        if(!dirs.empty()) {
            m_dirWatcher->removePaths(dirs);
            for(auto &it : dirs) {
                it.replace(oldName, newName);
            }
            m_dirWatcher->addPaths(dirs);
        }

        if(File::rename(oldName, newName)) {
            std::map<TString, ResourceSystem::ResourceInfo> back;

            ProjectSettings *project = ProjectSettings::instance();
            for(auto it = indices.cbegin(); it != indices.cend();) {
                TString path(project->contentPath() + "/" + it->first);
                if(path.startsWith(oldName)) {
                    back[path.toStdString()] = it->second;
                    it = indices.erase(it);
                } else {
                    ++it;
                }
            }

            for(auto &it : back) {
                TString newPath = it.first;
                newPath.replace(oldName, newName);
                asset->registerAsset(newPath, it.second);
            }
            asset->dumpBundle();
        } else {
            if(!dirs.empty()) {
                m_dirWatcher->addPaths(dirs);
            }
        }
    } else {
        if(File::rename(oldName, newName) &&
           File::rename(oldName + "." + gMetaExt, newName + "." + gMetaExt)) {
            auto it = indices.find(oldName);
            if(it != indices.end()) {
                ResourceSystem::ResourceInfo info = it->second; // To prevent data from deleteion in next line

                indices.erase(it);
                asset->registerAsset(newName, info);
                asset->dumpBundle();
            }

            AssetConverterSettings *settings = asset->fetchSettings(newName);
            if(settings) {
                AssetConverter *converter = asset->getConverter(newName);
                converter->renameAsset(settings, Url(oldName).baseName(), Url(newName).baseName());
            }
        }
    }
}

void BaseAssetProvider::duplicateResource(const TString &source) {
    AssetManager *asset = AssetManager::instance();

    TString src(ProjectSettings::instance()->contentPath() + "/" + source);

    Url info(src);

    TString name = info.baseName();
    TString path = info.absoluteDir() + "/";
    TString suff;
    if(!info.suffix().isEmpty()) {
        suff = TString(".") + info.suffix();
    }

    TString freeName(name);
    asset->findFreeName(freeName, path, suff);

    TString filePath(path + freeName + suff);

    if(!File::isDir(src)) {
        // Source and meta
        File::copy(src, filePath);
        File::copy(src + "." + gMetaExt, filePath + "." + gMetaExt);
    }

    AssetConverterSettings *targetSettings = asset->fetchSettings(filePath);
    if(targetSettings) {
        targetSettings->newSettings();
        targetSettings->saveSettings();

        if(!targetSettings->isCode()) {
            AssetConverterSettings *s = asset->fetchSettings(src);
            if(s) {
                asset->registerAsset(targetSettings->source(), targetSettings->info());
            }
        }

        asset->dumpBundle();
    }
}
