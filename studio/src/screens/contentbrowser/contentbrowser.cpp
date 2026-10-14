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
#include "contentbrowser.h"
#include "contenttree.h"

#include <button.h>
#include <frame.h>
#include <layout.h>
#include <lineedit.h>
#include <listview.h>
#include <menu.h>
#include <splitter.h>
#include <treeview.h>

#include <editor/assetmanager.h>
#include <editor/assetconverter.h>
#include <editor/codebuilder.h>
#include <editor.h>
#include <editor/projectsettings.h>

#include <engine.h>
#include <file.h>
#include <filedialog.h>
#include <message.h>
#include <standardpaths.h>
#include <url.h>
#include <aprocess.h>
#include <log.h>

namespace {
    const TString kContentMimeType("text/content");
    const TString kObjectMimeType("text/object");
}

ContentBrowser::ContentBrowser() :
        m_model(new ContentTree),
        m_treeModel(new ContentTree),
        m_settings(nullptr),
        m_newButton(nullptr),
        m_importButton(nullptr),
        m_actionsButton(nullptr),
        m_filterButton(nullptr),
        m_upButton(nullptr),
        m_search(nullptr),
        m_tree(nullptr),
        m_list(nullptr),
        m_newMenu(nullptr),
        m_actionsMenu(nullptr),
        m_filterMenu(nullptr),
        m_refreshPending(false) {
    m_treeModel->setDirectoriesOnly(true);
    m_currentDirectory = m_model->contentPath();
}

ContentBrowser::~ContentBrowser() {
    if(Editor::assets()) {
        disconnect(Editor::assets(), _SIGNAL(directoryChanged(TString)),
                   this, _SLOT(onDirectoryChanged(TString)));
    }
    delete m_model;
    delete m_treeModel;
}

void ContentBrowser::assetsSelected(Object *settings) {
    emitSignal(_SIGNAL(assetsSelected(Object*)), settings);
}

void ContentBrowser::openEditor(const TString &path) {
    emitSignal(_SIGNAL(openEditor(TString)), path);
}

