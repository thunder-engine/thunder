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
#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <abstractitemview.h>
#include <vector>

class ItemViewDelegate;

class UIKIT_EXPORT TreeView : public AbstractItemView {
    A_OBJECT(TreeView, AbstractItemView, UI)

    A_PROPERTIES(
        A_PROPERTY(int, rowHeight, TreeView::rowHeight, TreeView::setRowHeight),
        A_PROPERTY(int, indentation, TreeView::indentation, TreeView::setIndentation)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    TreeView();
    ~TreeView();

    void setModel(AbstractItemModel *model) override;
    bool edit(const ModelIndex &index) override;

    int rowHeight() const;
    void setRowHeight(int height);

    int indentation() const;
    void setIndentation(int indentation);

    void refresh() override;
    void expandToDepth(int depth);
    void expandToIndex(const ModelIndex &index);
    void scrollToIndex(const ModelIndex &index);
    ModelIndex modelIndexAtPosition(const Vector2 &position) override;

    ItemViewDelegate *delegate() const;
    void setDelegate(ItemViewDelegate *delegate);

protected:
    bool onMouseDown(int x, int y) override;
    bool onMouseUp(int x, int y) override;
    bool onMouseMove(int x, int y) override;
    bool onMouseDoubleClick(int x, int y) override;
    bool onKeyPress(KeyEvent *event) override;

    void update(const Vector2 &pos) override;
    void updateSelectionVisuals() override;
    void onDelegateAreaClicked(const ModelIndex &index, const TString &area) override;
    void composeComponent() override;
    void boundChanged(const Vector2 &size) override;
    void activateCurrentItem() override;
    void onVScrollChanged(int value) override;

    Vector2 cellSize() const override;
    void onDragFinished(bool dropped) override;

private:
    struct ItemData {
        ModelIndex index;
        ItemViewDelegate *delegate;

        ItemData() : delegate(nullptr) {}
        explicit ItemData(const ModelIndex &idx)
            : index(idx), delegate(nullptr) {}
    };

    void rebuildItems();
    void appendVisible(const ModelIndex &parent);
    void updateDelegatesStates();
    void handleItemClick(int row);
    void handleItemDoubleClick(int row);
    void scrollToItem(int row);
    int indexAtPosition(const Vector2 &pos) const;
    Vector2 positionAtIndex(int row) const;
    bool isExpanded(const ModelIndex &index) const;
    void setExpanded(const ModelIndex &index, bool expanded);
    void toggleExpanded(int row);
    Vector2 viewportSize();
    void clearDelegates();

    ItemData *getItemData(int row);
    const ItemData *getItemData(int row) const;

private:
    std::vector<ItemData> m_itemsData;
    std::list<ModelIndex> m_expandedIndexes;

    ItemViewDelegate *m_defaultDelegate;
    ItemViewDelegate *m_delegate;

    int m_rowHeight;
    int m_indentation;
    int m_firstVisibleIndex;

    bool m_isPressed;
    bool m_dirtyItems;

};

#endif // TREEVIEW_H
