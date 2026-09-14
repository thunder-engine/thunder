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
#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <abstractitemview.h>
#include <font.h>

class Frame;
class ItemViewDelegate;

class UIKIT_EXPORT ListView : public AbstractItemView {
    A_OBJECT(ListView, AbstractItemView, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(int, viewMode, ListView::viewMode, ListView::setViewMode, "enum=ViewMode"),
        A_PROPERTY(int, rowHeight, ListView::rowHeight, ListView::setRowHeight),
        A_PROPERTY(Vector2, gridSize, ListView::gridSize, ListView::setGridSize)
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(ViewMode,
            A_VALUE(ListMode),
            A_VALUE(IconMode)
        )
    )

public:
    enum ViewMode {
        ListMode = 0,
        IconMode = 1
    };

public:
    ListView();
    ~ListView();

    void setModel(AbstractItemModel *model) override;

    int viewMode() const;
    void setViewMode(int mode);

    int rowHeight() const;
    void setRowHeight(int height);

    Vector2 gridSize() const;
    void setGridSize(const Vector2 &size);

    ItemViewDelegate *delegate() const;
    void setDelegate(ItemViewDelegate *delegate);

protected:
    bool onMouseDown(int x, int y) override;
    bool onMouseUp(int x, int y) override;
    bool onMouseMove(int x, int y) override;
    bool onMouseDoubleClick(int x, int y) override;
    bool onMouseWheel(int delta, bool horizontal) override;
    bool onKeyPress(KeyEvent *event) override;

    void update(const Vector2 &pos) override;

    void composeComponent() override;
    void boundChanged(const Vector2 &size) override;
    void activateCurrentItem() override;

    void onVScrollChanged(int value) override;

    Vector2 cellSize() const override;

private:
    void rebuildItems();
    void handleItemClick(int index);
    void handleItemDoubleClick(int index);

    void calculateGridParams();

    int indexAtPosition(const Vector2 &pos);
    Vector2 positionAtIndex(int index);
    void scrollToItem(int index);

    Vector2 viewportSize();

    void updateDelegatesStates();

private:
    std::list<ItemViewDelegate *> m_items;

    Vector2 m_gridSize;

    ItemViewDelegate *m_delegate;

    int m_rowHeight;

    int m_viewMode;

    int m_firstVisibleIndex;

    int m_cachedColumns;

    int m_cachedRows;

    int m_iconAlignment = Alignment::Left | Alignment::Middle;

    bool m_isPressed = false;

    bool m_dirtyItems;
    
};

#endif // LISTVIEW_H
