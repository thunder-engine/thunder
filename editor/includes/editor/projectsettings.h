/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <set>

#include <editor.h>

class CodeBuilder;
class NativeCodeBuilder;

class EDITOR_EXPORT ProjectSettings : public Object {
    A_OBJECT(ProjectSettings, Object, Editor)

    A_PROPERTIES(
        A_PROPERTY(TString, projectName, ProjectSettings::projectName, ProjectSettings::setProjectName),
        A_PROPERTY(TString, companyName, ProjectSettings::projectCompany, ProjectSettings::setProjectCompany),
        A_PROPERTY(TString, projectVersion, ProjectSettings::projectVersion, ProjectSettings::setProjectVersion),
        A_PROPERTYEX(TString, projectId, ProjectSettings::projectId, ProjectSettings::setProjectId, "ReadOnly"),
        A_PROPERTYEX(TString, projectSdk, ProjectSettings::projectSdk, ProjectSettings::setProjectSdk, "ReadOnly"),
        A_PROPERTYEX(TString, firstMap, ProjectSettings::firstMap, ProjectSettings::setFirstMap, "editor=Asset,type=Map")
    )

public:
    ProjectSettings();
    ~ProjectSettings() {}

    void init(const TString &project, const TString &target = TString());

    void loadPlatforms();

    TString projectName() const;
    void setProjectName(const TString &name);

    TString projectId() const;
    void setProjectId(const TString &project);

    TString projectCompany() const;
    void setProjectCompany(const TString &name);

    TString projectVersion() const;
    void setProjectVersion(const TString &version);

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

    TString platformsPath() const;

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
    NativeCodeBuilder *currentBuilder(const TString &platform = TString()) const;

    void reportTypes(const std::set<TString> &types);

    StringList artifacts() const;
    void setArtifacts(const StringList &artifacts);

    void loadSettings();
    void saveSettings();

    Variant property(const char *name) const override;
    void setProperty(const char *name, const Variant &value) override;

private:
    VariantList getModules() const;
    void setModules(const VariantList &modules);

    VariantList getPlatforms() const;
    void setPlatforms(const VariantList &platforms);

private:
    StringList m_platforms;
    StringList m_artifacts;

    StringList m_modules;
    StringList m_autoModules;

    std::map<TString, bool> m_plugins;
    std::map<TString, CodeBuilder *> m_supportedPlatforms;

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
    TString m_platformsPath;

    TString m_sdkPath;
    TString m_resourcePath;
    TString m_templatePath;

    TString m_myProjectsPath;

    TString m_firstMap;

};

#endif // PROJECTSETTINGS_H
