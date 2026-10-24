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
#include "objecthierarchymodel.h"

#include <object.h>
#include <objectsystem.h>
#include <component.h>
#include <invalid.h>
#include <components/scene.h>

#include <algorithm>

ObjectHierarchyModel::ObjectHierarchyModel() :
        m_root(nullptr) {
}

Object *ObjectHierarchyModel::root() const {
    return m_root;
}

void ObjectHierarchyModel::setRoot(Object *root) {
    m_root = root;
}

void ObjectHierarchyModel::setFilter(const TString &filter) {
    m_filter = filter.toLower().trimmed();
}

Object *ObjectHierarchyModel::objectForIndex(const ModelIndex &index) const {
    return index.isValid() ? Engine::findObject(index.internalId()) : m_root;
}

std::vector<Object *> ObjectHierarchyModel::children(Object *parentObject) const {
    std::vector<Object *> result;
    if(!parentObject) {
        return result;
    }
    for(Object *child : parentObject->getChildren()) {
        if(!child || dynamic_cast<Component *>(child) || dynamic_cast<Invalid *>(child)) {
            continue;
        }
        if(m_filter.isEmpty() || subtreeMatches(child)) {
            result.push_back(child);
        }
    }
    return result;
}

bool ObjectHierarchyModel::matchesFilter(const Object *object) const {
    if(!object || m_filter.isEmpty()) {
        return true;
    }
    return object->name().toLower().contains(m_filter) ||
           object->typeName().toLower().contains(m_filter);
}

bool ObjectHierarchyModel::subtreeMatches(const Object *object) const {
    if(matchesFilter(object)) {
        return true;
    }
    for(Object *child : object->getChildren()) {
        if(child && !dynamic_cast<Component *>(child) &&
           !dynamic_cast<Invalid *>(child) && subtreeMatches(child)) {
            return true;
        }
    }
    return false;
}

int ObjectHierarchyModel::rowCount(const ModelIndex &parentIndex) const {
    return static_cast<int>(children(objectForIndex(parentIndex)).size());
}

int ObjectHierarchyModel::columnCount(const ModelIndex &parentIndex) const {
    A_UNUSED(parentIndex);
    return 1;
}

ModelIndex ObjectHierarchyModel::index(int row, int column, const ModelIndex &parentIndex) const {
    if(row < 0 || column != 0) {
        return ModelIndex();
    }
    const std::vector<Object *> items = children(objectForIndex(parentIndex));
    if(row >= static_cast<int>(items.size())) {
        return ModelIndex();
    }
    return createIndex(row, column, items[row]->uuid());
}

ModelIndex ObjectHierarchyModel::parent(const ModelIndex &modelIndex) const {
    Object *item = objectForIndex(modelIndex);
    Object *parentObject = item ? item->parent() : nullptr;
    if(!parentObject || parentObject == m_root) {
        return ModelIndex();
    }
    Object *grandParent = parentObject->parent();
    if(!grandParent) {
        return ModelIndex();
    }
    const std::vector<Object *> siblings = children(grandParent);
    auto it = std::find(siblings.begin(), siblings.end(), parentObject);
    if(it == siblings.end()) {
        return ModelIndex();
    }
    return createIndex(static_cast<int>(std::distance(siblings.begin(), it)), 0, parentObject->uuid());
}

Variant ObjectHierarchyModel::data(const ModelIndex &modelIndex, int role) const {
    if(!modelIndex.isValid()) {
        return Variant();
    }
    Object *item = objectForIndex(modelIndex);
    if(!item) {
        return Variant();
    }
    if(role == UserRole) {
        return Variant(TString::number(static_cast<long long>(item->uuid())) + ":" + item->name());
    }
    if(role != DisplayRole && role != ToolTipRole && role != EditRole) {
        return Variant();
    }
    TString label = item->name();
    Scene *scene = dynamic_cast<Scene *>(item);
    if(scene && scene->isModified()) {
        label += " *";
    }
    return Variant(label);
}

ModelIndex ObjectHierarchyModel::indexForObject(const Object *object) const {
    if(!object || !m_root || object == m_root) {
        return ModelIndex();
    }
    return findObjectIndex(object, ModelIndex());
}

ModelIndex ObjectHierarchyModel::findObjectIndex(const Object *object, const ModelIndex &parentIndex) const {
    const std::vector<Object *> items = children(objectForIndex(parentIndex));
    for(size_t row = 0; row < items.size(); ++row) {
        ModelIndex itemIndex = createIndex(static_cast<int>(row), 0, items[row]->uuid());
        if(items[row] == object) {
            return itemIndex;
        }
        ModelIndex result = findObjectIndex(object, itemIndex);
        if(result.isValid()) {
            return result;
        }
    }
    return ModelIndex();
}
