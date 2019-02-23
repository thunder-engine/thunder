#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QFileInfo>

#include <engine.h>

#include <assetmanager.h>

class ProjectManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Project_Name READ projectName WRITE setProjectName DESIGNABLE true USER true)
    Q_PROPERTY(QString Company_Name READ projectCompany WRITE setProjectCompany DESIGNABLE true USER true)
    Q_PROPERTY(QString Project_Version READ projectVersion WRITE setProjectVersion DESIGNABLE true USER true)

    Q_PROPERTY(Template First_Map READ firstMap WRITE setFirstMap DESIGNABLE true USER true)

public:
    static ProjectManager      *instance                    ();

    static void                 destroy                     ();

    void                        init                        (const QString &project, const QString &target = QString());

    QString                     projectName                 () const { return m_ProjectName; }
    void                        setProjectName              (const QString &value) { m_ProjectName = value; }

    QString                     projectId                   () const { return m_ProjectId; }

    QString                     projectCompany              () const { return m_CompanyName; }
    void                        setProjectCompany           (const QString &value) { m_CompanyName = value; }

    QString                     projectVersion              () const { return m_ProjectVersion; }
    void                        setProjectVersion           (const QString &value) { m_ProjectVersion = value; }

    Template                    firstMap                    () const { return Template(m_FirstMap, IConverter::ContentMap); }
    void                        setFirstMap                 (const Template &value) { m_FirstMap = value.path; }

    QString                     projectPath                 () const { return m_ProjectPath.absoluteFilePath(); }
    QString                     targetPath                  () const { return m_TargetPath.filePath(); }
    QString                     contentPath                 () const { return m_ContentPath.absoluteFilePath(); }
    QString                     cachePath                   () const { return m_CachePath.absoluteFilePath(); }
    QString                     importPath                  () const { return m_ImportPath.absoluteFilePath(); }
    QString                     iconPath                    () const { return m_IconPath.absoluteFilePath(); }
    QString                     generatedPath               () const { return m_GeneratedPath.absoluteFilePath(); }
    QString                     pluginsPath                 () const { return m_PluginsPath.absoluteFilePath(); }

    QString                     sdkPath                     () const { return m_SDKPath.absoluteFilePath(); }
    QString                     resourcePath                () const { return m_ResourcePath.absoluteFilePath(); }
    QString                     templatePath                () const { return m_TemplatePath.absoluteFilePath(); }

    QString                     myProjectsPath              () const { return m_MyProjectsPath.absoluteFilePath(); }

public slots:
    void                        loadSettings                ();
    void                        saveSettings                ();

private:
    ProjectManager              ();
    ~ProjectManager             () {}

    static ProjectManager      *m_pInstance;

private:
    QString                     m_ProjectId;
    QString                     m_ProjectName;
    QString                     m_CompanyName;
    QString                     m_ProjectVersion;

    QString                     m_FirstMap;

    QFileInfo                   m_ProjectPath;
    QFileInfo                   m_TargetPath;
    QFileInfo                   m_ContentPath;
    QFileInfo                   m_CachePath;
    QFileInfo                   m_ImportPath;
    QFileInfo                   m_IconPath;
    QFileInfo                   m_GeneratedPath;
    QFileInfo                   m_PluginsPath;

    QFileInfo                   m_SDKPath;
    QFileInfo                   m_ResourcePath;
    QFileInfo                   m_TemplatePath;

    QFileInfo                   m_MyProjectsPath;
};

#endif // PROJECTMANAGER_H
