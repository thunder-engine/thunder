#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <abstractitemview.h>
#include <vector>

class ItemViewDelegate;
class MaterialInstance;

class UIKIT_EXPORT TreeView : public AbstractItemView {
    A_OBJECT(TreeView, AbstractItemView, Components/UI)

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

    int rowHeight() const;
    void setRowHeight(int height);

    int indentation() const;
    void setIndentation(int indentation);

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

    void drawSub() override;

    Vector2 cellSize() const override;

private:
    void rebuildItems();
    void appendVisible(const ModelIndex &parent, int depth);
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

    void buildArrowInstances();

    bool isClickOnIndicator(int row, const Vector2 &pos) const;

    Vector4 getArrowRect(int row) const;

    bool isPointInRect(const Vector2 &point, const Vector4 &rect) const;

private:
    std::list<ItemViewDelegate *> m_items;
    std::vector<ModelIndex> m_visibleIndexes;
    std::list<ModelIndex> m_expandedIndexes;

    ItemViewDelegate *m_delegate;

    ByteArray m_instanceBuffer;

    int m_rowHeight;
    int m_indentation;
    int m_firstVisibleIndex;

    bool m_isPressed;
    bool m_dirtyItems;

    MaterialInstance *m_arrowMaterial;
    Sprite *m_arrowSprite;
    Mesh *m_arrowMesh;
};

#endif // TREEVIEW_H
