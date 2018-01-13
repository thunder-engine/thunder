#include "builder.h"

#include "log.h"
#include "projectmanager.h"
#include "codemanager.h"

#include <QCoreApplication>

Builder::Builder() {
    connect(CodeManager::instance(), SIGNAL(buildSucess(QString)), this, SLOT(onCompileDone(QString)));
}

void Builder::convert() {

}

void Builder::archive() {
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
}

void Builder::onCompileDone(const QString &path) {
    QFileInfo info(path);
    QString target  = ProjectManager::instance()->targetPath() + "/" + info.fileName();
    qDebug() << QFile::remove(target);
    qDebug() << QFile::copy(path, target);
    QCoreApplication::quit();
}
