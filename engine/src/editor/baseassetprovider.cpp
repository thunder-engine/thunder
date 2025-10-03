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

    QDirIterator it(ProjectSettings::instance()->importPath().data(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        TString path(it.next().toStdString());
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
    if(File::exists(filePath) && QFileInfo(path).suffix() != gMetaExt) {
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

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString item = it.next();
        if(QFileInfo(item).suffix() == gMetaExt) {
            continue;
        }

        if(File::isDir(item.toStdString())) {
            m_dirWatcher->addPath(item);
            continue;
        }

        m_fileWatcher->addPath(item);

        onFileChangedForce(item, force);
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

    QFileInfo src((project->contentPath() + "/" + oldName).data());
    QFileInfo dst((project->contentPath() + "/" + newName).data());

    ResourceSystem::Dictionary &indices(Engine::resourceSystem()->indices());

    if(src.isDir()) {
        QStringList dirs = m_dirWatcher->directories();
        QStringList files = m_fileWatcher->files();
        if(!dirs.isEmpty()) {
            m_dirWatcher->removePaths(dirs);
            m_dirWatcher->addPaths(dirs.replaceInStrings(src.absoluteFilePath(), dst.absoluteFilePath()));
        }
        if(!files.isEmpty()) {
            m_fileWatcher->removePaths(files);
            m_fileWatcher->addPaths(files.replaceInStrings(src.absoluteFilePath(), dst.absoluteFilePath()));
        }

        QDir dir;
        if(dir.rename(src.absoluteFilePath(), dst.absoluteFilePath())) {
            std::map<TString, ResourceSystem::ResourceInfo> back;

            for(auto it = indices.cbegin(); it != indices.cend();) {
                QString path = (project->contentPath() + "/" + it->first).data();
                if(path.startsWith(src.filePath())) {
                    back[path.toStdString()] = it->second;
                    it = indices.erase(it);
                } else {
                    ++it;
                }
            }

            for(auto it : back) {
                TString newPath = it.first;
                newPath.replace(src.filePath().toStdString(), dst.filePath().toStdString());
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
        if(QFile::rename(src.absoluteFilePath(), dst.absoluteFilePath()) &&
           QFile::rename(src.absoluteFilePath() + "." + gMetaExt, dst.absoluteFilePath() + "." + gMetaExt)) {
            auto it = indices.find(oldName);
            if(it != indices.end()) {
                TString md5 = it->second.md5;
                indices.erase(it);
                asset->registerAsset(dst.absoluteFilePath().toStdString(), it->second.uuid, asset->assetTypeName(oldName), md5);
                asset->dumpBundle();
            }

            AssetConverterSettings *settings = asset->fetchSettings(dst.filePath().toStdString());
            if(settings) {
                AssetConverter *converter = asset->getConverter(dst.filePath().toStdString());
                converter->renameAsset(settings, qPrintable(src.baseName()), qPrintable(dst.baseName()));
            }
        }
    }
}

void BaseAssetProvider::duplicateResource(const TString &source) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    QFileInfo src((project->contentPath() + "/" + source).data());

    QString name = src.baseName();
    QString path = src.absolutePath() + "/";
    QString suff = !src.suffix().isEmpty() ? "." + src.suffix() : "";
    TString freeName(name.toStdString());
    asset->findFreeName(freeName, path.toStdString(), suff.toStdString());
    QFileInfo target(src.absoluteFilePath(), path + name + suff);
    if(src.isDir()) {
        copyRecursively(src.absoluteFilePath().toStdString(), target.absoluteFilePath().toStdString());
    } else {
        // Source and meta
        QFile::copy(src.absoluteFilePath(), target.filePath());
        QFile::copy(src.absoluteFilePath() + "." + gMetaExt, target.filePath() + "." + gMetaExt);
    }

    AssetConverterSettings *targetSettings = asset->fetchSettings(target.filePath().toStdString());
    if(targetSettings) {
        TString uuid = targetSettings->destination();
        targetSettings->setDestination(QUuid::createUuid().toString().toStdString());
        targetSettings->setAbsoluteDestination(project->importPath() + "/" + targetSettings->destination());

        targetSettings->saveSettings();

        if(!targetSettings->isCode()) {
            AssetConverterSettings *s = asset->fetchSettings(src.filePath().toStdString());
            if(s) {
                asset->registerAsset(targetSettings->source(), targetSettings->destination(), s->typeName(), s->hash());
            }
        }
        // Icon and resource
        QFile::copy((project->iconPath() + "/" + uuid).data(),
                    (project->iconPath() + "/" + targetSettings->destination() + ".png").data());

        QFile::copy((project->importPath() + "/" + uuid).data(),
                    (project->importPath() + "/" + targetSettings->destination()).data());

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
