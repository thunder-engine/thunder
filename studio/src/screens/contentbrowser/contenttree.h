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
#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include <abstractitemmodel.h>
#include <map>
#include <set>
#include <vector>

class ContentTree : public AbstractItemModel {
public:
    ContentTree();
    ~ContentTree() override;

    int rowCount(const ModelIndex &parent = ModelIndex()) const override;
    int columnCount(const ModelIndex &parent = ModelIndex()) const override;
    ModelIndex index(int row, int column, const ModelIndex &parent = ModelIndex()) const override;
    ModelIndex parent(const ModelIndex &index) const override;
    Variant data(const ModelIndex &index, int role = DisplayRole) const override;

    void refresh();
    void setFilter(const TString &filter);
    void setContentTypes(const std::set<TString> &types);
    void setDirectoriesOnly(bool enabled);

    TString path(const ModelIndex &index) const;
    TString contentPath() const;
    ModelIndex indexForPath(const TString &path) const;
    bool isDirectory(const ModelIndex &index) const;
    TString typeName(const ModelIndex &index) const;

private:
    struct Node {
        uint32_t id;
        TString path;
        TString name;
        TString type;
        Node *parent;
        std::vector<Node *> children;
        bool directory;

        Node();
    };

    Node *nodeForIndex(const ModelIndex &index) const;
    std::vector<Node *> visibleChildren(const Node *parent) const;
    bool matchesFilter(const Node *node) const;
    bool subtreeMatches(const Node *node) const;
    Node *findNode(const TString &path) const;
    void scanDirectory(Node *parent);
    void clearNodes();

private:
    Node *m_root;
    std::map<uint32_t, Node *> m_nodes;
    std::set<TString> m_contentTypes;
    TString m_filter;
    TString m_contentPath;
    uint32_t m_nextId;
    bool m_directoriesOnly;
};

#endif // CONTENTTREE_H