void ContentBrowser::composeComponent() {
    Widget::composeComponent();

    Widget *root = this;
    if(!root) {
        aError() << "Unable to create the ContentBrowser layout.";
        return;
    }

    Layout *layout = new Layout;
    layout->setOrientation(Widget::Vertical);
    layout->setSpacing(2);
    root->setLayout(layout);

    Widget *toolbarActor = Widget::createWidget<Frame>("ContentToolbar", this);
    Frame *toolbar = toolbarActor ? dynamic_cast<Frame *>(toolbarActor) : nullptr;
    if(!toolbar) {
        aError() << "Unable to create the ContentBrowser toolbar.";
        return;
    }
    toolbar->setSize(Vector2(0.0f, 26.0f));
    toolbar->setHorizontalPolicy(Widget::Expanding);
    toolbar->setVerticalPolicy(Widget::Fixed);
    Layout *toolbarLayout = new Layout;
    toolbarLayout->setOrientation(Widget::Horizontal);
    toolbarLayout->setSpacing(2);
    toolbar->setLayout(toolbarLayout);
    layout->addWidget(toolbar);

    auto createButton = [toolbarActor, toolbarLayout](const TString &name, const TString &text, Button **result) {
        Widget *buttonActor = Widget::createWidget<Button>(name, toolbarActor);
        *result = buttonActor ? dynamic_cast<Button *>(buttonActor) : nullptr;
        if(*result) {
            (*result)->setText(text);
            (*result)->setSize(Vector2(72.0f, 24.0f));
            (*result)->setHorizontalPolicy(Widget::Fixed);
            (*result)->setVerticalPolicy(Widget::Fixed);
            toolbarLayout->addWidget((*result));
        }
    };
    createButton("ContentNewButton", "New", &m_newButton);
    createButton("ContentImportButton", "Import", &m_importButton);
    createButton("ContentActionsButton", "Actions", &m_actionsButton);
    createButton("ContentFilterButton", "Filter", &m_filterButton);
    createButton("ContentUpButton", "Up", &m_upButton);
    if(!m_newButton || !m_importButton || !m_actionsButton || !m_filterButton || !m_upButton) {
        aError() << "Unable to create the ContentBrowser toolbar buttons.";
        return;
    }

    connect(m_newButton, _SIGNAL(clicked()), this, _SLOT(onNewButtonClicked()));
    connect(m_importButton, _SIGNAL(clicked()), this, _SLOT(onImportButtonClicked()));
    connect(m_actionsButton, _SIGNAL(clicked()), this, _SLOT(onActionsButtonClicked()));
    connect(m_filterButton, _SIGNAL(clicked()), this, _SLOT(onFilterButtonClicked()));
    connect(m_upButton, _SIGNAL(clicked()), this, _SLOT(onUpButtonClicked()));

    Widget *newMenuActor = Widget::createWidget<Menu>("ContentNewMenu", m_newButton);
    m_newMenu = newMenuActor ? dynamic_cast<Menu *>(newMenuActor) : nullptr;
    Widget *actionsMenuActor = Widget::createWidget<Menu>("ContentActionsMenu", m_actionsButton);
    m_actionsMenu = actionsMenuActor ? dynamic_cast<Menu *>(actionsMenuActor) : nullptr;
    Widget *filterMenuActor = Widget::createWidget<Menu>("ContentFilterMenu", m_filterButton);
    m_filterMenu = filterMenuActor ? dynamic_cast<Menu *>(filterMenuActor) : nullptr;
    if(!m_newMenu || !m_actionsMenu || !m_filterMenu) {
        aError() << "Unable to create the ContentBrowser menus.";
        return;
    }
    m_newButton->setSubWidget(m_newMenu);
    m_actionsButton->setSubWidget(m_actionsMenu);
    m_filterButton->setSubWidget(m_filterMenu);

    m_newMenu->addAction("New Folder");
    m_newMenu->addSeparator();
    std::set<TString> templates;
    if(Editor::assets()) {
        for(AssetConverter *converter : Editor::assets()->converters()) {
            TString path = converter->templatePath();
            if(!path.isEmpty()) {
                templates.insert(path);
            }
        }
        for(CodeBuilder *builder : Editor::assets()->builders()) {
            TString path = builder->templatePath();
            if(!path.isEmpty()) {
                templates.insert(path);
            }
        }
    }
    for(const TString &path : templates) {
        Url url(path);
        TString label = url.baseName();
        label.replace('_', ' ');
        m_templatePaths.push_back(path);
        m_newMenu->addAction(label);
    }
    connect(m_newMenu, _SIGNAL(triggered(int)), this, _SLOT(onNewMenuTriggered(int)));

    m_actionsMenu->addAction("Open");
    m_actionsMenu->addAction("Show in Explorer");
    m_actionsMenu->addSeparator();
    m_actionsMenu->addAction("Duplicate");
    m_actionsMenu->addAction("Rename");
    m_actionsMenu->addAction("Delete");
    m_actionsMenu->addAction("Reimport");
    m_actionsMenu->addSeparator();
    m_actionsMenu->addAction("Commit Import Settings");
    m_actionsMenu->addAction("Revert Import Settings");
    connect(m_actionsMenu, _SIGNAL(triggered(int)), this, _SLOT(onActionMenuTriggered(int)));

    if(Editor::assets()) {
        for(const TString &type : Editor::assets()->labels()) {
            if(!type.isEmpty()) {
                m_filterLabels.push_back(type);
                m_filterMenu->addAction(type);
                m_filterMenu->setItemState(static_cast<int>(m_filterLabels.size()) - 1, true, true, false);
            }
        }
    }
    connect(m_filterMenu, _SIGNAL(triggered(int)), this, _SLOT(onFilterMenuTriggered(int)));

    Widget *searchActor = Widget::createWidget<LineEdit>("ContentSearch", toolbarActor);
    m_search = searchActor ? dynamic_cast<LineEdit *>(searchActor) : nullptr;
    if(!m_search) {
        aError() << "Unable to create the ContentBrowser search field.";
        return;
    }
    m_search->setSize(Vector2(120.0f, 24.0f));
    m_search->setHorizontalPolicy(Widget::Expanding);
    m_search->setVerticalPolicy(Widget::Fixed);
    toolbarLayout->addWidget(m_search);

    Widget *splitterActor = Widget::createWidget<Splitter>("ContentSplitter", this);
    Splitter *splitter = splitterActor ? dynamic_cast<Splitter *>(splitterActor) : nullptr;
    if(!splitter) {
        aError() << "Unable to create the ContentBrowser splitter.";
        return;
    }
    splitter->setOrientation(Widget::Horizontal);
    splitter->setHorizontalPolicy(Widget::Expanding);
    splitter->setVerticalPolicy(Widget::Expanding);
    layout->addWidget(splitter);

    Widget *treeActor = Widget::createWidget<TreeView>("ContentTree", splitterActor);
    m_tree = treeActor ? dynamic_cast<TreeView *>(treeActor) : nullptr;
    Widget *listActor = Widget::createWidget<ListView>("ContentList", splitterActor);
    m_list = listActor ? dynamic_cast<ListView *>(listActor) : nullptr;
    if(!m_tree || !m_list) {
        aError() << "Unable to create the ContentBrowser views.";
        return;
    }
    m_tree->setSize(Vector2(200.0f, 0.0f));
    m_tree->setHorizontalPolicy(Widget::Preferred);
    m_tree->setVerticalPolicy(Widget::Expanding);
    m_tree->setRowHeight(20);
    m_tree->setIndentation(16);
    m_tree->setDragEnabled(true);
    m_tree->setDragMimeType(kContentMimeType);
    m_tree->setDragDataRole(AbstractItemModel::UserRole);
    m_tree->setAcceptDrops(true);
    m_tree->setDropMimeTypes({kContentMimeType, kObjectMimeType});
    m_list->setHorizontalPolicy(Widget::Expanding);
    m_list->setVerticalPolicy(Widget::Expanding);
    m_list->setRowHeight(24);
    m_list->setDragEnabled(true);
    m_list->setDragMimeType(kContentMimeType);
    m_list->setDragDataRole(AbstractItemModel::UserRole);
    m_list->setAcceptDrops(true);
    m_list->setDropMimeTypes({kContentMimeType, kObjectMimeType});
    m_list->setModel(m_model);
    m_tree->setModel(m_treeModel);
    splitter->addWidget(m_tree);
    splitter->addWidget(m_list);

    connect(m_tree, _SIGNAL(clicked(ModelIndex)), this, _SLOT(onTreeClicked(ModelIndex)));
    connect(m_tree, _SIGNAL(activated(ModelIndex)), this, _SLOT(onTreeActivated(ModelIndex)));
    connect(m_tree, _SIGNAL(dropped(TString,TString,ModelIndex)),
            this, _SLOT(onTreeDropped(TString,TString,ModelIndex)));
    connect(m_list, _SIGNAL(clicked(ModelIndex)), this, _SLOT(onListClicked(ModelIndex)));
    connect(m_list, _SIGNAL(activated(ModelIndex)), this, _SLOT(onListActivated(ModelIndex)));
    connect(m_list, _SIGNAL(dropped(TString,TString,ModelIndex)),
            this, _SLOT(onListDropped(TString,TString,ModelIndex)));

    m_currentDirectory = m_model->contentPath();
    setDirectory(m_currentDirectory);
    if(Editor::assets()) {
        connect(Editor::assets(), _SIGNAL(directoryChanged(TString)),
                this, _SLOT(onDirectoryChanged(TString)));
    }
}

