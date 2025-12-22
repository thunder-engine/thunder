#include <QCoreApplication>
#include <QCommandLineParser>

#include <engine.h>
#include <file.h>

#include <global.h>

#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>
#include <editor/editorsettings.h>
#include <editor/editorplatform.h>

#include "builder.h"

#include <iostream>

#include <log.h>

static bool succeed = true;

class ConsoleLog : public LogHandler {
public:
    void setRecord(Log::LogTypes type, const char *record) {
        std::string level;
        switch(type) {
            case Log::CRT: level = "[ critical ]"; break;
            case Log::ERR: level = "[ error ]"; break;
            case Log::WRN: level = "[ warning ]"; break;
            case Log::INF: level = "[ info ]"; break;
            case Log::DBG: level = "[ debug ]"; break;

            default: break;
        }

        std::cout << level << record << std::endl;
        if(type <= Log::ERR) {
            succeed = false;
            QCoreApplication::exit(1);
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setApplicationName(PRODUCT_NAME);
    QCoreApplication::setApplicationVersion(SDK_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("Thunder Engine Builder tool.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption sourceFileOption(QStringList() << "s" << "source",
                QCoreApplication::translate("main", "Project file <.forge>"),
                QCoreApplication::translate("main", "project"));
    parser.addOption(sourceFileOption);

    QCommandLineOption targetDirectoryOption(QStringList() << "t" << "target",
                QCoreApplication::translate("main", "Build Project into <directory>"),
                QCoreApplication::translate("main", "directory"));
    parser.addOption(targetDirectoryOption);

    QCommandLineOption platformOption(QStringList() << "p" << "platform",
                QCoreApplication::translate("main", "Specify the target <platform>"),
                QCoreApplication::translate("main", "platform"));
    parser.addOption(platformOption);

    parser.process(a);

    if(!parser.isSet(sourceFileOption) || !parser.isSet(targetDirectoryOption)) {
        parser.showHelp(1);
    }

    Log::setHandler(new ConsoleLog());
    Log::setLogLevel(Log::DBG);

    aInfo() << "Starting builder...";

    Engine engine(argv[0]);
    Engine::setPlatformAdaptor(&EditorPlatform::instance());

    ProjectSettings::instance()->init(parser.value(sourceFileOption).toStdString(), parser.value(targetDirectoryOption).toStdString());
    ProjectSettings::instance()->loadSettings();

    PluginManager::instance()->init(&engine);
    AssetManager::instance()->init();

    ProjectSettings::instance()->loadPlatforms();

    if(!PluginManager::instance()->rescanProject(ProjectSettings::instance()->pluginsPath())) {
        aWarning() << "Not all plugins were loaded.";
    }
    PluginManager::instance()->initSystems();

    Builder builder;

    builder.setPlatform(parser.value(platformOption).toStdString());
    if(!succeed) {
        return 1;
    }

    int result = a.exec();

    AssetManager::destroy();

    return result;
}
