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
#ifndef EDITOR_H
#define EDITOR_H

#include <engine.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef EDITOR_LIBRARY
        #define EDITOR_EXPORT __declspec(dllexport)
    #else
        #define EDITOR_EXPORT __declspec(dllimport)
    #endif
#else
        #define EDITOR_EXPORT
#endif

class ProjectSettings;
class EditorSettings;
class AssetManager;
class PluginManager;

class LegacyAssetEditor;
class LegacyEditorGadget;

class LegacyDocumentModel;

class EDITOR_EXPORT Editor : public Object {
public:
    Editor(int argc, char *argv[]);
    ~Editor();

    static void init();

    static void backup();
    static void restore();

    static void addEditor(LegacyAssetEditor *editor);
    static void closeEditor(LegacyAssetEditor *editor);

    static LegacyAssetEditor *openFile(const TString &path);

    static LegacyAssetEditor *currentEditor();
    static void setCurrentEditor(LegacyAssetEditor *editor);

    static std::list<LegacyAssetEditor *> documents();

    static void addGadget(LegacyEditorGadget *gadget);
    static std::list<LegacyEditorGadget *> gadgets();

    static EditorSettings *settings();
    static ProjectSettings *project();
    static AssetManager *assets();
    static PluginManager *plugins();

    static TString applicationDirPath();

protected:
    static TString m_binPath;

    static std::list<LegacyEditorGadget *> s_gadgets;

    static LegacyAssetEditor *s_currentEditor;

    static EditorSettings *s_editorSettings;
    static ProjectSettings *s_projectSettings;

    static AssetManager *s_assetManager;
    static PluginManager *s_pluginManager;

    static LegacyDocumentModel *s_documentModel;

};

#endif // EDITOR_H
