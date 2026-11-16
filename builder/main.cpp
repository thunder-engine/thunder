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
#include <QTemporaryDir>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <engine.h>
#include <file.h>

#include <global.h>

#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>
#include <editor/editorsettings.h>
#include <editor/editorplatform.h>

#include "builder.h"
#include "rcc.h"

#include <log.h>

namespace {

struct BuilderOptions {
    std::string source;
    std::string target;
    std::string platform;
};

void printHelp(std::ostream &output, const char *program) {
    output << "Usage: " << program << " --target <directory> [--source <project>] [options]\n"
           << "Thunder Engine Builder tool.\n\n"
           << "Options:\n"
           << "  -h, --help                 Display this help.\n"
           << "  -v, --version              Display version information.\n"
           << "  -s, --source <project>     Project file <.forge> (required for game builds; omit for 'editor').\n"
           << "  -t, --target <directory>   Build project into <directory>.\n"
           << "  -p, --platform <platform>  Target platform; 'bundle' packages assets, 'editor' generates embedded editor resources.\n"
           << "      --rcc                  Run the resource compiler (must be the first argument).\n";
}

int parseOptions(int argc, char *argv[], BuilderOptions &options) {
    for(int i = 1; i < argc; ++i) {
        std::string argument(argv[i]);
        std::string option = argument;
        std::string value;
        bool hasValue = false;

        std::string::size_type separator = argument.find('=');
        if(separator != std::string::npos) {
            option = argument.substr(0, separator);
            value = argument.substr(separator + 1);
            hasValue = true;
        } else if(argument.size() > 2 && argument[0] == '-' && argument[1] != '-') {
            option = argument.substr(0, 2);
            value = argument.substr(2);
            if(!value.empty() && value[0] == '=') {
                value.erase(0, 1);
            }
            hasValue = true;
        }

        if(option == "-h" || option == "--help") {
            if(hasValue) {
                std::cerr << "Option " << option << " does not take a value.\n";
                return -1;
            }
            printHelp(std::cout, argv[0]);
            return 1;
        }
        if(option == "-v" || option == "--version") {
            if(hasValue) {
                std::cerr << "Option " << option << " does not take a value.\n";
                return -1;
            }
            std::cout << BUILDER_NAME << ' ' << SDK_VERSION << '\n';
            return 1;
        }

        std::string *destination = nullptr;
        if(option == "-s" || option == "--source") {
            destination = &options.source;
        } else if(option == "-t" || option == "--target") {
            destination = &options.target;
        } else if(option == "-p" || option == "--platform") {
            destination = &options.platform;
        } else if(option == "--rcc") {
            std::cerr << (hasValue ? "The --rcc option does not take a value.\n" :
                                     "The --rcc option must be the first argument.\n");
            return -1;
        } else {
            std::cerr << "Unknown option: " << argument << '\n';
            printHelp(std::cerr, argv[0]);
            return -1;
        }

        if(!hasValue) {
            if(i + 1 >= argc) {
                std::cerr << "Missing value for " << option << ".\n";
                printHelp(std::cerr, argv[0]);
                return -1;
            }
            value = argv[++i];
        }
        if(value.empty()) {
            std::cerr << "Empty value for " << option << ".\n";
            printHelp(std::cerr, argv[0]);
            return -1;
        }
        *destination = value;
    }

    if(options.target.empty() ||
       (options.platform != "editor" && options.source.empty()) ||
       (options.platform == "editor" && !options.source.empty())) {
        if(options.platform == "editor" && !options.source.empty()) {
            std::cerr << "The editor platform does not accept a project file.\n";
        }
        printHelp(std::cerr, argv[0]);
        return -1;
    }
    return 0;
}

}

int main(int argc, char *argv[]) {
    if(argc > 1 && std::string(argv[1]) == "--rcc") {
        std::vector<char *> arguments;
        arguments.reserve(argc - 1);
        arguments.push_back(argv[0]);
        for(int i = 2; i < argc; ++i) {
            arguments.push_back(argv[i]);
        }
        return runResourceCompiler(static_cast<int>(arguments.size()), arguments.data());
    }

    BuilderOptions options;
    const int parseResult = parseOptions(argc, argv, options);
    if(parseResult != 0) {
        return parseResult < 0 ? 1 : 0;
    }

    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setApplicationName(BUILDER_NAME);
    QCoreApplication::setApplicationVersion(SDK_VERSION);
    Log::setLogLevel(Log::DBG);

    Engine::setOrganizationName(COMPANY_NAME);
    Engine::setApplicationName(BUILDER_NAME);
    Engine::setApplicationVersion(TString(SDK_VERSION) + " rev " + REVISION);

    Engine engine;
    Engine::setPlatformAdaptor(&EditorPlatform::instance());

    Builder *builder = new Builder();
    Log::addHandler(builder);

    aInfo() << "Starting builder...";

    Editor::init(argc, argv);

    std::unique_ptr<QTemporaryDir> internalWorkspace;
    std::string projectPath = options.source;
    if(options.platform == "editor") {
        internalWorkspace.reset(new QTemporaryDir);
        if(!internalWorkspace->isValid()) {
            std::cerr << "Unable to create a temporary workspace for editor resources.\n";
            return 1;
        }
        projectPath = (internalWorkspace->path() + "/internal.forge").toStdString();
    }

    Editor::project()->init(projectPath, options.target);
    if(options.platform == "editor") {
        Editor::project()->setCurrentPlatform("editor");
        EditorPlatform::instance().setImportPath(Editor::project()->importPath());
    }

    // Strict after project initialization
    Editor::assets()->init();

    if(!Editor::plugins()->rescanProject(Editor::project()->pluginsPath())) {
        aWarning() << "Not all plugins were loaded.";
    }
    Editor::plugins()->initSystems();

    builder->setPlatform(options.platform);
    return builder->run();
}
