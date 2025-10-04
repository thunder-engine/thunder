#include "editor/baseassetprovider.h"

#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QUuid>

#include "editor/projectsettings.h"
#include "editor/assetmanager.h"
#include "editor/codebuilder.h"

#include "config.h"

BaseAssetProvider::BaseAssetProvider() :
        m_dirWatcher(new QFileSystemWatcher(this)),
        m_fileWatcher(new QFileSystemWatcher(this)) {

}

BaseAssetProvider::~BaseAssetProvider() {
    delete m_dirWatcher;
    delete m_fileWatcher;
}

void BaseAssetProvider::init() {
    QStringList paths = m_dirWatcher->directories();
    if(!paths.isEmpty()) {
        m_dirWatcher->removePaths(paths);
    }

    connect(m_dirWatcher, &QFileSystemWatcher::directoryChanged, this, &BaseAssetProvider::onDirectoryChanged);
    connect(m_dirWatcher, SIGNAL(directoryChanged(QString)), AssetManager::instance(), SIGNAL(directoryChanged(QString)));
    connect(m_dirWatcher, SIGNAL(directoryChanged(QString)), AssetManager::instance(), SLOT(reimport()));

    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &BaseAssetProvider::onFileChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, AssetManager::instance(), &AssetManager::fileChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, AssetManager::instance(), &AssetManager::reimport);
}

void BaseAssetProvider::cleanupBundle() {
    AssetManager *mgr = AssetManager::instance();

    for(auto &path : File::list(ProjectSettings::instance()->importPath())) {
        TString fileName(Url(path).name());
        if(!File::isDir(path) && fileName != gIndex && mgr->uuidToPath(fileName).isEmpty()) {
            File::remove(path);
        }
    }
}

void BaseAssetProvider::onFileChanged(const QString &path) {
    onFileChangedForce(path);
}

void BaseAssetProvider::onFileChangedForce(const QString &path, bool force) {
    AssetManager *mgr = AssetManager::instance();

    TString filePath(path.toStdString());
    if(File::exists(filePath) && Url(path.toStdString()).suffix() != gMetaExt) {
        AssetConverterSettings *settings = mgr->fetchSettings(filePath);
        if(settings) {
            if(force || settings->isOutdated()) {
                mgr->pushToImport(settings);
            } else {
                if(!settings->isCode()) {
                    TString uuid = settings->destination();
                    mgr->registerAsset(filePath, uuid, settings->typeName(), settings->hash());
                    for(const TString &it : settings->subKeys()) {
                        mgr->registerAsset(filePath + "/" + it, settings->subItem(it), settings->subTypeName(it), settings->hash());
                    }
                }
            }
        }
    }
}

void BaseAssetProvider::onDirectoryChanged(const QString &path) {
    onDirectoryChangedForce(path);
}

void BaseAssetProvider::onDirectoryChangedForce(const QString &path, bool force) {
    m_dirWatcher->addPath(path);

    for(auto &item : File::list(path.toStdString())) {
        if(Url(item).suffix() == gMetaExt) {
            continue;
        }

        if(File::isDir(item)) {
            m_dirWatcher->addPath(item.data());
            continue;
        }

        m_fileWatcher->addPath(item.data());

        onFileChangedForce(item.data(), force);
    }
}

