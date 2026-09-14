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
#include <QApplication>

#include <QSurfaceFormat>
#include <QSplashScreen>
#include <QFile>

#include "main/mainwindow.h"

#include "screens/projectbrowser/projectbrowser.h"

#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/editorplatform.h>

#include <engine.h>
#include <log.h>

#include <global.h>

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setVersion(4, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication::setKeyboardInputInterval(1);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setApplicationName(EDITOR_NAME);
    QCoreApplication::setApplicationVersion(SDK_VERSION);

    QFile qss(":/Style/styles/dark/style.qss");
    if(qss.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    // Need to set platform adaptor first
    Engine::setOrganizationName(COMPANY_NAME);
    Engine::setApplicationName(EDITOR_NAME);
    Engine::setApplicationVersion(TString(SDK_VERSION) + " rev " + REVISION);

    Log::setLogLevel(Log::DBG);

    Engine engine;
    Engine::setPlatformAdaptor(&EditorPlatform::instance());

    TString project;
    if(argc > 1) {
        project = QApplication::arguments().at(1).toStdString();
    } else {
        ProjectBrowser browser;
        if(browser.exec() == QDialog::Accepted) {
            project = browser.projectPath();
        }
    }

    if(project.isEmpty()) {
        return 0;
    }

    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();

    MainWindow window;

    splash.finish(&window);

    window.onOpenProject(project, engine);
    window.show();

    int result = app.exec();

    return result;
}
