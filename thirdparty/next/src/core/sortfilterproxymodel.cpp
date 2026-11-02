/*
    This file is part of Thunder Next.

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

#include "core/sortfilterproxymodel.h"

#include <algorithm>
#include <cmath>
#include <iterator>

SortFilterProxyModel::SortFilterProxyModel() :
        m_sourceModel(nullptr),
        m_sortColumn(-1),
        m_sortOrder(AscendingOrder),
        m_sortRole(DisplayRole),
        m_filterKeyColumn(0),
        m_filterRole(DisplayRole),
        m_filterCaseSensitivity(CaseSensitive) {
}

void SortFilterProxyModel::setSourceModel(AbstractItemModel *sourceModel) {
    m_sourceModel = sourceModel;
}

AbstractItemModel *SortFilterProxyModel::sourceModel() const {
    return m_sourceModel;
}

void SortFilterProxyModel::sort(int column, SortOrder order) {
    m_sortColumn = column;
    m_sortOrder = order;
}

int SortFilterProxyModel::sortRole() const {
    return m_sortRole;
}

void SortFilterProxyModel::setSortRole(int role) {
    m_sortRole = role;
}

int SortFilterProxyModel::filterKeyColumn() const {
    return m_filterKeyColumn;
}

void SortFilterProxyModel::setFilterKeyColumn(int column) {
    m_filterKeyColumn = column;
}

int SortFilterProxyModel::filterRole() const {
    return m_filterRole;
}

void SortFilterProxyModel::setFilterRole(int role) {
    m_filterRole = role;
}

SortFilterProxyModel::CaseSensitivity SortFilterProxyModel::filterCaseSensitivity() const {
    return m_filterCaseSensitivity;
}

void SortFilterProxyModel::setFilterCaseSensitivity(CaseSensitivity sensitivity) {
    m_filterCaseSensitivity = sensitivity;
}

TString SortFilterProxyModel::filterFixedString() const {
    return m_filter;
}

void SortFilterProxyModel::setFilterFixedString(const TString &filter) {
    m_filter = filter;
}

ModelIndex SortFilterProxyModel::mapToSource(const ModelIndex &proxyIndex) const {
    if(!m_sourceModel || !proxyIndex.isValid() || proxyIndex.model() != this) {
        return ModelIndex();
    }

    ModelIndex sourceParent;
    if(proxyIndex.parent().isValid()) {
        sourceParent = mapToSource(proxyIndex.parent());
        if(!sourceParent.isValid()) {
            return ModelIndex();
        }
    }

    const std::vector<int> rows = mappedRows(sourceParent);
    if(proxyIndex.row() >= static_cast<int>(rows.size())) {
        return ModelIndex();
    }

    return m_sourceModel->index(rows[proxyIndex.row()], proxyIndex.column(), sourceParent);
}

ModelIndex SortFilterProxyModel::mapFromSource(const ModelIndex &sourceIndex) const {
    if(!m_sourceModel || !sourceIndex.isValid() || sourceIndex.model() != m_sourceModel) {
        return ModelIndex();
    }

    ModelIndex sourceParent = sourceIndex.parent();
    if(sourceParent.isValid() && !mapFromSource(sourceParent).isValid()) {
        return ModelIndex();
    }

    const std::vector<int> rows = mappedRows(sourceParent);
    const auto it = std::find(rows.begin(), rows.end(), sourceIndex.row());
    if(it == rows.end()) {
        return ModelIndex();
    }

    ModelIndex proxyParent = sourceParent.isValid() ? mapFromSource(sourceParent) : ModelIndex();
    return createIndex(static_cast<int>(std::distance(rows.begin(), it)),
                       sourceIndex.column(), sourceIndex.internalId(), proxyParent);
}

int SortFilterProxyModel::rowCount(const ModelIndex &parentIndex) const {
    if(!m_sourceModel || (parentIndex.isValid() && parentIndex.model() != this)) {
        return 0;
    }

    ModelIndex sourceParent;
    if(parentIndex.isValid()) {
        sourceParent = mapToSource(parentIndex);
        if(!sourceParent.isValid()) {
            return 0;
        }
    }

    return static_cast<int>(mappedRows(sourceParent).size());
}

int SortFilterProxyModel::columnCount(const ModelIndex &parentIndex) const {
    if(!m_sourceModel || (parentIndex.isValid() && parentIndex.model() != this)) {
        return 0;
    }

    ModelIndex sourceParent;
    if(parentIndex.isValid()) {
        sourceParent = mapToSource(parentIndex);
        if(!sourceParent.isValid()) {
            return 0;
        }
    }
    return m_sourceModel->columnCount(sourceParent);
}

ModelIndex SortFilterProxyModel::index(int row, int column, const ModelIndex &parentIndex) const {
    if(!m_sourceModel || row < 0 || column < 0 ||
       (parentIndex.isValid() && parentIndex.model() != this)) {
        return ModelIndex();
    }

    ModelIndex sourceParent;
    if(parentIndex.isValid()) {
        sourceParent = mapToSource(parentIndex);
        if(!sourceParent.isValid()) {
            return ModelIndex();
        }
    }

    const std::vector<int> rows = mappedRows(sourceParent);
    if(row >= static_cast<int>(rows.size()) || column >= m_sourceModel->columnCount(sourceParent)) {
        return ModelIndex();
    }

    ModelIndex sourceIndex = m_sourceModel->index(rows[row], column, sourceParent);
    if(!sourceIndex.isValid()) {
        return ModelIndex();
    }
    return createIndex(row, column, sourceIndex.internalId(), parentIndex);
}

ModelIndex SortFilterProxyModel::parent(const ModelIndex &proxyIndex) const {
    if(!proxyIndex.isValid() || proxyIndex.model() != this) {
        return ModelIndex();
    }

    ModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? mapFromSource(sourceIndex.parent()) : ModelIndex();
}

Variant SortFilterProxyModel::data(const ModelIndex &proxyIndex, int role) const {
    ModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? m_sourceModel->data(sourceIndex, role) : Variant();
}

bool SortFilterProxyModel::setData(const ModelIndex &proxyIndex, const Variant &value, int role) {
    ModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() && m_sourceModel->setData(sourceIndex, value, role);
}

int SortFilterProxyModel::flags(const ModelIndex &proxyIndex) const {
    ModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? m_sourceModel->flags(sourceIndex) : NoItemFlags;
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const ModelIndex &sourceParent) const {
    if(m_filter.isEmpty()) {
        return true;
    }

    const int columnCount = m_sourceModel->columnCount(sourceParent);
    if(m_filterKeyColumn >= columnCount) {
        return false;
    }
    const int firstColumn = m_filterKeyColumn < 0 ? 0 : m_filterKeyColumn;
    const int lastColumn = m_filterKeyColumn < 0 ? columnCount : m_filterKeyColumn + 1;
    for(int column = firstColumn; column < lastColumn; ++column) {
        ModelIndex sourceIndex = m_sourceModel->index(sourceRow, column, sourceParent);
        if(!sourceIndex.isValid()) {
            continue;
        }
        TString value = m_sourceModel->data(sourceIndex, m_filterRole).toString();
        if(m_filterCaseSensitivity == CaseInsensitive) {
            value = value.toLower();
            if(value.contains(m_filter.toLower())) {
                return true;
            }
        } else if(value.contains(m_filter)) {
            return true;
        }
    }
    return false;
}

bool SortFilterProxyModel::lessThan(const ModelIndex &left, const ModelIndex &right) const {
    const Variant leftValue = m_sourceModel->data(left, m_sortRole);
    const Variant rightValue = m_sourceModel->data(right, m_sortRole);

    if(leftValue.isValid() != rightValue.isValid()) {
        return !leftValue.isValid();
    }
    if(!leftValue.isValid()) {
        return false;
    }

    const uint32_t leftType = leftValue.type();
    const uint32_t rightType = rightValue.type();
    const bool leftNumeric = leftType == MetaType::INTEGER || leftType == MetaType::FLOAT;
    const bool rightNumeric = rightType == MetaType::INTEGER || rightType == MetaType::FLOAT;
    if(leftNumeric && rightNumeric) {
        const double leftNumber = leftType == MetaType::INTEGER
                                  ? leftValue.toInt() : leftValue.toFloat();
        const double rightNumber = rightType == MetaType::INTEGER
                                   ? rightValue.toInt() : rightValue.toFloat();
        const bool leftNan = std::isnan(leftNumber);
        const bool rightNan = std::isnan(rightNumber);
        if(leftNan != rightNan) {
            return leftNan;
        }
        return !leftNan && leftNumber < rightNumber;
    }
    if(leftType != rightType) {
        return leftType < rightType;
    }
    if(leftType == MetaType::BOOLEAN) {
        return !leftValue.toBool() && rightValue.toBool();
    }
    if(leftType == MetaType::STRING) {
        return leftValue.toString() < rightValue.toString();
    }
    return false;
}

std::vector<int> SortFilterProxyModel::mappedRows(const ModelIndex &sourceParent) const {
    std::vector<int> rows;
    if(!m_sourceModel) {
        return rows;
    }

    const int count = m_sourceModel->rowCount(sourceParent);
    rows.reserve(count);
    for(int row = 0; row < count; ++row) {
        if(filterAcceptsRow(row, sourceParent)) {
            rows.push_back(row);
        }
    }

    if(m_sortColumn >= 0 && m_sortColumn < m_sourceModel->columnCount(sourceParent)) {
        std::stable_sort(rows.begin(), rows.end(), [this, &sourceParent](int leftRow, int rightRow) {
            ModelIndex left = m_sourceModel->index(leftRow, m_sortColumn, sourceParent);
            ModelIndex right = m_sourceModel->index(rightRow, m_sortColumn, sourceParent);
            if(m_sortOrder == DescendingOrder) {
                return lessThan(right, left);
            }
            return lessThan(left, right);
        });
    }

    return rows;
}
