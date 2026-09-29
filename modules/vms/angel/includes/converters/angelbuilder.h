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

#include <editor/codebuilder.h>
#include <abstractitemmodel.h>

class asSMessageInfo;
class asIScriptEngine;
class asITypeInfo;
class asIScriptFunction;

class AngelSystem;

class AngelScriptImportSettings : public BuilderSettings {
public:
    explicit AngelScriptImportSettings(CodeBuilder *builder);

    StringList typeNames() const override;

};

class AngelBuilder : public CodeBuilder {
public:
    AngelBuilder(AngelSystem *system);
    ~AngelBuilder() override;

protected:
    void init() override;

    bool buildProject() override;

    TString persistentName() const override;
    TString persistentAsset() const override;

    StringList suffixes() const override { return {"as"}; }

    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/templates/AngelBehaviour.as"; }

    static void messageCallback(const asSMessageInfo *msg, void *param);

    AngelSystem *m_system;

    asIScriptEngine *m_scriptEngine;

};

#endif // ANGELBUILDER_H