void ContentBrowser::update(const Vector2 &pos) {
    Widget::update(pos);
    if(m_search) {
        TString filter = m_search->text().toLower().trimmed();
        if(filter != m_appliedFilter) {
            m_appliedFilter = filter;
            m_model->setFilter(filter);
            m_treeModel->setFilter(filter);
            if(m_tree) {
                m_tree->refresh();
            }
            if(m_list) {
                m_list->refresh();
            }
        }
    }
    if(m_refreshPending) {
        m_refreshPending = false;
        refresh();
    }
}

void ContentBrowser::onDirectoryChanged(const TString &path) {
    A_UNUSED(path);
    m_refreshPending = true;
}

void ContentBrowser::onNewButtonClicked() {
    if(m_newMenu) {
        m_newMenu->show(Vector2());
    }
}

void ContentBrowser::onImportButtonClicked() {
    importFiles();
}

void ContentBrowser::onActionsButtonClicked() {
    if(m_actionsMenu) {
        m_actionsMenu->show(Vector2());
    }
}

void ContentBrowser::onFilterButtonClicked() {
    if(m_filterMenu) {
        m_filterMenu->show(Vector2());
    }
}

void ContentBrowser::onUpButtonClicked() {
    Url parent(m_currentDirectory);
    if(parent.cdUp() && parent.filePath().length() >= m_model->contentPath().length()) {
        setDirectory(parent.absoluteFilePath());
    }
}

