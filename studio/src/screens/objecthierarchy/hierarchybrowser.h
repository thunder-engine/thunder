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
#ifndef HIERARCHYBROWSER_H
#define HIERARCHYBROWSER_H

#include <components/widget.h>
#include <object.h>
#include <editor/asseteditor.h>

class LineEdit;
class Button;
class Menu;
class TreeView;
class ObjectHierarchyModel;

class HierarchyBrowser : public Widget {
    A_OBJECT(HierarchyBrowser, Widget, Editor)

    A_METHODS(
        A_SLOT(HierarchyBrowser::onObjectsHierarchyChanged),
        A_SLOT(HierarchyBrowser::onEditorSelectionChanged),
        A_SLOT(HierarchyBrowser::onTreeSelectionChanged),
        A_SLOT(HierarchyBrowser::onAddButtonClicked),
        A_SLOT(HierarchyBrowser::onAddMenuTriggered),
        A_SLOT(HierarchyBrowser::onTreeDropped)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    HierarchyBrowser();
    ~HierarchyBrowser() override;

    void setEditor(AssetEditor *editor);

protected:
    void composeComponent() override;
    void update(const Vector2 &pos) override;

private:
    void onObjectsHierarchyChanged(Object *root);
    void onEditorSelectionChanged();
    void onTreeSelectionChanged();
    void onAddButtonClicked();
    void onAddMenuTriggered(int index);
    void onTreeDropped(const TString &mimeType, const TString &data, const ModelIndex &index);

private:
    AssetEditor *m_editor;
    ObjectHierarchyModel *m_model;
    Button *m_addButton;
    Menu *m_addMenu;
    LineEdit *m_search;
    TreeView *m_tree;
    TString m_appliedFilter;
};

#endif // HIERARCHYBROWSER_H
