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
#ifndef ANGELBUILDER_H
#define ANGELBUILDER_H

#include <editor/assetconverter.h>
#include <abstractitemmodel.h>

#include <set>

class asSMessageInfo;
class asIScriptEngine;
class asITypeInfo;
class asIScriptFunction;

class AngelSystem;

class AngelScriptImportSettings : public AssetConverterSettings {
public:
    bool isCode() const override { return true; }
    StringList typeNames() const override;
};

class AngelBuilder : public AssetConverter {
public:
    AngelBuilder(AngelSystem *system);
    ~AngelBuilder() override;

    StringList suffixes() const override { return {"as"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;
    AssetConverterSettings *createSettings() override;
    void finalizeBatch() override;
    void onFileRemoved(const TString &source) override;

protected:
    void init() override;

    TString templatePath() const override { return ":/templates/AngelBehaviour.as"; }

    static void messageCallback(const asSMessageInfo *msg, void *param);

    AngelSystem *m_system;

    asIScriptEngine *m_scriptEngine;
    bool m_rebuildPending;
    std::set<AssetConverterSettings *> m_batchSettings;

};

#endif // ANGELBUILDER_H
