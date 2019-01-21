#include "qbsbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>

#include <log.h>
#include <config.h>

#include <projectmanager.h>
#include <pluginmodel.h>

QRegExp gClass("A_REGISTER\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");

const QString gRegisterComponents("${RegisterComponents}");
const QString gUnregisterComponents("${UnregisterComponents}");
const QString gComponentNames("${ComponentNames}");
const QString gIncludes("${Includes}");

const QString gFilesList("${filesList}");

const QString gSdkPath("${sdkPath}");

const QString gIncludePaths("${includePaths}");
const QString gLibraryPaths("${libraryPaths}");
const QString gLibraries("${libraries}");

const QString gEditorSuffix("-Editor");

#ifndef _DEBUG
    const QString gMode = "release";
#else
    const QString gMode = "debug";
#endif

// generate --generator visualstudio2013

QbsBuilder::QbsBuilder() :
        IBuilder(),
        m_Progress(false) {

    m_pMgr      = ProjectManager::instance();

    m_Suffix    = gShared;
    if(!m_pMgr->targetPath().isEmpty()) {
        m_Suffix= gApplication;
    }

    m_Project   = m_pMgr->generatedPath() + "/";

    setEnvironment(QStringList(), QStringList(), QStringList());

    const QMetaObject *meta = m_pMgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        m_Values[QString("${%1}").arg(property.name())]   = property.read(m_pMgr).toString();
    }

    m_Settings << "--settings-dir" << QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/..";

    if(m_pMgr->targetPath().isEmpty()) {
        m_Artifact  = m_Project + gMode + "/install-root/" + m_pMgr->projectName() + gEditorSuffix + m_Suffix;
    } else {
        m_Artifact  = m_Project + gMode + "/install-root/" + m_pMgr->projectName() + m_Suffix;
    }

    AssetManager::instance()->setArtifact(m_Artifact);

    m_pProcess  = new QProcess(this);
    m_pProcess->setWorkingDirectory(m_Project);

    connect( m_pProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
    connect( m_pProcess, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );

    connect( m_pProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int)) );

#if defined(Q_OS_WIN)
    m_Profiles << "MSVC2015-x86";
#elif defined(Q_OS_MAC)
    m_Profiles << "xcode-macosx-x86_64";
#elif defined(Q_OS_UNIX)
    m_Profiles << "clang";
#endif
    m_Profiles << "Android";

    if(!checkProfile(m_Profiles[0])) {
        Log(Log::INF) << "Initializing QBS...";
        builderInit();
    }
}

void QbsBuilder::generateProject() {
    QStringList code = rescanSources(m_pMgr->contentPath());

    StringMap classes;
    // Generate plugin loader
    foreach(QString it, code) {
        QFile file(it);
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray data = file.readLine();
            bool valid      = true;
            while(!data.isEmpty()) {
                if(!valid && data.indexOf("*/") != -1) {
                    valid = true;
                }
                int comment = data.indexOf("/*");
                if(comment == -1) {
                    int comment = data.indexOf("//");
                    int index   = gClass.indexIn(QString(data));
                    if(valid && index != -1 && !gClass.cap(1).isEmpty() && (comment == -1 || comment > index)) {
                        classes[gClass.cap(1)] = it;
                    }
                } else if(data.indexOf("*/", comment + 2) == -1) {
                    valid   = false;
                }
                data = file.readLine();
            }
            file.close();
        }
    }

    StringMap values(m_Values);

    values[gRegisterComponents]     = "";
    values[gUnregisterComponents]   = "";
    values[gComponentNames]         = "";

    const QMetaObject *meta = m_pMgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        values[QString("${%1}").arg(property.name())]   = property.read(m_pMgr).toString();
    }

    QStringList includes;
    QMapIterator<QString, QString> it(classes);
    while(it.hasNext()) {
        it.next();
        includes << "#include \"" + it.value() + "\"\n";
        values[gRegisterComponents].append(it.key() + "::registerClassFactory(&system);\n\t\t");
        values[gUnregisterComponents].append(it.key() + "::unregisterClassFactory(&system);\n\t\t");
        values[gComponentNames].append("result.push_back(\"" + it.key() + "\");\n\t\t");
    }
    includes.removeDuplicates();
    values[gIncludes].append(includes.join(""));

    copyTemplate(m_pMgr->templatePath() + "/plugin.cpp", project() + "plugin.cpp", values);
    copyTemplate(m_pMgr->templatePath() + "/application.cpp", project() + "application.cpp", values);

    values[gSdkPath]        = m_pMgr->sdkPath();
    values[gIncludePaths]   = formatList(m_IncludePath);
    values[gLibraryPaths]   = formatList(m_LibPath);
    values[gFilesList]      = formatList(code);
    values[gLibraries]      = formatList(m_Libs);

    copyTemplate(m_pMgr->templatePath() + "/project.qbs", m_Project + m_pMgr->projectName() + ".qbs", values);
}

