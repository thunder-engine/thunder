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

    QDirIterator it(ProjectSettings::instance()->importPath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(!info.isDir() && info.fileName() != gIndex && mgr->guidToPath(info.fileName().toStdString()).isEmpty()) {
            QFile::remove(info.absoluteFilePath());
        }
    }
}

void BaseAssetProvider::onFileChanged(const QString &path) {
    onFileChangedForce(path);
}

void BaseAssetProvider::onFileChangedForce(const QString &path, bool force) {
    AssetManager *mgr = AssetManager::instance();

    QFileInfo info(path);
    if(info.exists() && info.suffix() != gMetaExt) {
        AssetConverterSettings *settings = mgr->fetchSettings(path);
        if(settings) {
            if(force || settings->isOutdated()) {
                mgr->pushToImport(settings);
            } else {
                if(!settings->isCode()) {
                    QString guid = settings->destination();
                    mgr->registerAsset(path, guid, settings->typeName());
                    for(const QString &it : settings->subKeys()) {
                        mgr->registerAsset(info.absoluteFilePath() + "/" + it, settings->subItem(it), settings->subTypeName(it));
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
        QFileInfo info(item);
        if(info.suffix() == gMetaExt) {
            continue;
        }

        if(info.isDir()) {
            m_dirWatcher->addPath(info.absoluteFilePath());
            continue;
        }

        m_fileWatcher->addPath(info.absoluteFilePath());

        onFileChangedForce(item, force);
    }
}

void BaseAssetProvider::removeResource(const QString &source) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    QString src(project->contentPath() + "/" + source);
    QFileInfo info(src);
    if(info.isDir()) {
        m_dirWatcher->removePath(info.absoluteFilePath());

        QDir dir(project->contentPath());
        QDirIterator it(info.absoluteFilePath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        while(it.hasNext()) {
            removeResource(dir.relativeFilePath(it.next()));
        }
        QDir().rmdir(info.absoluteFilePath());
        return;
    } else {
        CodeBuilder *builder = nullptr;
        BuilderSettings *settings = dynamic_cast<BuilderSettings *>(asset->fetchSettings(src));
        if(settings) {
            builder = settings->builder();
        }
        m_fileWatcher->removePath(info.absoluteFilePath());
        Engine::unloadResource(source.toStdString());

        QString uuid = asset->unregisterAsset(source);
        QFile::remove(project->importPath() + "/" + uuid);
        QFile::remove(project->iconPath() + "/" + uuid + ".png");

        QFile::remove(info.absoluteFilePath() + "." + gMetaExt);
        QFile::remove(info.absoluteFilePath());

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

void BaseAssetProvider::renameResource(const QString &oldName, const QString &newName) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    QFileInfo src(project->contentPath() + "/" + oldName);
    QFileInfo dst(project->contentPath() + "/" + newName);

    ResourceSystem::DictionaryMap &indices(Engine::resourceSystem()->indices());

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
            QMap<QString, QString> back;

            for(auto guid = indices.cbegin(); guid != indices.cend();) {
                QString path = project->contentPath() + "/" + guid->first.data();
                if(path.startsWith(src.filePath())) {
                    back[path] = guid->second.second.data();
                    guid = indices.erase(guid);
                } else {
                    ++guid;
                }
            }

            QMapIterator<QString, QString> it(back);
            while(it.hasNext()) {
                it.next();
                QString newPath = it.key();
                newPath.replace(src.filePath(), dst.filePath());
                asset->registerAsset(newPath, it.value(), asset->assetTypeName(QFileInfo(it.value())));
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
            auto it = indices.find(oldName.toStdString());
            if(it != indices.end()) {
                QString guid = it->second.second.data();
                indices.erase(it);
                asset->registerAsset(dst.absoluteFilePath(), guid, asset->assetTypeName(QFileInfo(guid)));
                asset->dumpBundle();
            }

            AssetConverterSettings *settings = asset->fetchSettings(dst.filePath());
            if(settings) {
                AssetConverter *converter = asset->getConverter(dst.filePath());
                converter->renameAsset(settings, src.baseName(), dst.baseName());
            }
        }
    }
}

void BaseAssetProvider::duplicateResource(const QString &source) {
    AssetManager *asset = AssetManager::instance();
    ProjectSettings *project = ProjectSettings::instance();

    QFileInfo src(project->contentPath() + "/" + source);

    QString name = src.baseName();
    QString path = src.absolutePath() + "/";
    QString suff = !src.suffix().isEmpty() ? "." + src.suffix() : "";
    asset->findFreeName(name, path, suff);
    QFileInfo target(src.absoluteFilePath(), path + name + suff);
    if (src.isDir()) {
        copyRecursively(src.absoluteFilePath(), target.absoluteFilePath());
    } else {
        // Source and meta
        QFile::copy(src.absoluteFilePath(), target.filePath());
        QFile::copy(src.absoluteFilePath() + "." + gMetaExt, target.filePath() + "." + gMetaExt);
    }

    AssetConverterSettings *settings = asset->fetchSettings(target.filePath());
    if(settings) {
        QString guid = settings->destination();
        settings->setDestination(qPrintable(QUuid::createUuid().toString()));
        settings->setAbsoluteDestination(qPrintable(project->importPath() + "/" + settings->destination()));

        settings->saveSettings();

        if(!settings->isCode()) {
            AssetConverterSettings *s = asset->fetchSettings(src.filePath());
            if(s) {
                asset->registerAsset(settings->source(), settings->destination(), s->typeName());
            }
        }
        // Icon and resource
        QFile::copy(project->iconPath() + "/" + guid,
                    project->iconPath() + "/" + settings->destination()+ ".png");

        QFile::copy(project->importPath() + "/" + guid,
                    project->importPath() + "/" + settings->destination());

        asset->dumpBundle();
    }
}

// Copied from: https://forum.qt.io/topic/59245/is-there-any-api-to-recursively-copy-a-directory-and-all-it-s-sub-dirs-and-files/3
bool BaseAssetProvider::copyRecursively(const QString &sourceFolder, const QString &destFolder) {
    QDir sourceDir(sourceFolder);

    if(!sourceDir.exists()) {
        return false;
    }

    QDir destDir(destFolder);
    if(!destDir.exists()) {
        destDir.mkdir(destFolder);
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        if(!QFile::copy(srcName, destName)) {
            return false;
        }
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        if(!copyRecursively(srcName, destName)) {
            return false;
        }
    }

    return true;
}
