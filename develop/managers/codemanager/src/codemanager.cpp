#include "codemanager.h"

#include <QDirIterator>
#include <QProcess>
#include <QCoreApplication>
#include <QMetaProperty>

#include <log.h>

#include <projectmanager.h>

QRegExp gClass("A_REGISTER\\((\\w+),(|\\s+)(\\w+),(|\\s+)(\\w+)\\)");

const QString gRegisterComponents("${RegisterComponents}");
const QString gUnregisterComponents("${UnregisterComponents}");
const QString gComponentNames("${ComponentNames}");
const QString gIncludes("${Includes}");

#include "qbsbuilder.h"

CodeManager *CodeManager::m_pInstance   = nullptr;

CodeManager *CodeManager::instance() {
    if(!m_pInstance) {
        m_pInstance = new CodeManager;
    }
    return m_pInstance;
}

void CodeManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void CodeManager::init() {
    m_Suffixes << "cpp" << "h";

    m_Outdated  = false;

    QStringList includepath;
    QStringList libpath;
    QStringList libs;

    m_pBuilder  = new QbsBuilder();
    m_pBuilder->setEnvironment(includepath, libpath, libs);

    connect(m_pBuilder, SIGNAL(buildFinished(int)), this, SLOT(onBuildFinished(int)));

    m_pProject  = ProjectManager::instance();
}

void CodeManager::rebuildProject() {
    setOutdated();
    buildProject();
}

void CodeManager::buildProject() {
    if(m_Outdated) {
        QStringList code    = rescanSources(m_pProject->contentPath());

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

        StringMap values;

        values[gRegisterComponents]     = "";
        values[gUnregisterComponents]   = "";
        values[gComponentNames]         = "";

        const QMetaObject *meta = ProjectManager::instance()->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property  = meta->property(i);
            values[QString("${%1}").arg(property.name())]   = property.read(ProjectManager::instance()).toString();
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

        m_pBuilder->copyTemplate(m_pProject->templatePath() + "/plugin.cpp", m_pBuilder->project() + "plugin.cpp", values);
        m_pBuilder->copyTemplate(m_pProject->templatePath() + "/application.cpp", m_pBuilder->project() + "application.cpp", values);

        code.removeDuplicates();

        m_pBuilder->generateProject(code);

        m_pBuilder->buildProject();
    }
}

void CodeManager::onBuildFinished(int exitCode) {
    if(exitCode == 0) {
        emit buildSucess(m_pBuilder->artifact());
        m_Outdated  = false;
    } else {
        emit buildFailed();
    }
}

bool CodeManager::isOutdated() const {
    return m_Outdated;
}

void CodeManager::setOutdated() {
    m_Outdated  = true;
}

QString CodeManager::artifact() const {
    return m_pBuilder->artifact();
}

QStringList CodeManager::rescanSources(const QString &path) {
    QStringList result;
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(m_Suffixes.contains(info.completeSuffix(), Qt::CaseInsensitive)) {
            result.push_back(info.absoluteFilePath());
        }
    }
    return result;
}
