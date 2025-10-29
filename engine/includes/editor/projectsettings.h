#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <engine.h>

#include <resources/map.h>

#include <editor/assetconverter.h>

class CodeBuilder;

class ENGINE_EXPORT ProjectSettings : public Object {
    A_OBJECT(ProjectSettings, Object, Editor)

    A_PROPERTIES(
        A_PROPERTY(TString, projectName, ProjectSettings::projectName, ProjectSettings::setProjectName),
        A_PROPERTY(TString, companyName, ProjectSettings::projectCompany, ProjectSettings::setProjectCompany),
        A_PROPERTY(TString, projectVersion, ProjectSettings::projectVersion, ProjectSettings::setProjectVersion),
        A_PROPERTYEX(TString, projectId, ProjectSettings::projectId, ProjectSettings::setProjectId, "ReadOnly"),
        A_PROPERTYEX(TString, projectSdk, ProjectSettings::projectSdk, ProjectSettings::setProjectSdk, "ReadOnly"),
        A_PROPERTYEX(TString, firstMap, ProjectSettings::firstMap, ProjectSettings::setFirstMap, "editor=Asset,type=Map")
        //A_PROPERTY(TString[], modules, ProjectSettings::getModules, ProjectSettings::setModules),
        //A_PROPERTY(TString[], platforms, ProjectSettings::getPlatforms, ProjectSettings::setPlatforms)
    )

public:
    ProjectSettings();
    ~ProjectSettings() {}

    static ProjectSettings *instance();
    static void destroy();

    void init(const TString &project, const TString &target = TString());

    void loadPlatforms();

    TString projectName() const;
    void setProjectName(const TString &value);

    TString projectId() const;
    void setProjectId(const TString &value);

    TString projectCompany() const;
    void setProjectCompany(const TString &value);

    TString projectVersion() const;
    void setProjectVersion(const TString &value);

    TString firstMap() const;
    void setFirstMap(const TString &value);

    TString projectSdk() const;
    void setProjectSdk(const TString &sdk);

    TString projectPath() const;
    TString targetPath() const;
    TString contentPath() const;
    TString cachePath() const;
    TString importPath() const;
    TString iconPath() const;
    TString generatedPath() const;
    TString pluginsPath() const;

    TString manifestFile() const;

    TString sdkPath() const;
    TString resourcePath() const;
    TString templatePath() const;

    TString myProjectsPath() const;

    StringList modules() const;
    StringList autoModules() const;

    StringList platforms() const;

    std::map<TString, bool> &plugins();

    void setCurrentPlatform(const TString &platform = TString());
    TString currentPlatformName() const;
    CodeBuilder *currentBuilder(const TString &platform = TString()) const;

    void reportTypes(const std::set<TString> &types);

    StringList artifacts() const;
    void setArtifacts(const StringList &value);

    void loadSettings();
    void saveSettings();

private:
    VariantList getModules() const;
    void setModules(VariantList modules);

    VariantList getPlatforms() const;
    void setPlatforms(VariantList platforms);

    static ProjectSettings *m_pInstance;

private:
    StringList m_platforms;
    StringList m_artifacts;

    std::map<TString, bool> m_plugins;
    std::map<TString, CodeBuilder *> m_supportedPlatforms;

    std::set<TString> m_modules;
    std::set<TString> m_autoModules;

    TString m_projectId;
    TString m_projectName;
    TString m_companyName;
    TString m_projectVersion;
    TString m_projectSdk;

    TString m_currentPlatform;

    TString m_projectPath;
    TString m_targetPath;
    TString m_contentPath;
    TString m_cachePath;
    TString m_importPath;
    TString m_iconPath;
    TString m_generatedPath;
    TString m_pluginsPath;

    TString m_sdkPath;
    TString m_resourcePath;
    TString m_templatePath;

    TString m_myProjectsPath;

    TString m_manifestFile;

    TString m_firstMap;

};

#endif // PROJECTSETTINGS_H
