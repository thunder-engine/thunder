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
#ifndef CODEBUILDER_H
#define CODEBUILDER_H

#include <set>

#include "assetconverter.h"

class QAbstractItemModel;

class EDITOR_EXPORT CodeBuilder : public AssetConverter {
public:
    CodeBuilder();

    virtual bool buildProject() = 0;

    virtual StringList platforms() const;

    TString project() const;

    StringList sources() const;

    virtual void rescanSources(const TString &path);
    virtual bool isEmpty() const;

    void makeOutdated();
    bool isOutdated() const;

    virtual QAbstractItemModel *classMap() const;

    ReturnCode convertFile(AssetConverterSettings *) override;

    void buildSuccessful(bool flag);

    virtual TString persistentName() const;
    virtual TString persistentAsset() const;

private:
    AssetConverterSettings *createSettings() override;

    void renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName) override;

protected:
    void init() override;

    void updateTemplate(const TString &src, const TString &dst, bool fromSource = false);

    void copyTempalte(const TString &src, const TString &dst);

protected:
    std::map<TString, TString> m_values;

    std::set<TString> m_sources;

    TString m_project;

    bool m_outdated;

};

class EDITOR_EXPORT BuilderSettings : public AssetConverterSettings {
public:
    explicit BuilderSettings(CodeBuilder *builder);

    CodeBuilder *builder() const;

private:
    StringList typeNames() const override;

    bool isCode() const override;

private:
    CodeBuilder *m_builder;

};

#endif // CODEBUILDER_H