bool QbsBuilder::buildProject() {
    if(m_Outdated && !m_Progress) {
        generateProject();

        QString product = m_pMgr->projectName();
        if(m_pMgr->targetPath().isEmpty()) {
            product    += gEditorSuffix;
        }
        {
            QProcess qbs(this);
            qbs.setWorkingDirectory(m_Project);
            qbs.start(m_pMgr->qbsPath(), QStringList() << "resolve" << gMode << "profile:" + m_Profiles[0]);
            if(qbs.waitForStarted()) {
                qbs.waitForFinished();
                Log(Log::INF) << "Resolved:" << qbs.readAll().constData();
            }
        }
        {
            QStringList args;
            args << "build" << m_Settings;
            args << "--products" << product << gMode << "profile:" + m_Profiles[0];
            m_pProcess->start(m_pMgr->qbsPath(), args);
            if(!m_pProcess->waitForStarted()) {
                Log(Log::ERR) << "Failed:" << qPrintable(m_pProcess->errorString()) << qPrintable(m_pMgr->qbsPath());
                return false;
            }
            m_Progress = true;
        }
    }

    return true;
}

void QbsBuilder::onBuildFinished(int exitCode) {
    if(exitCode == 0) {
        PluginModel::instance()->reloadPlugin(m_Artifact);
    }
    m_Outdated = false;
    m_Progress = false;
}

QString QbsBuilder::builderVersion() {
    QProcess qbs(this);
    qbs.setWorkingDirectory(m_Project);
    qbs.start(m_pMgr->qbsPath(), QStringList() << "--version" );
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        return qbs.readAll().simplified();
    }
    return QString();
}

void QbsBuilder::builderInit() {
    {
        QProcess qbs(this);
        qbs.setWorkingDirectory(m_Project);
        qbs.start(m_pMgr->qbsPath(), QStringList() << "setup-toolchains" << "--detect" << m_Settings);
        if(qbs.waitForStarted()) {
            qbs.waitForFinished();
            Log(Log::INF) << "Found:" << qbs.readAll().constData();
        }
    }
    {
        QStringList args;
        args << "setup-android" << m_Settings;
        args << "--sdk-dir" << "D:/Environment/Android/sdk";
        args << "--ndk-dir" << "D:/Environment/Android/sdk/ndk-bundle";
        //D:/Environment/Android/sdk/ndk-bundle/sysroot
        args << "Android";

        QProcess qbs(this);
        qbs.setWorkingDirectory(m_Project);
        qbs.start(m_pMgr->qbsPath(), args);
        if(qbs.waitForStarted()) {
            qbs.waitForFinished();
        }
    }
}

void QbsBuilder::readOutput() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardOutput());
    }
}

void QbsBuilder::readError() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardError());
    }
}

QString QbsBuilder::formatList(const QStringList &list) {
    bool first  = true;
    QString result;
    foreach(QString it, list) {
        result += QString("%2\n\t\t\t\"%1\"").arg(it).arg((!first) ? "," : "");
        first   = false;
    }
    return result;
}

void QbsBuilder::parseLogs(const QString &log) {
    QStringList list = log.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" error ")) {
            Log(Log::ERR) << qPrintable(it);
        } else if(it.contains(" warning ")) {
            Log(Log::WRN) << qPrintable(it);
        } else {
            Log(Log::INF) << qPrintable(it);
        }
    }
}

bool QbsBuilder::checkProfile(const QString &profile) {
    QStringList args;
    args << "config" << "--list" << m_Settings;
    QProcess qbs(this);
    qbs.setWorkingDirectory(m_Project);
    qbs.start(m_pMgr->qbsPath(), args);
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        return qbs.readAll().contains(qPrintable(profile));
    }
    return false;
}

void QbsBuilder::setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs) {
    m_IncludePath   = incp;
    m_LibPath       = libp;
    m_Libs          = libs;
}