void ContentBrowser::onNewMenuTriggered(int index) {
    if(index == 0) {
        TString name("NewFolder");
        AssetManager::findFreeName(name, m_currentDirectory);
        TString path = m_currentDirectory + "/" + name;
        if(!File::mkDir(path)) {
            aError() << "Unable to create content folder:" << path;
            return;
        }
        Editor::assets()->directoryChanged(m_currentDirectory);
    } else if(index >= 2 && index - 2 < static_cast<int>(m_templatePaths.size())) {
        createAsset(m_templatePaths[index - 2]);
    }
}

void ContentBrowser::onActionMenuTriggered(int index) {
    ModelIndex selected = currentIndex();
    TString path = m_model->path(selected);
    if(index == 1) {
        showInExplorer();
        return;
    }
    if(index == 0) {
        if(selected.isValid() && m_model->isDirectory(selected)) {
            setDirectory(path);
        } else if(selected.isValid()) {
            openEditor(path);
        }
        return;
    }
    if(!selected.isValid()) {
        return;
    }

    switch(index) {
    case 3:
        Editor::assets()->duplicateResource(path);
        break;
    case 4:
        renameCurrentAsset();
        break;
    case 5:
        deleteCurrentAsset();
        break;
    case 6:
        if(m_settings && !m_model->isDirectory(selected)) {
            if(!Editor::assets()->pushToImport(m_settings)) {
                aWarning() << "Unable to queue asset for reimport:" << path;
            }
        }
        break;
    case 8:
        if(m_settings && m_settings->isModified()) {
            m_settings->saveSettings();
            if(!Editor::assets()->pushToImport(m_settings)) {
                aWarning() << "Unable to queue updated import settings:" << path;
            }
        }
        break;
    case 9:
        if(m_settings && m_settings->isModified()) {
            if(!m_settings->loadSettings()) {
                aWarning() << "Unable to restore import settings:" << path;
            } else {
                m_refreshPending = true;
            }
        }
        break;
    default:
        break;
    }
}

void ContentBrowser::onFilterMenuTriggered(int index) {
    if(!m_filterMenu) {
        return;
    }
    TString type = m_filterMenu->itemText(index);
    auto it = m_contentTypes.find(type);
    if(it == m_contentTypes.end()) {
        m_contentTypes.insert(type);
    } else {
        m_contentTypes.erase(it);
    }
    for(int i = 0; i < static_cast<int>(m_filterLabels.size()); ++i) {
        TString label = m_filterMenu->itemText(i);
        m_filterMenu->setItemState(i, true, true, m_contentTypes.find(label) != m_contentTypes.end());
    }
    m_model->setContentTypes(m_contentTypes);
    m_treeModel->setContentTypes(m_contentTypes);
    m_settings = nullptr;
    assetsSelected(nullptr);
    if(m_tree) {
        m_tree->refresh();
    }
    if(m_list) {
        m_list->refresh();
    }
}

