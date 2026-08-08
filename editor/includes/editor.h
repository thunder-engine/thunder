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

class AssetEditor;
class EditorGadget;

class DocumentModel;

class EDITOR_EXPORT Editor : public Object {
    A_OBJECT(Editor, Object, General)

public:
    Editor();
    ~Editor();

    static void init();

    static void backup();
    static void restore();

    static void addEditor(AssetEditor *editor);
    static void closeEditor(AssetEditor *editor);

    static AssetEditor *openFile(const TString &path);

    static AssetEditor *currentEditor();
    static void setCurrentEditor(AssetEditor *editor);

    static std::list<AssetEditor *> documents();

    static void addGadget(EditorGadget *gadget);
    static std::list<EditorGadget *> gadgets();

    static EditorSettings *settings();
    static ProjectSettings *project();
    static AssetManager *assets();

protected:
    static std::list<EditorGadget *> s_gadgets;

    static AssetEditor *s_currentEditor;

    static EditorSettings *s_editorSettings;
    static ProjectSettings *s_projectSettings;
    static AssetManager *s_assetManager;

    static DocumentModel *s_documentModel;

};

#endif // EDITOR_H
