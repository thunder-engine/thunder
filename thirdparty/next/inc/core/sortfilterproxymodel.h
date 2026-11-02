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

#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include "abstractitemmodel.h"

#include <vector>

class NEXT_LIBRARY_EXPORT SortFilterProxyModel : public AbstractItemModel {
public:
    enum SortOrder {
        AscendingOrder,
        DescendingOrder
    };

    enum CaseSensitivity {
        CaseSensitive,
        CaseInsensitive
    };

public:
    SortFilterProxyModel();

    void setSourceModel(AbstractItemModel *sourceModel);
    AbstractItemModel *sourceModel() const;

    void sort(int column, SortOrder order = AscendingOrder);
    int sortRole() const;
    void setSortRole(int role);

    int filterKeyColumn() const;
    void setFilterKeyColumn(int column);

    int filterRole() const;
    void setFilterRole(int role);

    CaseSensitivity filterCaseSensitivity() const;
    void setFilterCaseSensitivity(CaseSensitivity sensitivity);

    TString filterFixedString() const;
    void setFilterFixedString(const TString &filter);

    ModelIndex mapToSource(const ModelIndex &proxyIndex) const;
    ModelIndex mapFromSource(const ModelIndex &sourceIndex) const;

    int rowCount(const ModelIndex &parent = ModelIndex()) const override;
    int columnCount(const ModelIndex &parent = ModelIndex()) const override;
    ModelIndex index(int row, int column, const ModelIndex &parent = ModelIndex()) const override;
    ModelIndex parent(const ModelIndex &index) const override;
    Variant data(const ModelIndex &index, int role = DisplayRole) const override;
    bool setData(const ModelIndex &index, const Variant &value, int role = EditRole) override;
    int flags(const ModelIndex &index) const override;

protected:
    virtual bool filterAcceptsRow(int sourceRow, const ModelIndex &sourceParent) const;
    virtual bool lessThan(const ModelIndex &left, const ModelIndex &right) const;

private:
    std::vector<int> mappedRows(const ModelIndex &sourceParent) const;

    AbstractItemModel *m_sourceModel;
    int m_sortColumn;
    SortOrder m_sortOrder;
    int m_sortRole;
    int m_filterKeyColumn;
    int m_filterRole;
    CaseSensitivity m_filterCaseSensitivity;
    TString m_filter;
};

#endif // SORTFILTERPROXYMODEL_H
