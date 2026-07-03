#include "editor/baseassetprovider.h"

#include <os/filesystemwatcher.h>

#include "editor/projectsettings.h"
#include "editor/assetmanager.h"
#include "editor/codebuilder.h"

#include <file.h>
#include <url.h>

#include "config.h"

/*!
    \class BaseAssetProvider
    \brief Manages asset discovery and lifecycle in the editor
    \module Editor

    This class monitors the project file system and manages asset operations including
    imports, registration, and removal. It works closely with AssetManager and
    ProjectSettings to maintain a consistent view of available assets.

    \note All paths must be passed as an absolute file paths.
*/

/*!
    Initializes the BaseAssetProvider and creates the FileSystemWatcher.
    Connects file system events to appropriate slot handlers for asset change detection.

    \sa init(), onFileChanged(), onDirectoryChanged()
*/
BaseAssetProvider::BaseAssetProvider() :
        m_dirWatcher(nullptr) {

    FileSystemWatcher::registerClassFactory(&Engine::instance());

    m_dirWatcher = Engine::objectCreate<FileSystemWatcher>();

    connect(m_dirWatcher, _SIGNAL(directoryChanged(TString)), this, _SLOT(onDirectoryChanged(TString)));
    connect(m_dirWatcher, _SIGNAL(fileChanged(TString)), this, _SLOT(onFileChanged(TString)));
}
/*!
    Cleans up the FileSystemWatcher and releases associated resources.
*/
BaseAssetProvider::~BaseAssetProvider() {
    delete m_dirWatcher;
}
/*!
    This method performs initial asset discovery and sets up directory watching for:
    Argument \a force If true, forces re-import of all assets; if false, only imports outdated assets

    \sa onDirectoryChangedForce()
*/
void BaseAssetProvider::init(bool force) {
    StringList paths = m_dirWatcher->directories();
    if(!paths.empty()) {
        m_dirWatcher->removePaths(paths);
    }

    ProjectSettings *mgr = ProjectSettings::instance();
    TString resourcePath(ProjectSettings::instance()->resourcePath());

    bool watch = false;
    onDirectoryChangedForce(resourcePath + "/engine", force);
    if(mgr->targetPath().isEmpty()) { // Skip for Builder
        onDirectoryChangedForce(resourcePath + "/editor", force);
        watch = true;
    }

    onDirectoryChangedForce(mgr->contentPath(), force, watch); // We need to watch only a project files in editor.
}
/*!
    Handles file \a path modification events by processing the file and notifying the
    AssetManager to trigger re-import if necessary.

    \sa onFileChangedForce(), AssetManager::fileChanged()
*/
void BaseAssetProvider::onFileChanged(const TString &path) {
    onFileChangedForce(path);

    AssetManager::instance()->fileChanged(path);
    AssetManager::instance()->reimport();
}
/*!
    Internal file change handler with \a force import option

    Checks if file exists and is not a metadata file (.meta extension).
    Accepts only absolute file \a path.
    If asset settings exist and are outdated or \a force is true, queues asset for import.
    Otherwise, registers the asset directly if it's not code.

    For non-code assets, also registers all sub-assets defined in the settings.

    \sa AssetManager::pushToImport(), AssetManager::registerAsset()
*/
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
/*!
    Slot called when directorycontents change

    Processes all files in the directory \a path and notifies AssetManager of the directory change.
    Files are processed with default (non-force) settings, importing only if they are outdated.

    \sa onDirectoryChangedForce(), onFileChangedForce()
*/
void BaseAssetProvider::onDirectoryChanged(const TString &path) {
    for(auto &item : File::list(path, false)) {
        if(File::isFile(item)) {
            onFileChangedForce(item, false);
        }
    }

    AssetManager::instance()->directoryChanged(path);
}
/*!
    Internal directory change handler with optional watching and force import

    Optionally adds the \a path to the file system \a watch. Recursively processes all files
    in the directory hierarchy, applying import settings based on \a force parameter.

    \sa FileSystemWatcher::addPath(), onFileChangedForce()
*/
void BaseAssetProvider::onDirectoryChangedForce(const TString &path, bool force, bool watch) {
    if(watch) {
        m_dirWatcher->addPath(path);
    }

    for(auto &item : File::list(path)) {
        if(File::isFile(item)) {
            onFileChangedForce(item, force);
        }
    }
}
/*!
    Removes an asset resource from the project

    Deletes an asset and its associated metadata. This operation includes:
    \list
    \li Unloading the resource from the engine
    \li Unregistering from AssetManager
    \li Removing imported asset data and thumbnails
    \li Removing metadata files
    \li If the asset is code, rebuilds the project
    \endlist

    \a source Absolute file path to the asset to remove. If empty, operation is skipped.

    \sa AssetManager::unregisterAsset(), BuilderSettings::builder()
*/
void BaseAssetProvider::removeResource(const TString &source) {
    if(source.isEmpty()) {
        return;
    }

    ProjectSettings *project = ProjectSettings::instance();
    AssetManager *asset = AssetManager::instance();

    Engine::unloadResource(asset->pathToLocal(source));
    TString uuid(asset->unregisterAsset(source));
    if(!uuid.isEmpty()) {
        File::remove(project->importPath() + "/" + uuid);
        File::remove(project->iconPath() + "/" + uuid + ".png");
    }

    File::remove(source + "." + gMetaExt);
    File::remove(source);

    BuilderSettings *settings = dynamic_cast<BuilderSettings *>(asset->fetchSettings(source));
    if(settings) {
        CodeBuilder *builder = settings->builder();
        if(builder) {
            builder->rescanSources(project->contentPath());
            builder->makeOutdated();
            builder->buildProject();
        }
    }

    asset->dumpBundle();
}
/*!
    Renames or moves an asset resource

    Handles renaming for both files and directories:
    \list
    \li For directories: Updates watcher paths, renames directory and all contained assets
    \li For files: Renames both the file and its metadata file
    \endlist

    Updates asset registry entries and calls asset converter to handle format-specific
    renaming (e.g., updating internal asset references).

    \a oldName Absolute path to the original asset
    \a newName Absolute path to the new location/name

    \sa AssetManager::registerAsset(), AssetConverter::renameAsset()
*/
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
                    back[path] = it->second;
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
/*!
    Creates a duplicate copy of an asset

    Creates a copy of an asset file and its metadata in the same directory.
    The duplicate is assigned a unique name using AssetManager::findFreeName().

    For non-code assets:
    \list
    \li Creates new settings for the duplicated asset
    \li Registers the asset with the original asset's information
    \endlist

    Updates the asset bundle after duplication.

    \a source Absolute path to the asset from project content directory

    \note The method automatically finds a free filename to avoid conflicts.

    \sa AssetManager::findFreeName(), AssetManager::registerAsset()
*/
void BaseAssetProvider::duplicateResource(const TString &source) {
    Url info(source);

    TString name = info.baseName();
    TString path = info.absoluteDir() + "/";
    TString suff;
    if(!info.suffix().isEmpty()) {
        suff = TString(".") + info.suffix();
    }

    TString freeName(name);
    AssetManager::findFreeName(freeName, path, suff);

    TString filePath(path + freeName + suff);

    if(!File::isDir(source)) {
        // Source and meta
        File::copy(source, filePath);
        File::copy(source + "." + gMetaExt, filePath + "." + gMetaExt);
    }

    AssetManager *asset = AssetManager::instance();
    AssetConverterSettings *targetSettings = asset->fetchSettings(filePath);
    if(targetSettings) {
        targetSettings->newSettings();
        targetSettings->saveSettings();

        if(!targetSettings->isCode()) {
            AssetConverterSettings *s = asset->fetchSettings(source);
            if(s) {
                asset->registerAsset(targetSettings->source(), targetSettings->info());
            }
        }

        asset->dumpBundle();
    }
}
