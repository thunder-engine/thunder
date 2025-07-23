#include "emscriptenbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>
#include <QRegularExpression>

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
        m_process(nullptr),
        m_progress(false) {

    EditorSettings *settings = EditorSettings::instance();

    settings->value(gEmscriptenPath, QVariant::fromValue(QFileInfo("/")));

    m_proxy = new EmscriptenProxy;
    m_proxy->setBuilder(this);

    m_process = new QProcess(m_proxy);

    QObject::connect(settings, &EditorSettings::updated, m_proxy, &EmscriptenProxy::onApplySettings);

    QObject::connect( m_process, &QProcess::readyReadStandardOutput, m_proxy, &EmscriptenProxy::readOutput );
    QObject::connect( m_process, &QProcess::readyReadStandardError, m_proxy, &EmscriptenProxy::readError );

    QObject::connect( m_process, SIGNAL(finished(int,QProcess::ExitStatus)), m_proxy, SLOT(onBuildFinished(int)) );

    ProjectSettings *mgr = ProjectSettings::instance();
    TString sdk = mgr->sdkPath().toStdString();

    m_includePath.push_back(sdk + "/include/engine");
    m_includePath.push_back(sdk + "/include/modules");
    m_includePath.push_back(sdk + "/include/next");
    m_includePath.push_back(sdk + "/include/next/math");
    m_includePath.push_back(sdk + "/include/next/core");

    m_libPath.push_back(sdk + "/emscripten/x86/static");

    m_libs.push_back("engine");
    m_libs.push_back("next");
    m_libs.push_back("physfs");
    m_libs.push_back("zlib");
    m_libs.push_back("glfm");
    m_libs.push_back("bullet");
    m_libs.push_back("bullet3");
    m_libs.push_back("rendergl");
    m_libs.push_back("freetype");
    m_libs.push_back("uikit");
    m_libs.push_back("media");
    m_libs.push_back("vorbis");
    m_libs.push_back("vorbisfile");
    m_libs.push_back("ogg");
    m_libs.push_back("angel");
    m_libs.push_back("angelscript");
}

bool EmscriptenBuilder::isEmpty() const {
    return (ProjectSettings::instance()->currentPlatformName() != "webgl");
}

bool EmscriptenBuilder::buildProject() {
    if(m_outdated && !m_progress) {
        aInfo() << gLabel << "Build started.";

        onApplySettings();

        ProjectSettings *mgr = ProjectSettings::instance();

        m_project = mgr->generatedPath().toStdString() + "/";

        m_process->setWorkingDirectory(m_project.data());

        generateProject();

        m_artifact = mgr->cachePath().toStdString() + "/" + mgr->currentPlatformName().toStdString() + "/release";

        QDir dir;
        dir.mkpath(m_artifact.data());

        mgr->setArtifact(m_artifact.data());
        {
            QStringList args;

            for(auto it : m_includePath) {
                args.append(QString("-I") + it.data());
            }

            for(auto it : m_libPath) {
                args.append(QString("-L") + it.data());
            }

            for(auto it : m_libs) {
                args.append(QString("-l") + it.data());
            }

            args.append("-std=c++20");
            args.append("-sMIN_WEBGL_VERSION=2");
            args.append("-sALLOW_MEMORY_GROWTH");
            args.append("-DTHUNDER_MOBILE");

            args.append("-O3");

            args.append("application.cpp");
            args.append("-o");
            args.append((m_artifact + "/application.js").data());

            args.append("--preload-file");
            args.append("../webgl/assets@/");

            m_process->start(m_binary.data(), args);
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

    m_values[gSdkPath] = mgr->sdkPath().toStdString();
    const QMetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        m_values[QString("${%1}").arg(property.name()).toStdString()] = property.read(mgr).toString().toStdString();
    }

    StringList list;
    for(auto it : mgr->modules()) {
        list.push_back(it.toStdString());
    }

    generateLoader(mgr->templatePath().toStdString(), list);
}

TString EmscriptenBuilder::builderVersion() {
    m_process->start(m_binary.data(), { "--version" });

    if(m_process->waitForStarted() && m_process->waitForFinished()) {
        return m_process->readAll().simplified().toStdString();
    }
    return TString();
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
            buildSuccessful();
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

    m_binary = sdk.toStdString() + "\\upstream\\emscripten\\emcc.bat";

    QString path = env.value("PATH");
    path += ";" + sdk;
    path += ";" + sdk + "\\upstream\\emscripten";
    path += ";" + sdk + "\\node\\16.20.0_64bit\\bin";

    env.insert("PATH", path);

    m_process->setProcessEnvironment(env);
}

void EmscriptenBuilder::parseLogs(const QString &log) {
    QStringList list = log.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);

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

bool EmscriptenBuilder::isBundle(const TString &platform) const {
    if(platform == platforms().front()) {
        return true;
    }
    return false;
}
