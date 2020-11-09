#include <QCoreApplication>
#include <QCommandLineParser>

#include <engine.h>
#include <file.h>

#include <global.h>
#include "projectmanager.h"
#include "assetmanager.h"
#include "pluginmanager.h"

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

    QCommandLineOption platformOption(QStringList() << "p" << "platform",
                QCoreApplication::translate("main", "Specify the target <platform>."),
                QCoreApplication::translate("main", "platform"));
    parser.addOption(platformOption);

    parser.process(a);

    if(!parser.isSet(sourceFileOption) || !parser.isSet(targetDirectoryOption)) {
        parser.showHelp(1);
    }

    ProjectManager *mgr = ProjectManager::instance();
    mgr->init(parser.value(sourceFileOption), parser.value(targetDirectoryOption));

    Log::overrideHandler(new ConsoleLog());
    Log::setLogLevel(Log::DBG);
    File *file = new File();
    file->finit(qPrintable(QCoreApplication::arguments().at(0)));

    Log(Log::INF) << "Starting builder...";

    Engine engine(file, argv[0]);

    PluginManager::instance()->init(&engine);
    PluginManager::instance()->rescanPath(ProjectManager::instance()->pluginsPath());
    AssetManager::instance()->init(&engine);

    Builder builder;

    PluginManager::instance()->rescan();
    PluginManager::instance()->initSystems();

    builder.setPlatform(parser.value(platformOption));

    int result  = a.exec();

    AssetManager::destroy();
    PluginManager::destroy();

    return result;
}
