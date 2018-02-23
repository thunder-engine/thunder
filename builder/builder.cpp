#include "builder.h"

#include "log.h"
#include "projectmanager.h"
#include "codemanager.h"

#include <QCoreApplication>

Builder::Builder() {
    connect(AssetManager::instance(), &AssetManager::importFinished, this, &Builder::onImportDone);

    connect(CodeManager::instance(), SIGNAL(buildSucess(QString)), this, SLOT(onCompileDone(QString)));
    connect(CodeManager::instance(), &CodeManager::buildFailed, QCoreApplication::instance(), &QCoreApplication::quit);

    connect(this, &Builder::packDone, CodeManager::instance(), &CodeManager::rebuildProject);
    connect(this, &Builder::moveDone, QCoreApplication::instance(), &QCoreApplication::quit);
}

void Builder::onImportDone() {
    Log(Log::INF) << "Packaging Assets";

    QuaZip zip(ProjectManager::instance()->targetPath() + "/base.pak");
    if(!zip.open(QuaZip::mdCreate)) {
        Log(Log::ERR) << "Can't open package";
        return;
    }
    QuaZipFile outZipFile(&zip);

    QDirIterator it(ProjectManager::instance()->importPath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString path = it.next();
        QFileInfo info(path);
        if(info.isFile()){
            QFile inFile(info.absoluteFilePath());

            string origin   = AssetManager::instance()->guidToPath(info.fileName().toStdString());
            Log(Log::INF) << "\tCoping:" << origin.c_str();

            if(!inFile.open(QIODevice::ReadOnly)) {
                zip.close();
                Log(Log::ERR) << "Can't open input file";
                return;
            }

            if(!outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(info.fileName(), info.absoluteFilePath()))) {
                inFile.close();
                zip.close();
                Log(Log::ERR) << "Can't open output file";
                return;
            }
            outZipFile.write(inFile.readAll());

            outZipFile.close();
            inFile.close();
        }
    }
    Log(Log::INF) << "Packaging Done";
    emit packDone();
    CodeManager::instance()->setOutdated();
}

bool copyRecursively(QString sourceFolder, QString destFolder) {
    bool success = false;
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
        success = QFile::copy(srcName, destName);
        if(!success)
            return false;
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        success = copyRecursively(srcName, destName);
        if(!success) {
            return false;
        }
    }
    return true;
}

void Builder::onCompileDone(const QString &path) {
    QFileInfo info(path);
    QFileInfo target(ProjectManager::instance()->targetPath() + "/" + info.fileName());

    if((target.isDir() && QDir(target.absoluteFilePath()).removeRecursively()) || QFile::remove(target.absoluteFilePath())) {
        Log(Log::INF) << "Previous build removed.";
    }

    if(copyRecursively(path, target.absoluteFilePath())) {
        Log(Log::INF) << "New build copied to:" << qPrintable(target.absoluteFilePath());
        emit moveDone();
        return;
    }
    QCoreApplication::exit(1);
}