void BaseAssetProvider::removeResource(const TString &source) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    TString src(project->contentPath() + "/" + source);
    if(File::isDir(src)) {
        m_dirWatcher->removePath(src.data());

        QDir dir(project->contentPath().data());
        QDirIterator it(src.data(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        while(it.hasNext()) {
            removeResource(dir.relativeFilePath(it.next()).toStdString());
        }
        QDir().rmdir(src.data());
        return;
    } else {
        CodeBuilder *builder = nullptr;
        BuilderSettings *settings = dynamic_cast<BuilderSettings *>(asset->fetchSettings(src));
        if(settings) {
            builder = settings->builder();
        }
        m_fileWatcher->removePath(src.data());
        Engine::unloadResource(source);

        TString uuid = asset->unregisterAsset(source);
        File::remove(project->importPath() + "/" + uuid);
        File::remove(project->iconPath() + "/" + uuid + ".png");

        File::remove(src + "." + gMetaExt);
        File::remove(src);

        if(builder) {
            builder->rescanSources(project->contentPath());
            if(!builder->isEmpty()) {
                builder->makeOutdated();
                builder->buildProject();
            }
        }
    }

    asset->dumpBundle();
}

void BaseAssetProvider::renameResource(const TString &oldName, const TString &newName) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    TString src(project->contentPath() + "/" + oldName);
    TString dst(project->contentPath() + "/" + newName);

    ResourceSystem::Dictionary &indices(Engine::resourceSystem()->indices());

    if(File::isDir(src)) {
        QStringList dirs = m_dirWatcher->directories();
        QStringList files = m_fileWatcher->files();
        if(!dirs.isEmpty()) {
            m_dirWatcher->removePaths(dirs);
            m_dirWatcher->addPaths(dirs.replaceInStrings(src.data(), dst.data()));
        }
        if(!files.isEmpty()) {
            m_fileWatcher->removePaths(files);
            m_fileWatcher->addPaths(files.replaceInStrings(src.data(), dst.data()));
        }

        QDir dir;
        if(dir.rename(src.data(), dst.data())) {
            std::map<TString, ResourceSystem::ResourceInfo> back;

            for(auto it = indices.cbegin(); it != indices.cend();) {
                QString path = (project->contentPath() + "/" + it->first).data();
                if(path.startsWith(src.data())) {
                    back[path.toStdString()] = it->second;
                    it = indices.erase(it);
                } else {
                    ++it;
                }
            }

            for(auto it : back) {
                TString newPath = it.first;
                newPath.replace(src, dst);
                asset->registerAsset(newPath, it.second.uuid, it.second.type, it.second.md5);
            }
            asset->dumpBundle();
        } else {
            if(!dirs.isEmpty()) {
                m_dirWatcher->addPaths(dirs);
            }
            if(!files.isEmpty()) {
                m_fileWatcher->addPaths(files);
            }
        }
    } else {
        if(File::rename(src, dst) &&
           File::rename(src + "." + gMetaExt, dst + "." + gMetaExt)) {
            auto it = indices.find(oldName);
            if(it != indices.end()) {
                TString md5 = it->second.md5;
                indices.erase(it);
                asset->registerAsset(dst, it->second.uuid, asset->assetTypeName(oldName), md5);
                asset->dumpBundle();
            }

            AssetConverterSettings *settings = asset->fetchSettings(dst);
            if(settings) {
                AssetConverter *converter = asset->getConverter(dst);
                converter->renameAsset(settings, Url(src).baseName(), Url(src).baseName());
            }
        }
    }
}

void BaseAssetProvider::duplicateResource(const TString &source) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    TString src(project->contentPath() + "/" + source);

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

    if(File::isDir(src)) {
        copyRecursively(src, filePath);
    } else {
        // Source and meta
        File::copy(src, filePath);
        File::copy(src + "." + gMetaExt, filePath + "." + gMetaExt);
    }

    AssetConverterSettings *targetSettings = asset->fetchSettings(filePath);
    if(targetSettings) {
        TString uuid = asset->fetchSettings(src)->destination();
        targetSettings->setAbsoluteDestination(project->importPath() + "/" + targetSettings->destination());

        targetSettings->saveSettings();

        if(!targetSettings->isCode()) {
            AssetConverterSettings *s = asset->fetchSettings(src);
            if(s) {
                asset->registerAsset(targetSettings->source(), targetSettings->destination(), s->typeName(), s->hash());
            }
        }
        // Icon
        TString iconPath(project->iconPath() + "/" + uuid + ".png");
        if(File::exists(iconPath)) {
            File::copy(iconPath, project->iconPath() + "/" + targetSettings->destination() + ".png");
        }

        // Resource
        TString resourcePath(project->importPath() + "/" + uuid);
        if(File::exists(resourcePath)) {
            File::copy(resourcePath, project->importPath() + "/" + targetSettings->destination());
        }

        asset->dumpBundle();
    }
}

// Copied from: https://forum.qt.io/topic/59245/is-there-any-api-to-recursively-copy-a-directory-and-all-it-s-sub-dirs-and-files/3
bool BaseAssetProvider::copyRecursively(const TString &sourceFolder, const TString &destFolder) {
    QDir sourceDir(sourceFolder.data());

    if(!sourceDir.exists()) {
        return false;
    }

    QDir destDir(destFolder.data());
    if(!destDir.exists()) {
        destDir.mkdir(destFolder.data());
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++) {
        TString srcName = sourceFolder + "/" + files[i].toStdString();
        TString destName = destFolder + "/" + files[i].toStdString();
        if(!QFile::copy(srcName.data(), destName.data())) {
            return false;
        }
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i < files.count(); i++) {
        TString srcName = sourceFolder + "/" + files[i].toStdString();
        TString destName = destFolder + "/" + files[i].toStdString();
        if(!copyRecursively(srcName, destName)) {
            return false;
        }
    }

    return true;
}
