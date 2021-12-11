#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QFileInfo>
#include <QProcess>
#include <QSet>

#include <engine.h>
#include <resources/map.h>

#include "assetmanager.h"

class Platform;

class ProjectManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Project_Name READ projectName WRITE setProjectName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(QString Company_Name READ projectCompany WRITE setProjectCompany NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(QString Project_Version READ projectVersion WRITE setProjectVersion NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(Template First_Map READ firstMap WRITE setFirstMap NOTIFY updated DESIGNABLE true USER true)

public:
    static ProjectManager *instance();
    static void destroy();

    void init(const QString &project, const QString &target = QString());

    QString projectName() const { return m_projectName; }
    void setProjectName(const QString &value) { m_projectName = value; emit updated(); }

    QString projectId() const { return m_projectId; }

    QString projectCompany() const { return m_companyName; }
    void setProjectCompany(const QString &value) { m_companyName = value; emit updated(); }

    QString projectVersion() const { return m_projectVersion; }
    void setProjectVersion(const QString &value) { m_projectVersion = value; emit updated(); }

    Template firstMap() const { return Template(m_firstMap, MetaType::type<Map *>()); }
    void setFirstMap(const Template &value) { m_firstMap = value.path; emit updated(); }

    QString projectPath() const { return m_projectPath.absoluteFilePath(); }
    QString targetPath() const { return m_targetPath.filePath(); }
    QString contentPath() const { return m_contentPath.absoluteFilePath(); }
    QString cachePath() const { return m_cachePath.absoluteFilePath(); }
    QString importPath() const { return m_importPath.absoluteFilePath(); }
    QString iconPath() const { return m_iconPath.absoluteFilePath(); }
    QString generatedPath() const { return m_generatedPath.absoluteFilePath(); }
    QString pluginsPath() const { return m_pluginsPath.absoluteFilePath(); }

    QString manifestFile() const { return m_manifestFile.absoluteFilePath(); }

    QString sdkPath() const { return m_sdkPath.absoluteFilePath(); }
    QString resourcePath() const { return m_resourcePath.absoluteFilePath(); }
    QString templatePath() const { return m_templatePath.absoluteFilePath(); }

    QString myProjectsPath() const { return m_myProjectsPath.absoluteFilePath(); }

    QStringList modules() const;
    QStringList autoModules() const;

    QStringList platforms() const;
    Platform *supportedPlatform(const QString &platform);
    void setSupportedPlatform(Platform *platform);

    Platform *currentPlatform() const;
    void setCurrentPlatform(const QString &platform = QString());

    void reportModules(QSet<QString> &modules);

signals:
    void updated();

    void readBuildLogs(QString log);

public slots:
    void loadSettings();
    void saveSettings();

    void build(QString platform);

private slots:
    void onBuildFinished(int exitCode, QProcess::ExitStatus);

    void readOutput();
    void readError();

private:
    ProjectManager();
    ~ProjectManager() {}

    static ProjectManager *m_pInstance;

private:
    QString m_projectId;
    QString m_projectName;
    QString m_companyName;
    QString m_projectVersion;

    QString m_firstMap;

    QStringList m_platforms;
    Platform *m_currentPlatform;

    QMap<QString, Platform *> m_supportedPlatforms;

    QFileInfo m_projectPath;
    QFileInfo m_targetPath;
    QFileInfo m_contentPath;
    QFileInfo m_cachePath;
    QFileInfo m_importPath;
    QFileInfo m_iconPath;
    QFileInfo m_generatedPath;
    QFileInfo m_pluginsPath;

    QFileInfo m_sdkPath;
    QFileInfo m_resourcePath;
    QFileInfo m_templatePath;

    QFileInfo m_myProjectsPath;

    QFileInfo m_manifestFile;

    QSet<QString> m_modules;
    QSet<QString> m_autoModules;

    QProcess *m_builder;
};

#endif // PROJECTMANAGER_H
