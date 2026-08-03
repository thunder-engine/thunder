#ifndef ABSTRACTITEMVIEW_H
#define ABSTRACTITEMVIEW_H

#include "abstractscrollarea.h"
#include "abstractitemmodel.h"

class UIKIT_EXPORT AbstractItemView : public AbstractScrollArea {
    A_OBJECT(AbstractItemView, AbstractScrollArea, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(int, selectionMode, AbstractItemView::selectionMode, AbstractItemView::setSelectionMode, "editor=SelectionMode")
    )
    A_METHODS(
        A_SIGNAL(AbstractItemView::activated),
        A_SIGNAL(AbstractItemView::clicked),
        A_SIGNAL(AbstractItemView::pressed),
        A_SIGNAL(AbstractItemView::selectionChanged)
    )
    A_ENUMS(
        A_ENUM(SelectionMode,
            A_VALUE(NoSelection),
            A_VALUE(SingleSelection),
            A_VALUE(MultiSelection)
        )
    )

public:
    enum SelectionMode {
        NoSelection = 0,
        SingleSelection,
        MultiSelection,
        ExtendedSelection
    };

public:
    AbstractItemView();
    ~AbstractItemView();

    virtual void setModel(AbstractItemModel *model);
    AbstractItemModel *model() const;

    void setRootIndex(const ModelIndex &index);
    ModelIndex rootIndex() const;

    int selectionMode() const;
    void setSelectionMode(int mode);

    void selectAll();
    bool isIndexSelected(const ModelIndex &index) const;
    void toggleSelection(const ModelIndex &index);
    void selectRange(const ModelIndex &from, const ModelIndex &to);

    void setCurrentIndex(const ModelIndex &index);
    ModelIndex currentIndex() const { return m_currentIndex; }

    std::list<ModelIndex> selectedIndexes() const;

public: // signals
    void activated(const ModelIndex &index);

    void clicked(const ModelIndex &index);

    void pressed(const ModelIndex &index);

    void selectionChanged();

protected:
    virtual void selectItem(const ModelIndex &index);
    virtual void activateCurrentItem();

    bool isIndexValid(const ModelIndex &index) const;
    void clearSelection();
    void selectItemWithModifiers(const ModelIndex &index);

protected:
    AbstractItemModel *m_model;

    std::list<ModelIndex> m_selected;

    ModelIndex m_rootIndex;
    ModelIndex m_currentIndex;

    int m_selectionMode;

};

#endif // ABSTRACTITEMVIEW_H