void ContentBrowser::onTreeClicked(const ModelIndex &index) {
    if(m_treeModel->isDirectory(index)) {
        setDirectory(m_treeModel->path(index));
    }
}

void ContentBrowser::onTreeActivated(const ModelIndex &index) {
    if(m_treeModel->isDirectory(index)) {
        setDirectory(m_treeModel->path(index));
    } else {
        openEditor(m_treeModel->path(index));
    }
}

void ContentBrowser::onListClicked(const ModelIndex &index) {
    m_settings = nullptr;
    if(index.isValid() && !m_model->isDirectory(index) && Editor::assets()) {
        m_settings = Editor::assets()->fetchSettings(m_model->path(index));
    }
    assetsSelected(m_settings);
}

void ContentBrowser::onListActivated(const ModelIndex &index) {
    if(!index.isValid()) {
        return;
    }
    TString path = m_model->path(index);
    if(m_model->isDirectory(index)) {
        setDirectory(path);
    } else {
        openEditor(path);
    }
}

void ContentBrowser::onTreeDropped(const TString &mimeType, const TString &data, const ModelIndex &index) {
    TString target = m_treeModel->contentPath();
    if(index.isValid()) {
        if(!m_treeModel->isDirectory(index)) {
            return;
        }
        target = m_treeModel->path(index);
    }
    handleDrop(mimeType, data, target);
}

void ContentBrowser::onListDropped(const TString &mimeType, const TString &data, const ModelIndex &index) {
    TString target = m_currentDirectory;
    if(index.isValid()) {
        if(!m_model->isDirectory(index)) {
            return;
        }
        target = m_model->path(index);
    }
    handleDrop(mimeType, data, target);
}

void ContentBrowser::handleDrop(const TString &mimeType, const TString &data, const TString &targetPath) {
    if(!Editor::assets() || !File::isDir(targetPath)) {
        return;
    }

    if(mimeType == kContentMimeType) {
        const TString contentRoot = m_model->contentPath();
        for(const TString &source : data.split(';')) {
            if(source.isEmpty() || !File::exists(source) || source == targetPath) {
                continue;
            }
            if(!source.startsWith(contentRoot + "/")) {
                aWarning() << "Ignoring a resource dropped from outside the project content directory:" << source;
                continue;
            }
            if(File::isDir(source) && targetPath.startsWith(source + "/")) {
                aWarning() << "A folder cannot be moved into itself or one of its subfolders:" << source;
                continue;
            }

            Url sourceUrl(source);
            TString destination = targetPath + "/" + sourceUrl.name();
            if(destination == source) {
                continue;
            }
            if(File::exists(destination)) {
                aWarning() << "Unable to move resource because the destination already exists:" << destination;
                continue;
            }
            Editor::assets()->renameResource(source, destination);
        }
    } else if(mimeType == kObjectMimeType) {
        for(const TString &object : data.split(';')) {
            if(object.isEmpty()) {
                continue;
            }
            if(object.indexOf(':') <= 0) {
                aWarning() << "Ignoring malformed object drag data.";
                continue;
            }
            Editor::assets()->makePrefab(object, targetPath);
        }
    }
}

void ContentBrowser::setDirectory(const TString &path) {
    if(!m_model || !File::isDir(path)) {
        return;
    }
    m_currentDirectory = path;
    if(m_list) {
        ModelIndex root = m_model->indexForPath(path);
        m_list->setRootIndex(root);
    }
    m_settings = nullptr;
    assetsSelected(nullptr);
}

