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
#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <components/widget.h>
#include <object.h>
#include <abstractitemmodel.h>

#include <set>
#include <vector>

class AssetConverterSettings;
class Button;
class ContentTree;
class LineEdit;
class ListView;
class Menu;
class TreeView;

class ContentBrowser : public Widget {
    A_OBJECT(ContentBrowser, Widget, Editor)

    A_METHODS(
        A_SIGNAL(ContentBrowser::assetsSelected),
        A_SIGNAL(ContentBrowser::openEditor),
        A_SLOT(ContentBrowser::onDirectoryChanged),
        A_SLOT(ContentBrowser::onNewButtonClicked),
        A_SLOT(ContentBrowser::onImportButtonClicked),
        A_SLOT(ContentBrowser::onActionsButtonClicked),
        A_SLOT(ContentBrowser::onFilterButtonClicked),
        A_SLOT(ContentBrowser::onUpButtonClicked),
        A_SLOT(ContentBrowser::onNewMenuTriggered),
        A_SLOT(ContentBrowser::onActionMenuTriggered),
        A_SLOT(ContentBrowser::onFilterMenuTriggered),
        A_SLOT(ContentBrowser::onTreeClicked),
        A_SLOT(ContentBrowser::onTreeActivated),
        A_SLOT(ContentBrowser::onListClicked),
        A_SLOT(ContentBrowser::onListActivated),
        A_SLOT(ContentBrowser::onTreeDropped),
        A_SLOT(ContentBrowser::onListDropped)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    ContentBrowser();
    ~ContentBrowser() override;

    void assetsSelected(Object *settings);
    void openEditor(const TString &path);
    void importDroppedFiles(const StringList &files);

protected:
    void composeComponent() override;
    void update(const Vector2 &pos) override;

private:
    void onDirectoryChanged(const TString &path);
    void onNewButtonClicked();
    void onImportButtonClicked();
    void onActionsButtonClicked();
    void onFilterButtonClicked();
    void onUpButtonClicked();
    void onNewMenuTriggered(int index);
    void onActionMenuTriggered(int index);
    void onFilterMenuTriggered(int index);
    void onTreeClicked(const ModelIndex &index);
    void onTreeActivated(const ModelIndex &index);
    void onListClicked(const ModelIndex &index);
    void onListActivated(const ModelIndex &index);
    void onTreeDropped(const TString &mimeType, const TString &data, const ModelIndex &index);
    void onListDropped(const TString &mimeType, const TString &data, const ModelIndex &index);

    void setDirectory(const TString &path);
    void handleDrop(const TString &mimeType, const TString &data, const TString &targetPath);
    void refresh();
    ModelIndex currentIndex() const;
    TString currentPath() const;
    void showInExplorer();
    void importFiles();
    void createAsset(const TString &templatePath);
    void renameCurrentAsset();
    void deleteCurrentAsset();

private:
    ContentTree *m_model;
    ContentTree *m_treeModel;
    AssetConverterSettings *m_settings;
    Button *m_newButton;
    Button *m_importButton;
    Button *m_actionsButton;
    Button *m_filterButton;
    Button *m_upButton;
    LineEdit *m_search;
    TreeView *m_tree;
    ListView *m_list;
    Menu *m_newMenu;
    Menu *m_actionsMenu;
    Menu *m_filterMenu;
    std::vector<TString> m_templatePaths;
    std::vector<TString> m_filterLabels;
    std::set<TString> m_contentTypes;
    TString m_currentDirectory;
    TString m_appliedFilter;
    bool m_refreshPending;
};

#endif // CONTENTBROWSER_H
