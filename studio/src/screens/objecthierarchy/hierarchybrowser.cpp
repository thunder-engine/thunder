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
#include "hierarchybrowser.h"

#include "objecthierarchymodel.h"

#include <button.h>
#include <frame.h>
#include <layout.h>
#include <lineedit.h>
#include <menu.h>
#include <treeview.h>

#include <engine.h>
#include <log.h>

namespace {
    const TString kContentMimeType("text/content");
    const TString kObjectMimeType("text/object");
}

HierarchyBrowser::HierarchyBrowser() :
        m_editor(nullptr),
        m_model(new ObjectHierarchyModel),
        m_addButton(nullptr),
        m_addMenu(nullptr),
        m_search(nullptr),
        m_tree(nullptr) {
}

HierarchyBrowser::~HierarchyBrowser() {
    delete m_model;
}

void HierarchyBrowser::setEditor(AssetEditor *editor) {
    if(m_editor == editor) {
        return;
    }
    if(m_editor) {
        disconnect(m_editor, _SIGNAL(objectsHierarchyChanged(Object *)), this, _SLOT(onObjectsHierarchyChanged(Object *)));
        disconnect(m_editor, _SIGNAL(selectionChanged()), this, _SLOT(onEditorSelectionChanged()));
    }
    m_editor = editor;

    onObjectsHierarchyChanged(nullptr);

    if(m_editor) {
        connect(m_editor, _SIGNAL(objectsHierarchyChanged(Object *)), this, _SLOT(onObjectsHierarchyChanged(Object *)));
        connect(m_editor, _SIGNAL(selectionChanged()), this, _SLOT(onEditorSelectionChanged()));
        m_editor->onActivated();
    }
}

void HierarchyBrowser::composeComponent() {
    Widget::composeComponent();

    Layout *layout = new Layout;
    layout->setOrientation(Widget::Vertical);
    layout->setSpacing(2);
    setLayout(layout);

    Widget *searchBar = Widget::createWidget<Widget>("searchBar", this);
    searchBar->setSize(Vector2(0.0f, 24.0f));
    searchBar->setHorizontalPolicy(Widget::Expanding);
    searchBar->setVerticalPolicy(Widget::Fixed);
    Layout *searchBarLayout = new Layout;
    searchBarLayout->setOrientation(Widget::Horizontal);
    searchBarLayout->setSpacing(2);
    searchBar->setLayout(searchBarLayout);
    layout->addWidget(searchBar);

    m_addButton = Widget::createWidget<Button>("addButton", searchBar);
    m_addButton->setText("+");
    m_addButton->setSize(Vector2(24.0f, 24.0f));
    m_addButton->setHorizontalPolicy(Widget::Fixed);
    m_addButton->setVerticalPolicy(Widget::Fixed);
    searchBarLayout->addWidget(m_addButton);
    connect(m_addButton, _SIGNAL(clicked()), this, _SLOT(onAddButtonClicked()));

    m_addMenu = Widget::createWidget<Menu>("addMenu", m_addButton);
    m_addButton->setSubWidget(m_addMenu);
    for(const auto &factory : Engine::factories()) {
        m_addMenu->addAction(factory.first);
    }
    connect(m_addMenu, _SIGNAL(triggered(int)), this, _SLOT(onAddMenuTriggered(int)));

    m_search = Widget::createWidget<LineEdit>("Search", this);
    m_search->setPlaceholderText("Search");
    m_search->setSize(Vector2(120.0f, 24.0f));
    m_search->setHorizontalPolicy(Widget::Expanding);
    m_search->setVerticalPolicy(Widget::Fixed);
    searchBarLayout->addWidget(m_search);

    m_tree = Widget::createWidget<TreeView>("hierarchyTree", this);
    m_tree->setHorizontalPolicy(Widget::Expanding);
    m_tree->setVerticalPolicy(Widget::Expanding);
    m_tree->setSelectionMode(AbstractItemView::ExtendedSelection);
    m_tree->setRowHeight(20);
    m_tree->setIndentation(16);
    m_tree->setDragEnabled(true);
    m_tree->setDragMimeType(kObjectMimeType);
    m_tree->setDragDataRole(AbstractItemModel::UserRole);
    m_tree->setAcceptDrops(true);
    m_tree->setDropMimeTypes({kObjectMimeType, kContentMimeType});
    m_tree->setModel(m_model);
    layout->addWidget(m_tree);
    connect(m_tree, _SIGNAL(selectionChanged()), this, _SLOT(onTreeSelectionChanged()));
    connect(m_tree, _SIGNAL(dropped(TString,TString,ModelIndex)),
            this, _SLOT(onTreeDropped(TString,TString,ModelIndex)));
}