void ContentBrowser::refresh() {
    if(!m_model) {
        return;
    }
    m_model->refresh();
    m_treeModel->refresh();
    m_model->setFilter(m_appliedFilter);
    m_treeModel->setFilter(m_appliedFilter);
    m_model->setContentTypes(m_contentTypes);
    m_treeModel->setContentTypes(m_contentTypes);
    m_settings = nullptr;
    assetsSelected(nullptr);
    if(!File::isDir(m_currentDirectory)) {
        m_currentDirectory = m_model->contentPath();
    }
    if(m_tree) {
        m_tree->setModel(m_treeModel);
        m_tree->expandToDepth(0);
    }
    if(m_list) {
        m_list->setModel(m_model);
        m_list->setRootIndex(m_model->indexForPath(m_currentDirectory));
    }
}

ModelIndex ContentBrowser::currentIndex() const {
    return m_list ? m_list->currentIndex() : ModelIndex();
}

TString ContentBrowser::currentPath() const {
    ModelIndex index = currentIndex();
    return index.isValid() ? m_model->path(index) : m_currentDirectory;
}

void ContentBrowser::showInExplorer() {
    if(!Process::openExplorer(currentPath())) {
        aWarning() << "Unable to show content in the file explorer:" << currentPath();
    }
}

void ContentBrowser::importFiles() {
    FileDialog dialog;
    dialog.setDirectory(StandardPaths::writableLocation(StandardPaths::DocumentsLocation));
    dialog.setWindowTitle("Select files to import");
    dialog.setMode(FileDialog::OpenFiles);
    StringList extensions;
    extensions.push_back("*.*");
    dialog.addFilter("All", extensions);
    if(!dialog.exec()) {
        return;
    }
    importDroppedFiles(dialog.getSelectedFiles());
}

void ContentBrowser::importDroppedFiles(const StringList &files) {
    if(!Editor::assets()) {
        aWarning() << "Unable to import files because the asset manager is unavailable.";
        return;
    }
    for(const TString &file : files) {
        if(!Editor::assets()->import(file, m_currentDirectory)) {
            aWarning() << "Unable to import asset:" << file;
        }
    }
}

void ContentBrowser::createAsset(const TString &templatePath) {
    Url url(templatePath);
    TString name = TString("New") + url.baseName();
    TString suffix = url.suffix().isEmpty() ? TString() : TString(".") + url.suffix();
    AssetManager::findFreeName(name, m_currentDirectory, suffix);
    TString destination = m_currentDirectory + "/" + name + suffix;
    AssetConverter *converter = Editor::assets()->getConverter(destination);
    if(!converter) {
        aError() << "No asset converter is registered for:" << destination;
        return;
    }
    converter->createFromTemplate(destination);
    if(!File::exists(destination) || !Editor::assets()->pushToImport(destination)) {
        aError() << "Unable to create asset from template:" << templatePath;
        return;
    }
    Editor::assets()->directoryChanged(m_currentDirectory);
}

void ContentBrowser::renameCurrentAsset() {
    const TString source = currentPath();
    Url sourceUrl(source);
    FileDialog dialog;
    dialog.setDirectory(sourceUrl.absoluteDir());
    dialog.setWindowTitle("Rename asset");
    dialog.setMode(FileDialog::SaveFile);
    StringList extensions;
    extensions.push_back("*.*");
    dialog.addFilter("All", extensions);
    if(!dialog.exec()) {
        return;
    }
    TString target = dialog.getSelectedFile();
    if(target.isEmpty() || Url(target).absoluteDir() != sourceUrl.absoluteDir()) {
        aWarning() << "Renaming an asset outside its current folder is not supported.";
        return;
    }
    Editor::assets()->renameResource(source, target);
}

void ContentBrowser::deleteCurrentAsset() {
    Message message;
    message.setIcon(Message::Question);
    message.setText("Delete Asset");
    message.setInformativeText("This action cannot be reverted. Do you want to delete the selected asset?");
    message.setStandardButtons(Message::Yes | Message::No);
    message.setDefaultButton(Message::No);
    if(message.exec() == Message::Yes) {
        Editor::assets()->removeResource(currentPath());
        m_settings = nullptr;
        assetsSelected(nullptr);
    }
}
