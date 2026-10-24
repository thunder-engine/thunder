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
#ifndef OBJECTHIERARCHYMODEL_H
#define OBJECTHIERARCHYMODEL_H

#include <abstractitemmodel.h>
#include <astring.h>
#include <modelindex.h>

#include <vector>

class Object;

class ObjectHierarchyModel : public AbstractItemModel {
public:
    ObjectHierarchyModel();

    Object *root() const;
    void setRoot(Object *root);

    void setFilter(const TString &filter);
    ModelIndex indexForObject(const Object *object) const;

    int rowCount(const ModelIndex &parent = ModelIndex()) const override;
    int columnCount(const ModelIndex &parent = ModelIndex()) const override;
    ModelIndex index(int row, int column, const ModelIndex &parent = ModelIndex()) const override;
    ModelIndex parent(const ModelIndex &index) const override;
    Variant data(const ModelIndex &index, int role = DisplayRole) const override;

private:
    Object *objectForIndex(const ModelIndex &index) const;
    std::vector<Object *> children(Object *parent) const;
    bool matchesFilter(const Object *object) const;
    bool subtreeMatches(const Object *object) const;
    ModelIndex findObjectIndex(const Object *object, const ModelIndex &parent) const;

private:
    Object *m_root;
    TString m_filter;
};

#endif // OBJECTHIERARCHYMODEL_H
