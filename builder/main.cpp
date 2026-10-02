/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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

#include <log.h>

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

    Log::setLogLevel(Log::DBG);

    Engine::setOrganizationName(COMPANY_NAME);
    Engine::setApplicationName(BUILDER_NAME);
    Engine::setApplicationVersion(TString(SDK_VERSION) + " rev " + REVISION);

    Engine engine;
    Engine::setPlatformAdaptor(&EditorPlatform::instance());

    Editor editor(argc, argv);
    Builder *builder = new Builder();
    Log::addHandler(builder);

    aInfo() << "Starting builder...";

    Editor::project()->init(parser.value(sourceFileOption).toStdString(), parser.value(targetDirectoryOption).toStdString());

    Editor::plugins()->init(&engine);
    Editor::assets()->init();

    if(!Editor::plugins()->rescanProject(Editor::project()->pluginsPath())) {
        aWarning() << "Not all plugins were loaded.";
    }
    Editor::plugins()->initSystems();

    builder->setPlatform(parser.value(platformOption).toStdString());
    return builder->run();
}
