#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <QObject>
#include <QFileInfo>
#include <QProcess>
#include <QSet>
#include <QVariant>

#include <engine.h>
#include <editor/assetconverter.h>

class CodeBuilder;

class ENGINE_EXPORT ProjectSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Project_Name READ projectName WRITE setProjectName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(QString Company_Name READ projectCompany WRITE setProjectCompany NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(QString Project_Version READ projectVersion WRITE setProjectVersion NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(Template First_Map READ firstMap WRITE setFirstMap NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(QVariantList modules READ getModules WRITE setModules NOTIFY updated RESET resetModules DESIGNABLE true USER true)
    Q_PROPERTY(QVariantList platforms READ getPlatforms WRITE setPlatforms NOTIFY updated RESET resetPlatforms DESIGNABLE true USER true)

public:
    static ProjectSettings *instance();
    static void destroy();

    void init(const QString &project, const QString &target = QString());

    void loadPlatforms();

    QString projectName() const;
    void setProjectName(const QString &value);

    QString projectId() const;

    QString projectCompany() const;
    void setProjectCompany(const QString &value);

    QString projectVersion() const;
    void setProjectVersion(const QString &value);

    Template firstMap() const;
    void setFirstMap(const Template &value);

    QString projectSdk() const;
    void setProjectSdk(const QString &sdk);

    QString projectPath() const;
    QString targetPath() const;
    QString contentPath() const;
    QString cachePath() const;
    QString importPath() const;
    QString iconPath() const;
    QString generatedPath() const;
    QString pluginsPath() const;

    QString manifestFile() const;

    QString sdkPath() const;
    QString resourcePath() const;
    QString templatePath() const;

    QString myProjectsPath() const;

    QStringList modules() const;
    QStringList autoModules() const;

    QStringList platforms() const;

    QVariantMap &plugins();

    void setCurrentPlatform(const QString &platform = QString());
    QString currentPlatformName() const;
    CodeBuilder *currentBuilder(const QString &platform = QString()) const;

    void reportModules(QSet<QString> &modules);

    QString artifact() const;
    void setArtifact(const QString &value);

signals:
    void updated();

public slots:
    void loadSettings();
    void saveSettings();

private:
    ProjectSettings();
    ~ProjectSettings() {}

    QVariantList getModules();
    void setModules(QVariantList modules);
    void resetModules();

    QVariantList getPlatforms();
    void setPlatforms(QVariantList platforms);
    void resetPlatforms();

    static ProjectSettings *m_pInstance;

private:
    QStringList m_platforms;
    QVariantMap m_plugins;

    QString m_projectId;
    QString m_projectName;
    QString m_companyName;
    QString m_projectVersion;
    QString m_projectSdk;

    QString m_firstMap;
    QString m_currentPlatform;

    QString m_artifact;

    std::map<TString, CodeBuilder *> m_supportedPlatforms;

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

};

#endif // PROJECTSETTINGS_H
