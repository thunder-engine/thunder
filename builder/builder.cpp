#include "builder.h"

#include <log.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/assetmanager.h>
#include <editor/codebuilder.h>

#include <quazip.h>
#include <quazipfile.h>

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
    QFileInfo info(target.data());
    QString dir = info.absolutePath();
#if defined(Q_OS_MAC)
    dir = target.data();
    if(ProjectSettings::instance()->currentPlatformName() == "desktop") {
        dir += "/Contents/MacOS";
    }
#endif
    dir += "/base.pak";

    aInfo() << "Packaging Assets to:" << qPrintable(dir);
    QuaZip zip(dir);
    if(!zip.open(QuaZip::mdCreate)) {
        aError() << "Can't open package.";
        return;
    }
    QuaZipFile outZipFile(&zip);

    QDirIterator it(ProjectSettings::instance()->importPath().data(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString path = it.next();
        QFileInfo info(path);
        if(info.isFile()) {
            QFile inFile(info.absoluteFilePath());

            TString origin = AssetManager::instance()->guidToPath(info.fileName().toStdString());
            aInfo() << "\tCoping:" << origin.data();

            if(!inFile.open(QIODevice::ReadOnly)) {
                zip.close();
                aError() << "Can't open input file.";
                return;
            }

            if(!outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(info.fileName(), info.absoluteFilePath()))) {
                inFile.close();
                zip.close();
                aError() << "Can't open output file.";
                return;
            }
            outZipFile.write(inFile.readAll());

            outZipFile.close();
            inFile.close();
        }
    }
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
    QFileInfo info(path.data());
    TString targetPath = project->targetPath() + "/" + platform + "/";

    QDir dir;
    dir.mkpath(targetPath.data());
    QFileInfo target(QString(targetPath.data()) + info.fileName());

    if((target.isDir() && QDir(target.absoluteFilePath()).removeRecursively()) || QFile::remove(target.absoluteFilePath())) {
        aInfo() << "Previous build removed.";
    }

    if((info.isDir() && copyRecursively(path.data(), target.absoluteFilePath())) || QFile::copy(path.data(), target.absoluteFilePath())) {
        aInfo() << "New build copied to:" << qPrintable(target.absoluteFilePath());

        if(!project->currentBuilder()->isBundle(platform)) {
            package(target.absoluteFilePath().toStdString());

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
