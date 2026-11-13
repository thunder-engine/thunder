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

#include "tst_common.h"

#include "sortfilterproxymodel.h"

#include <vector>

namespace NextSuite {
    class ProxyTestModel : public AbstractItemModel {
    public:
        struct Row {
            TString name;
            int value;
        };

        ProxyTestModel() :
                rows{{"Charlie", 3}, {"alice", 1}, {"Bob", 2}} {
        }

        int rowCount(const ModelIndex &parent = ModelIndex()) const override {
            return parent.isValid() ? 0 : static_cast<int>(rows.size());
        }

        int columnCount(const ModelIndex &parent = ModelIndex()) const override {
            return parent.isValid() ? 0 : 2;
        }

        ModelIndex index(int row, int column, const ModelIndex &parent = ModelIndex()) const override {
            if(parent.isValid() || row < 0 || row >= rowCount() || column < 0 || column >= columnCount()) {
                return ModelIndex();
            }
            return createIndex(row, column, 0);
        }

        ModelIndex parent(const ModelIndex &) const override {
            return ModelIndex();
        }

        Variant data(const ModelIndex &index, int = DisplayRole) const override {
            if(!index.isValid() || index.row() >= rowCount() || index.column() >= columnCount()) {
                return Variant();
            }
            return index.column() == 0 ? Variant(rows[index.row()].name) : Variant(rows[index.row()].value);
        }

        bool setData(const ModelIndex &index, const Variant &value, int = EditRole) override {
            if(!index.isValid() || index.row() >= rowCount() || index.column() != 1) {
                return false;
            }
            rows[index.row()].value = value.toInt();
            return true;
        }

        std::vector<Row> rows;
    };

    TEST(SortFilterProxyModelTest, SortsAndMapsRows) {
        ProxyTestModel source;
        SortFilterProxyModel proxy;
        proxy.setSourceModel(&source);
        proxy.sort(1);

        ASSERT_EQ(proxy.rowCount(), 3);
        ModelIndex first = proxy.index(0, 0);
        EXPECT_EQ(proxy.data(first).toString(), TString("alice"));
        EXPECT_EQ(proxy.mapToSource(first).row(), 1);
        EXPECT_EQ(proxy.mapFromSource(source.index(2, 0)).row(), 1);
        EXPECT_FALSE(first.parent().isValid());

        proxy.sort(1, SortFilterProxyModel::DescendingOrder);
        EXPECT_EQ(proxy.data(proxy.index(0, 0)).toString(), TString("Charlie"));
    }

    TEST(SortFilterProxyModelTest, FiltersAndForwardsData) {
        ProxyTestModel source;
        SortFilterProxyModel proxy;
        proxy.setSourceModel(&source);
        proxy.setFilterCaseSensitivity(SortFilterProxyModel::CaseInsensitive);
        proxy.setFilterFixedString("AL");

        ASSERT_EQ(proxy.rowCount(), 1);
        ModelIndex visibleIndex = proxy.index(0, 0);
        EXPECT_EQ(proxy.data(visibleIndex).toString(), TString("alice"));
        EXPECT_TRUE(proxy.setData(proxy.index(0, 1), 42));
        EXPECT_EQ(source.rows[1].value, 42);

        proxy.setFilterFixedString("");
        EXPECT_EQ(proxy.rowCount(), 3);
    }

    class ProxyTreeTestModel : public AbstractItemModel {
    public:
        int rowCount(const ModelIndex &parent = ModelIndex()) const override {
            return !parent.isValid() ? 1 : (parent.internalId() == 0 ? 1 : 0);
        }

        int columnCount(const ModelIndex &) const override {
            return 1;
        }

        ModelIndex index(int row, int column, const ModelIndex &parent = ModelIndex()) const override {
            if(column != 0 || row < 0 || row >= rowCount(parent)) {
                return ModelIndex();
            }
            return createIndex(row, column, parent.isValid() ? 1 : 0);
        }

        ModelIndex parent(const ModelIndex &index) const override {
            return index.internalId() == 1 ? createIndex(0, 0, 0) : ModelIndex();
        }

        Variant data(const ModelIndex &index, int = DisplayRole) const override {
            return index.internalId() == 1 ? Variant("child") : Variant("parent");
        }
    };

    TEST(SortFilterProxyModelTest, MapsParentIndexes) {
        ProxyTreeTestModel source;
        SortFilterProxyModel proxy;
        proxy.setSourceModel(&source);

        ModelIndex proxyParent = proxy.index(0, 0);
        ModelIndex proxyChild = proxy.index(0, 0, proxyParent);
        ASSERT_TRUE(proxyChild.isValid());
        EXPECT_EQ(proxyChild.parent(), proxyParent);
        EXPECT_EQ(proxy.data(proxyChild).toString(), TString("child"));
        EXPECT_EQ(proxy.mapToSource(proxyChild).parent(), source.index(0, 0));
    }
}
