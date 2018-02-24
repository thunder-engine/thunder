#include "projectmanager.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include <QMetaProperty>
#include <QCoreApplication>

#include "common.h"

const QString gCompany("Company");
const QString gQBS("QBS");

ProjectManager::ProjectManager() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#if __APPLE__
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    m_SDKPath       = QFileInfo(dir.absolutePath());
    m_ResourcePath  = QFileInfo(sdkPath() + "/resources");
    m_QBSPath       = QFileInfo("qbs");
    m_QBSDefault    = m_QBSPath;

    m_MyProjectsPath    = QFileInfo(dir.absolutePath());
}

void ProjectManager::init(const QString &project, const QString &target) {
    m_ProjectPath   = QFileInfo(project);

    if(!target.isEmpty()) {
        QDir dir;
        dir.mkpath(target);
    }
    m_TargetPath    = QFileInfo(target);

    loadSettings();

    m_ProjectName   = m_ProjectPath.completeBaseName();

    m_ContentPath   = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gContent);
    m_CachePath     = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gCache);
    m_PluginsPath   = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gPlugins);

    m_ImportPath    = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gImport);
    m_IconPath      = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gIcons);
    m_GeneratedPath = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gGenerated);

    QDir dir;
    dir.mkpath(m_ContentPath.absoluteFilePath());
    dir.mkpath(m_ImportPath.absoluteFilePath());
    dir.mkpath(m_IconPath.absoluteFilePath());
    dir.mkpath(m_GeneratedPath.absoluteFilePath());
    dir.mkpath(m_PluginsPath.absoluteFilePath());
}

void ProjectManager::setQbsPath(const QString &path) {
    m_QBSPath   = path;
}

void ProjectManager::loadSettings() {
    QFile file(m_ProjectPath.absoluteFilePath());
    if(file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if(!doc.isNull()) {
            const QMetaObject *meta = metaObject();
            QJsonObject object      = doc.object();
            foreach(const QString &it, doc.object().keys()) {
                int index   = meta->indexOfProperty(qPrintable(it));
                if(index > -1) {
                    QMetaProperty property  = meta->property(index);
                    QVariant value  = object.value(it).toVariant();

                    if(property.userType() == qMetaTypeId<Template>()) {
                        value   = QVariant::fromValue<Template>(Template(value.toString(), IConverter::ContentMap));
                    }
                    property.write(this, value);
                }
            }

            QJsonValue value    = object.value(gQBS);
            if(!value.isUndefined()) {
                setQbsPath(value.toString());
            }
        }
    }
}

void ProjectManager::saveSettings() {
    QJsonDocument doc;

    const QMetaObject *meta = metaObject();

    QJsonObject object;
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        if(property.isUser(this)) {
            const char *name    = property.name();
            QVariant value      = property.read(this);
            if(value.canConvert<Template>()) {
                object[name] = QJsonValue(value.value<Template>().path);
            } else {
                object[name] = QJsonValue(value.toString());
            }

        }
    }

    QString qbs = qbsPath();
    if(m_QBSDefault != qbs) {
        object[gQBS]    = qbs;
    }
    doc.setObject(object);

    QFile file(m_ProjectPath.absoluteFilePath());
    if(file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

