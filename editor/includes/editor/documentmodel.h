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
#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <editor.h>
#include <object.h>

class AssetEditor;

class EDITOR_EXPORT DocumentModel : public Object {
    A_OBJECT(DocumentModel, Object, Editor)
    A_METHODS(
        A_SIGNAL(DocumentModel::updated),
        A_SIGNAL(DocumentModel::selectionChanged),
        A_SLOT(DocumentModel::onEditorUpdated),
        A_SLOT(DocumentModel::onEditorSelectionChanged),
        A_SLOT(DocumentModel::onEditorDestroyed),
        A_SLOT(DocumentModel::onLoadAsset)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    DocumentModel();
    ~DocumentModel() override;

    void addEditor(AssetEditor *editor);
    AssetEditor *openFile(const TString &path);
    bool closeFile(AssetEditor *editor);

    std::list<AssetEditor *> documents() const;

    AssetEditor *currentEditor() const;
    void setCurrentEditor(AssetEditor *editor);

    void updated();
    void selectionChanged();

public:
    void onEditorUpdated();
    void onEditorSelectionChanged();
    void onEditorDestroyed();
    void onLoadAsset(const TString &path);

private:
    void connectEditor(AssetEditor *editor);

    std::map<TString, AssetEditor *> m_editors;
    std::list<AssetEditor *> m_documents;
    std::list<AssetEditor *> m_connectedEditors;
    AssetEditor *m_currentEditor;
};

#endif // DOCUMENTMODEL_H
