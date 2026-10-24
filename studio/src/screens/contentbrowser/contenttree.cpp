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
#include "contenttree.h"

#include <editor/assetmanager.h>
#include <editor.h>
#include <editor/projectsettings.h>
#include <file.h>
#include <url.h>

#include "config.h"

#include <algorithm>

ContentTree::Node::Node() :
        id(0),
        parent(nullptr),
        directory(false) {
}

ContentTree::ContentTree() :
        m_root(new Node),
        m_contentRoot(nullptr),
        m_nextId(1),
        m_directoriesOnly(false) {
    m_root->directory = true;
    refresh();
}

ContentTree::~ContentTree() {
    clearNodes();
    delete m_root;
}

int ContentTree::rowCount(const ModelIndex &parentIndex) const {
    return static_cast<int>(visibleChildren(nodeForIndex(parentIndex)).size());
}

int ContentTree::columnCount(const ModelIndex &parentIndex) const {
    A_UNUSED(parentIndex);
    return 1;
}

ModelIndex ContentTree::index(int row, int column, const ModelIndex &parentIndex) const {
    if(row < 0 || column != 0) {
        return ModelIndex();
    }
    const std::vector<Node *> children = visibleChildren(nodeForIndex(parentIndex));
    if(row >= static_cast<int>(children.size())) {
        return ModelIndex();
    }
    Node *node = children[row];
    return createIndex(row, column, node->id);
}

ModelIndex ContentTree::parent(const ModelIndex &modelIndex) const {
    Node *node = nodeForIndex(modelIndex);
    if(!node || !node->parent || node->parent == m_root) {
        return ModelIndex();
    }
    std::vector<Node *> siblings = visibleChildren(node->parent->parent);
    auto it = std::find(siblings.begin(), siblings.end(), node->parent);
    if(it == siblings.end()) {
        return ModelIndex();
    }
    return createIndex(static_cast<int>(std::distance(siblings.begin(), it)), 0, node->parent->id);
}

Variant ContentTree::data(const ModelIndex &modelIndex, int role) const {
    Node *node = nodeForIndex(modelIndex);
    if(!node) {
        return Variant();
    }
    if(role == DisplayRole || role == EditRole) {
        return Variant(node->name);
    }
    if(role == ToolTipRole && !node->type.isEmpty()) {
        return Variant(node->type);
    }
    if(role == UserRole) {
        return Variant(node->path);
    }
    return Variant();
}

bool ContentTree::setData(const ModelIndex &modelIndex, const Variant &value, int role) {
    if(role != EditRole || !modelIndex.isValid() || modelIndex.model() != this || !Editor::assets()) {
        return false;
    }

    Node *node = nodeForIndex(modelIndex);
    TString name = value.toString().trimmed();
    if(!node || name.isEmpty() || name == "." || name == ".." ||
       name.contains("/") || name.contains("\\")) {
        return false;
    }

    Url sourceUrl(node->path);
    TString target = sourceUrl.absoluteDir() + "/" + name;
    if(!node->directory && !sourceUrl.suffix().isEmpty()) {
        target += TString(".") + sourceUrl.suffix();
    }
    if(target == node->path) {
        return true;
    }
    if(File::exists(target)) {
        return false;
    }

    Editor::assets()->renameResource(node->path, target);
    return File::exists(target);
}

int ContentTree::flags(const ModelIndex &modelIndex) const {
    return modelIndex.isValid() && modelIndex.model() == this && nodeForIndex(modelIndex) ?
               ItemIsSelectable | ItemIsEditable : NoItemFlags;
}

void ContentTree::refresh() {
    clearNodes();
    m_contentPath = Editor::project() ? Editor::project()->contentPath() : TString();
    m_root->path = m_contentPath;
    if(!m_contentPath.isEmpty() && File::isDir(m_contentPath)) {
        m_contentRoot = new Node;
        m_contentRoot->id = m_nextId++;
        m_contentRoot->path = m_contentPath;
        m_contentRoot->name = "Content";
        m_contentRoot->parent = m_root;
        m_contentRoot->directory = true;
        m_root->children.push_back(m_contentRoot);
        m_nodes[m_contentRoot->id] = m_contentRoot;
        scanDirectory(m_contentRoot);
    }
}