void HierarchyBrowser::update(const Vector2 &pos) {
    Widget::update(pos);
    if(!m_search || !m_tree) {
        return;
    }
    TString filter = m_search->text().toLower().trimmed();
    if(filter != m_appliedFilter) {
        m_appliedFilter = filter;
        m_model->setFilter(filter);
        m_tree->expandToDepth(0);
        onEditorSelectionChanged();
    }
}

void HierarchyBrowser::onObjectsHierarchyChanged(Object *root) {
    m_model->setRoot(root);
    if(m_tree) {
        m_tree->setModel(m_model);
        m_tree->expandToDepth(0);
    }
    onEditorSelectionChanged();
}

void HierarchyBrowser::onEditorSelectionChanged() {
    if(!m_tree || !m_model) {
        return;
    }
    std::list<ModelIndex> indexes;
    if(m_editor) {
        for(Object *object : m_editor->selected()) {
            ModelIndex index = m_model->indexForObject(object);
            if(index.isValid()) {
                m_tree->expandToIndex(index);
                indexes.push_back(index);
            }
        }
    }
    const bool signalsBlocked = m_tree->isSignalsBlocked();
    m_tree->blockSignals(true);
    m_tree->setSelectedIndexes(indexes);
    m_tree->blockSignals(signalsBlocked);
    m_tree->refresh();
}

void HierarchyBrowser::onTreeSelectionChanged() {
    if(!m_editor || !m_tree) {
        return;
    }
    Object::ObjectList objects;
    for(const ModelIndex &index : m_tree->selectedIndexes()) {
        Object *object = Engine::findObject(index.internalId());
        if(object) {
            objects.push_back(object);
        }
    }
    m_editor->onObjectsSelected(objects, false);
}

void HierarchyBrowser::onAddButtonClicked() {
    if(m_addMenu && m_editor) {
        m_addMenu->show(Vector2());
    }
}

void HierarchyBrowser::onAddMenuTriggered(int index) {
    if(!m_editor || !m_addMenu) {
        return;
    }
    TString type = m_addMenu->itemText(index);
    if(!type.isEmpty()) {
        m_editor->onObjectCreate(type);
    }
}

void HierarchyBrowser::onTreeDropped(const TString &mimeType, const TString &data, const ModelIndex &index) {
    if(!m_editor || data.isEmpty()) {
        return;
    }

    if(mimeType == kContentMimeType) {
        for(const TString &path : data.split(';')) {
            if(!path.isEmpty()) {
                m_editor->onDropAsset(path, false);
            }
        }
        return;
    }
    if(mimeType != kObjectMimeType || !m_model->root()) {
        return;
    }

    Object *parent = index.isValid() ? Engine::findObject(index.internalId()) : m_model->root();
    if(!parent) {
        return;
    }

    Object::ObjectList objects;
    for(const TString &entry : data.split(';')) {
        const int separator = entry.indexOf(':');
        if(separator <= 0) {
            aWarning() << "Ignoring malformed object drag data.";
            continue;
        }
        const size_t id = entry.left(separator).toLong();
        Object *object = Engine::findObject(static_cast<uint32_t>(id));
        if(!object || object == parent) {
            continue;
        }

        bool createsCycle = false;
        for(Object *ancestor = parent; ancestor; ancestor = ancestor->parent()) {
            if(ancestor == object) {
                createsCycle = true;
                break;
            }
        }
        if(!createsCycle) {
            objects.push_back(object);
        }
    }

    if(!objects.empty()) {
        m_editor->changeParent(objects, parent);
    }
}
