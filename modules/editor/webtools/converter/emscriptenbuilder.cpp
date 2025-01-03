#include "emscriptenbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>

#include <log.h>
#include <config.h>

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>
#include <editor/pluginmanager.h>

namespace {
    const char *gSdkPath("${sdkPath}");

    const char *gEmscriptenPath("Emscripten_Builder/Emscripten_SDK");

    const char *gLabel("[EmscriptenBuilder]");

};

EmscriptenBuilder::EmscriptenBuilder() :
        m_process(new QProcess(this)),
        m_progress(false) {

    EditorSettings *settings = EditorSettings::instance();

    settings->value(gEmscriptenPath, QVariant::fromValue(QFileInfo("/")));

    connect(settings, &EditorSettings::updated, this, &EmscriptenBuilder::onApplySettings);

    connect( m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
    connect( m_process, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );

    connect( m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int)) );

    ProjectSettings *mgr = ProjectSettings::instance();
    QString sdk = mgr->sdkPath();

    m_includePath = QStringList()
            << sdk + "/include/engine"
            << sdk + "/include/modules"
            << sdk + "/include/next"
            << sdk + "/include/next/math"
            << sdk + "/include/next/core";

    m_libPath = QStringList()
            << sdk + "/emscripten/x86/static";

    m_libs = QStringList() << "engine" << "next" << "physfs" << "zlib" << "glfm" <<
                              "bullet" << "bullet3" <<
                              "rendergl" << "freetype" << "uikit" <<
                              "media" << "vorbis" << "vorbisfile" << "ogg";
}

bool EmscriptenBuilder::buildProject() {
    if(m_outdated && !m_progress) {
        aInfo() << gLabel << "Build started.";

        onApplySettings();

        ProjectSettings *mgr = ProjectSettings::instance();

        m_project = mgr->generatedPath() + "/";

        m_process->setWorkingDirectory(m_project);

        generateProject();

        m_artifact = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/release";

        QDir dir;
        dir.mkpath(m_artifact);

        mgr->setArtifact(m_artifact);
        {
            QStringList args;

            for(auto it : m_includePath) {
                args.append("-I" + it);
            }

            for(auto it : m_libPath) {
                args.append("-L" + it);
            }

            for(auto it : m_libs) {
                args.append("-l" + it);
            }

            args.append("-std=c++20");
            args.append("-sMIN_WEBGL_VERSION=2");
            args.append("-sALLOW_MEMORY_GROWTH");
            args.append("-DTHUNDER_MOBILE");

            args.append("-O3");

            args.append("application.cpp");
            args.append("-o");
            args.append(m_artifact + "/application.js");

            args.append("--preload-file");
            args.append("../webgl/assets@/");

            m_process->start(m_binary, args);
            if(!m_process->waitForStarted()) {
                aError() << "Failed:" << qPrintable(m_process->errorString());
                return false;
            }
            m_progress = true;
        }
    }
    return true;
}

void EmscriptenBuilder::generateProject() {
    ProjectSettings *mgr = ProjectSettings::instance();

    aInfo() << gLabel << "Generating project";

    m_values[gSdkPath] = mgr->sdkPath();
    const QMetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        m_values[QString("${%1}").arg(property.name())] = property.read(mgr).toString();
    }

    generateLoader(mgr->templatePath(), mgr->modules());
}

QString EmscriptenBuilder::builderVersion() {
    m_process->start(m_binary, { "--version" });

    if(m_process->waitForStarted() && m_process->waitForFinished()) {
        return m_process->readAll().simplified();
    }
    return QString();
}

void EmscriptenBuilder::onBuildFinished(int exitCode) {
    ProjectSettings *mgr = ProjectSettings::instance();
    if(exitCode == 0) {
        QString targetFile(mgr->artifact() + "/application.html");

        qPrintable(targetFile);
        QFile::remove(targetFile);
        QFile::copy(":/application.html", targetFile);
        QFile::setPermissions(targetFile, QFileDevice::WriteOwner);

        aInfo() << gLabel << "Build finished";

        if(mgr->targetPath().isEmpty()) {
            emit buildSuccessful();
        }
    }
    m_outdated = false;
    m_progress = false;
}

void EmscriptenBuilder::readOutput() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardOutput());
    }
}

void EmscriptenBuilder::readError() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardError());
    }
}

void EmscriptenBuilder::onApplySettings() {
    QFileInfo info = EditorSettings::instance()->value(gEmscriptenPath).value<QFileInfo>();
    QString sdk = info.absoluteFilePath();

    sdk.replace('/', '\\');

    if(!info.exists()) {
        aCritical() << "Can't find the Emscripten SDK by the path:" << qPrintable(sdk);
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("EMSDK", sdk);
    env.insert("EMSDK_PYTHON", sdk + "\\python\\3.9.2-nuget_64bit\\python.exe");
    env.insert("EMSDK_NODE", sdk + "\\node\\16.20.0_64bit\\bin\\node.exe");
    env.insert("JAVA_HOME", sdk + "\\java\\8.152_64bit");

    m_binary = sdk + "\\upstream\\emscripten\\emcc.bat";

    QString path = env.value("PATH");
    path += ";" + sdk;
    path += ";" + sdk + "\\upstream\\emscripten";
    path += ";" + sdk + "\\node\\16.20.0_64bit\\bin";

    env.insert("PATH", path);

    m_process->setProcessEnvironment(env);
}

void EmscriptenBuilder::parseLogs(const QString &log) {
    QStringList list = log.split(QRegExp("[\r\n]"), Qt::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" error ") || it.contains(" error:", Qt::CaseInsensitive)) {
            aError() << gLabel << qPrintable(it);
        } else if(it.contains(" warning ") || it.contains(" warning:", Qt::CaseInsensitive)) {
            aWarning() << gLabel << qPrintable(it);
        } else {
            aInfo() << gLabel << qPrintable(it);
        }
    }
}

bool EmscriptenBuilder::isBundle(const QString &platform) const {
    if(platform == platforms().front()) {
        return true;
    }
    return false;
}