void ContentTree::setFilter(const TString &filter) {
    m_filter = filter.toLower().trimmed();
}

void ContentTree::setContentTypes(const std::set<TString> &types) {
    m_contentTypes = types;
}

void ContentTree::setDirectoriesOnly(bool enabled) {
    m_directoriesOnly = enabled;
}

TString ContentTree::path(const ModelIndex &index) const {
    Node *node = nodeForIndex(index);
    return node ? node->path : m_contentPath;
}

TString ContentTree::contentPath() const {
    return m_contentPath;
}

ModelIndex ContentTree::indexForPath(const TString &path) const {
    Node *node = findNode(path);
    if(!node || node == m_root) {
        return ModelIndex();
    }

    const std::vector<Node *> siblings = visibleChildren(node->parent);
    auto it = std::find(siblings.begin(), siblings.end(), node);
    if(it == siblings.end()) {
        return ModelIndex();
    }
    return createIndex(static_cast<int>(std::distance(siblings.begin(), it)), 0, node->id);
}

bool ContentTree::isDirectory(const ModelIndex &index) const {
    Node *node = nodeForIndex(index);
    return node && node->directory;
}

TString ContentTree::typeName(const ModelIndex &index) const {
    Node *node = nodeForIndex(index);
    return node ? node->type : TString();
}

ContentTree::Node *ContentTree::nodeForIndex(const ModelIndex &index) const {
    if(!index.isValid() || index.model() != this) {
        return m_root;
    }
    auto it = m_nodes.find(index.internalId());
    return it == m_nodes.end() ? nullptr : it->second;
}

std::vector<ContentTree::Node *> ContentTree::visibleChildren(const Node *parentNode) const {
    std::vector<Node *> result;
    if(!parentNode) {
        return result;
    }
    for(Node *node : parentNode->children) {
        if(m_directoriesOnly && !node->directory) {
            continue;
        }
        if((node->directory || node->type.isEmpty() || m_contentTypes.empty() ||
            m_contentTypes.find(node->type) != m_contentTypes.end()) &&
           (m_filter.isEmpty() || subtreeMatches(node))) {
            result.push_back(node);
        }
    }
    std::sort(result.begin(), result.end(), [](const Node *left, const Node *right) {
        if(left->directory != right->directory) {
            return left->directory;
        }
        return left->name.toLower() < right->name.toLower();
    });
    return result;
}

bool ContentTree::matchesFilter(const Node *node) const {
    if(!node) {
        return false;
    }
    return node->name.toLower().contains(m_filter) || node->type.toLower().contains(m_filter);
}

bool ContentTree::subtreeMatches(const Node *node) const {
    if(matchesFilter(node)) {
        return true;
    }
    for(Node *child : node->children) {
        if(subtreeMatches(child)) {
            return true;
        }
    }
    return false;
}

ContentTree::Node *ContentTree::findNode(const TString &path) const {
    if(path == m_contentPath) {
        return m_contentRoot;
    }
    for(const auto &entry : m_nodes) {
        if(entry.second->path == path) {
            return entry.second;
        }
    }
    return nullptr;
}

void ContentTree::scanDirectory(Node *parentNode) {
    StringList entries = File::list(parentNode->path, false);
    for(const TString &entry : entries) {
        Url url(entry);
        if(url.suffix() == gMetaExt) {
            continue;
        }

        const bool isDirectory = File::isDir(entry);
        if(m_directoriesOnly && !isDirectory) {
            continue;
        }
        Node *node = new Node;
        node->id = m_nextId++;
        node->path = entry;
        node->name = url.baseName();
        node->parent = parentNode;
        node->directory = isDirectory;
        if(!node->directory && Editor::assets()) {
            node->type = Editor::assets()->assetTypeName(entry);
        }
        parentNode->children.push_back(node);
        m_nodes[node->id] = node;

        if(node->directory) {
            scanDirectory(node);
        }
    }
}

void ContentTree::clearNodes() {
    for(auto &entry : m_nodes) {
        delete entry.second;
    }
    m_nodes.clear();
    m_contentRoot = nullptr;
    m_root->children.clear();
    m_nextId = 1;
}
