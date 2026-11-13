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
#ifndef ASSETEDITOR_H
#define ASSETEDITOR_H

#include <widget.h>

#include <editor/assetconverter.h>
#include <editor/undostack.h>

class EDITOR_EXPORT AssetEditor : public Widget {
    A_OBJECT(AssetEditor, Widget, Editor)

    A_METHODS(
        A_SIGNAL(AssetEditor::updated),
        A_SIGNAL(AssetEditor::objectsHierarchyChanged),
        A_SIGNAL(AssetEditor::selectionChanged),
        A_SIGNAL(AssetEditor::objectsChanged),
        A_SIGNAL(AssetEditor::copyPasteChanged),
        A_SIGNAL(AssetEditor::dropAsset),
        A_SIGNAL(AssetEditor::editorClosing),
        A_SLOT(AssetEditor::onActivated),
        A_SLOT(AssetEditor::onNewAsset),
        A_SLOT(AssetEditor::onOpenAsset),
        A_SLOT(AssetEditor::onSave),
        A_SLOT(AssetEditor::onSaveAs),
        A_SLOT(AssetEditor::onObjectCreate),
        A_SLOT(AssetEditor::onObjectsSelected),
        A_SLOT(AssetEditor::onDropAsset),
        A_SLOT(AssetEditor::onSelectionDeleted),
        A_SLOT(AssetEditor::onUpdated),
        A_SLOT(AssetEditor::onObjectsChanged)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    AssetEditor();
    ~AssetEditor() override;

    virtual void loadAsset(AssetConverterSettings *settings) {}
    virtual StringList suffixes() const { return StringList(); }
    virtual TString assetType() const { return TString(); }
    virtual bool isSingleInstance() const { return true; }
    virtual AssetEditor *createInstance() { return nullptr; }
    virtual bool checkSave();
    virtual StringList componentGroups() const;
    virtual Object::ObjectList selected() const;
    virtual bool maskPropertyType() const;
    virtual void changeParent(const Object::ObjectList &objects, Object *parent, int position = -1);

    virtual VariantMap saveState();
    virtual void restoreState(const VariantMap &data);

    virtual void backup();
    virtual void restore();

public: // slots
    virtual void onActivated();
    virtual void onNewAsset();
    virtual void onOpenAsset();
    virtual void onSave();
    virtual void onSaveAs();
    virtual void onObjectCreate(const TString &type);
    virtual void onObjectsSelected(bool force);
    virtual void onDropAsset(const TString &path, bool additive);
    virtual void onSelectionDeleted();
    virtual void onUpdated();
    virtual void onObjectsChanged(const TString &property, const Variant &value);

    std::list<AssetConverterSettings *> &openedDocuments();
    UndoStack *undoRedo() const;

public: // signals
    void updated();
    void objectsHierarchyChanged(Object *root);
    void selectionChanged();
    void objectsChanged(const TString &property, const Variant &value);
    void copyPasteChanged();
    void dropAsset(const TString &path);
    void editorClosing();

protected:
    virtual bool isModified() const;
    virtual void cleanModified() const;
    virtual void saveAsset(const TString &path = TString());

    std::list<AssetConverterSettings *> m_settings;
    UndoStack *m_undoRedo;
};

#endif // ASSETEDITOR_H
