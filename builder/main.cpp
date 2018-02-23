#include <QCoreApplication>
#include <QCommandLineParser>

#include <engine.h>
#include <file.h>

#include "common.h"
#include "projectmanager.h"
#include "assetmanager.h"
#include "codemanager.h"

#include "consolelog.h"

#include "builder.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setApplicationName(BUILDER_NAME);
    QCoreApplication::setApplicationVersion(SDK_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("Thunder Engine Builder tool.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption sourceFileOption(QStringList() << "s" << "source",
                QCoreApplication::translate("main", "Project file <.forge>."),
                QCoreApplication::translate("main", "project"));
    parser.addOption(sourceFileOption);

    QCommandLineOption targetDirectoryOption(QStringList() << "t" << "target",
                QCoreApplication::translate("main", "Build Project into <directory>."),
                QCoreApplication::translate("main", "directory"));
    parser.addOption(targetDirectoryOption);

    parser.process(a);

    ProjectManager *mgr = ProjectManager::instance();
    mgr->init(parser.value(sourceFileOption), parser.value(targetDirectoryOption));

    Log::overrideHandler(new ConsoleLog());
    Log::setLogLevel(Log::DBG);
    IFile *file = new IFile();
    file->finit(qPrintable(QCoreApplication::arguments().at(0)));
    file->fsearchPathAdd(qPrintable(mgr->importPath()), true);

    Engine engine(file);

    Builder builder;

    CodeManager::instance()->init();
    AssetManager::instance()->init();

    return a.exec();
}
