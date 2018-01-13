#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QFileInfo>
#include <patterns/asingleton.h>

#include <engine.h>

#include <assetmanager.h>

class ProjectManager : public QObject, public ASingleton<ProjectManager> {
    Q_OBJECT

    Q_PROPERTY(QString Project_Name READ projectName WRITE setProjectName DESIGNABLE true USER true)
    Q_PROPERTY(QString Company_Name READ projectCompany WRITE setProjectCompany DESIGNABLE true USER true)
    Q_PROPERTY(QString Project_Version READ projectVersion WRITE setProjectVersion DESIGNABLE true USER true)

    Q_PROPERTY(Template First_Map READ firstMap WRITE setFirstMap DESIGNABLE true USER true)

public:

    void                        init                        (const QString &project, const QString &target = QString());

    QString                     projectName                 () const { return m_ProjectName; }
    void                        setProjectName              (const QString &value) { m_ProjectName = value; }

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

public slots:
    void                        loadSettings                ();
    void                        saveSettings                ();

protected:
    friend class ASingleton<ProjectManager>;

    ProjectManager              ();

private:
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

};

#endif // PROJECTMANAGER_H
