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
#ifndef ABSTRACTITEMVIEW_H
#define ABSTRACTITEMVIEW_H

#include <abstractscrollarea.h>
#include <abstractitemmodel.h>

class UIKIT_EXPORT AbstractItemView : public AbstractScrollArea {
    A_OBJECT(AbstractItemView, AbstractScrollArea, UI)

    A_PROPERTIES(
        A_PROPERTYEX(int, selectionMode, AbstractItemView::selectionMode, AbstractItemView::setSelectionMode, "editor=SelectionMode")
    )
    A_METHODS(
        A_SIGNAL(AbstractItemView::activated),
        A_SIGNAL(AbstractItemView::clicked),
        A_SIGNAL(AbstractItemView::pressed),
        A_SIGNAL(AbstractItemView::selectionChanged),
        A_SIGNAL(AbstractItemView::dropped)
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

    enum IconPosition {
        TextBesideIcon = 0,
        TextUnderIcon
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
    void setSelectedIndexes(const std::list<ModelIndex> &indexes);

    void setDragEnabled(bool enabled);
    void setAcceptDrops(bool enabled);
    void setDragMimeType(const TString &mimeType);
    void setDragDataRole(int role);
    void setDropMimeTypes(const StringList &mimeTypes);

    virtual Vector2 cellSize() const;
    IconPosition iconPosition() const;

public: // signals
    void activated(const ModelIndex &index);

    void clicked(const ModelIndex &index);

    void pressed(const ModelIndex &index);

    void selectionChanged();

    void dropped(const TString &mimeType, const TString &data, const ModelIndex &index);

protected:
    virtual void selectItem(const ModelIndex &index);
    virtual void activateCurrentItem();

    bool isIndexValid(const ModelIndex &index) const;
    void clearSelection();
    void selectItemWithModifiers(const ModelIndex &index);
    void prepareDrag(const ModelIndex &index);
    bool onDrop(const TString &mimeType, const TString &data, int x, int y) override;
    virtual ModelIndex modelIndexAtPosition(const Vector2 &position);
    void onDragFinished(bool dropped) override;

protected:
    AbstractItemModel *m_model;

    std::list<ModelIndex> m_selected;

    ModelIndex m_rootIndex;
    ModelIndex m_currentIndex;

    int m_selectionMode;

    IconPosition m_iconPosition;
    TString m_dragMimeType;
    StringList m_dropMimeTypes;
    int m_dragDataRole;
    bool m_dragEnabled;
    bool m_acceptDrops;

};

#endif // ABSTRACTITEMVIEW_H
