#include "builder.h"

#include <log.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/assetmanager.h>
#include <editor/codebuilder.h>

#include <minizip/zip.h>

#include <QCoreApplication>
#include <QDirIterator>

Builder::Builder() {
    connect(AssetManager::instance(), &AssetManager::importFinished, this, &Builder::onImportFinished, Qt::QueuedConnection);
}

void Builder::setPlatform(const TString &platform) {
    ProjectSettings *project = ProjectSettings::instance();
    EditorSettings::instance()->loadSettings();
    if(platform.isEmpty()) {
        for(const TString &it : project->platforms()) {
            m_platformsToBuild.push(it);
        }
    } else {
        m_platformsToBuild.push(platform);
    }

    if(!m_platformsToBuild.empty()) {
        project->setCurrentPlatform(m_platformsToBuild.top());
        m_platformsToBuild.pop();

        CodeBuilder *builder = project->currentBuilder();
        if(builder) {
            builder->convertFile(nullptr);
        }

        AssetManager::instance()->rescan();
    }
}

void Builder::package(const TString &target) {
    Url info(target);
    TString pak = info.absoluteDir();
#if defined(Q_OS_MAC)
    pak = target;
    if(ProjectSettings::instance()->currentPlatformName() == "desktop") {
        pak += "/Contents/MacOS";
    }
#endif
    pak += "/base.pak";

    aInfo() << "Packaging Assets to:" << pak;

    zipFile zf = zipOpen(pak.data(), 0);
    if(!zf) {
        aError() << "Can't open package.";
        return;
    }

    StringList list(File::list(ProjectSettings::instance()->importPath()));
    for(auto &it : list) {
        if(File::isFile(it)) {
            Url info(it);

            TString origin = AssetManager::instance()->uuidToPath(info.baseName());
            aInfo() << "\tCoping:" << origin.data();

            File inFile(it);
            if(!inFile.open(File::ReadOnly)) {
                zipClose(zf, nullptr);
                aError() << "Can't open input file.";
                return;
            }

            zip_fileinfo zi = {0};
            zipOpenNewFileInZip(zf, info.name().data(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_NO_COMPRESSION);

            ByteArray data(inFile.readAll());
            inFile.close();

            zipWriteInFileInZip(zf, data.data(), data.size());
            zipCloseFileInZip(zf);
        }
    }

    zipClose(zf, nullptr);
}

bool copyRecursively(QString sourceFolder, QString destFolder) {
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
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        if(!QFile::copy(srcName, destName)) {
            return false;
        }
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        if(!copyRecursively(srcName, destName)) {
            return false;
        }
    }
    return true;
}

void Builder::onImportFinished() {
    ProjectSettings *project = ProjectSettings::instance();
    TString platform = project->currentPlatformName();
    TString path = project->artifact();
    Url info(path.data());
    TString targetPath = project->targetPath() + "/" + platform + "/";

    QDir dir;
    dir.mkpath(targetPath.data());
    TString target(targetPath + info.name());

    bool isDir = File::isDir(target);
    if((isDir && QDir(target.data()).removeRecursively()) || File::remove(target)) {
        aInfo() << "Previous build removed.";
    }

    if(isDir && copyRecursively(path.data(), target.data())) {
        File::copy(path, target);

        aInfo() << "New build copied to:" << target;

        if(!project->currentBuilder()->isBundle(platform)) {
            package(target);

            aInfo() << "Packaging Done.";
        }

        if(!m_platformsToBuild.empty()) {
            project->setCurrentPlatform(m_platformsToBuild.top());
            m_platformsToBuild.pop();
            AssetManager::instance()->rescan();

            return;
        }
    }

    QCoreApplication::exit(0);
}
