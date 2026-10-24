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
#include "components/abstractitemview.h"

#include "components/lineedit.h"

#include <abstractitemmodel.h>
#include <input.h>

#include <algorithm>

AbstractItemView::AbstractItemView() :
        m_model(nullptr),
        m_rootIndex(),
        m_selectionMode(SingleSelection),
        m_iconPosition(TextBesideIcon),
        m_dragDataRole(AbstractItemModel::UserRole),
        m_dragEnabled(false),
        m_acceptDrops(false),
        m_editor(nullptr),
        m_editing(false) {
}

AbstractItemView::~AbstractItemView() {
}

void AbstractItemView::setModel(AbstractItemModel *model) {
    cancelEditing();
    m_model = model;
}

AbstractItemModel *AbstractItemView::model() const {
    return m_model;
}

void AbstractItemView::refresh() {
    repaint();
}

bool AbstractItemView::edit(const ModelIndex &index) {
    A_UNUSED(index);
    return false;
}

bool AbstractItemView::beginEditing(const ModelIndex &index, Widget *geometry) {
    if(m_editing || !m_model || !m_editor || !m_content || !geometry ||
       !isIndexValid(index) || !(m_model->flags(index) & AbstractItemModel::ItemIsEditable)) {
        return false;
    }

    Vector3 worldPosition = geometry->worldPosition();
    Vector2 localPosition = m_content->mapFromGlobal(worldPosition.x, worldPosition.y);
    m_editor->setParent(m_content, -1, true);
    m_editor->setAnchors(Vector2(0.0f), Vector2(0.0f));
    m_editor->setPivot(Vector2(0.0f));
    m_editor->setPosition(Vector3(localPosition, 0.0f));
    m_editor->setSize(geometry->size());

    Variant value = m_model->data(index, AbstractItemModel::EditRole);
    if(!value.isValid()) {
        value = m_model->data(index, AbstractItemModel::DisplayRole);
    }
    m_editor->setText(value.toString());
    m_editor->selectAll();
    m_editingIndex = index;
    m_editing = true;
    m_editor->setVisible(true);
    Widget::setFocusWidget(m_editor);
    return true;
}

void AbstractItemView::cancelEditing() {
    if(!m_editing) {
        return;
    }

    m_editing = false;
    m_editingIndex = ModelIndex();
    if(m_editor) {
        m_editor->setVisible(false);
    }
    Widget::setFocusWidget(this);
}

void AbstractItemView::onEditingFinished() {
    if(!m_editing) {
        return;
    }

    ModelIndex index = m_editingIndex;
    m_editing = false;
    m_editingIndex = ModelIndex();
    m_editor->setVisible(false);
    Widget::setFocusWidget(this);

    if(isIndexValid(index)) {
        m_model->setData(index, Variant(m_editor->text()), AbstractItemModel::EditRole);
        refresh();
    }
}

void AbstractItemView::onEditingCanceled() {
    cancelEditing();
}

void AbstractItemView::composeComponent() {
    AbstractScrollArea::composeComponent();

    Widget *editorActor = Widget::createWidget<LineEdit>("editor", this);
    m_editor = dynamic_cast<LineEdit *>(editorActor);
    if(m_editor) {
        connect(m_editor, _SIGNAL(editingFinished()), this, _SLOT(onEditingFinished()));
        connect(m_editor, _SIGNAL(focusOut()), this, _SLOT(onEditingFinished()));
        connect(m_editor, _SIGNAL(editingCanceled()), this, _SLOT(onEditingCanceled()));
        m_editor->setVisible(false);
        m_editor->setAnchors(Vector2(0.0f), Vector2(0.0f));
    }
}

void AbstractItemView::setRootIndex(const ModelIndex &index) {
    cancelEditing();
    m_rootIndex = index;
}

ModelIndex AbstractItemView::rootIndex() const {
    return m_rootIndex;
}

int AbstractItemView::selectionMode() const {
    return m_selectionMode;
}

void AbstractItemView::setSelectionMode(int mode) {
    m_selectionMode = mode;
    if(mode == NoSelection) {
        clearSelection();
    }
}

void AbstractItemView::selectItem(const ModelIndex &index) {
    if(!isIndexValid(index)) {
        return;
    }

    switch(m_selectionMode) {
    case NoSelection: return;
    case SingleSelection: {
        if(!m_selected.empty() && m_selected.front() == index) {
            return;
        }
        m_selected = {index};
        m_currentIndex = index;
        selectionChanged();
        break;
    }
        case MultiSelection: {
            auto it = std::find(m_selected.begin(), m_selected.end(), index);
            if(it != m_selected.end()) {
                if(m_selected.size() > 1) {
                    m_selected.erase(it);
                    selectionChanged();
                }
            } else {
                m_selected.push_back(index);
                m_currentIndex = index;
                selectionChanged();
            }
        } break;
        case ExtendedSelection: {
            selectItemWithModifiers(index);
        } break;
    }
}

void AbstractItemView::selectItemWithModifiers(const ModelIndex &index) {
    if(!isIndexValid(index)) {
        return;
    }

    bool ctrlPressed = Input::isKey(Input::KEY_LEFT_CONTROL) || Input::isKey(Input::KEY_RIGHT_CONTROL);
    bool shiftPressed = Input::isKey(Input::KEY_LEFT_SHIFT) || Input::isKey(Input::KEY_RIGHT_SHIFT);

    if(ctrlPressed) {
        toggleSelection(index);
    } else if (shiftPressed && m_currentIndex.isValid()) {
        selectRange(m_currentIndex, index);
    } else {
        if(!m_selected.empty() && m_selected.front() == index) {
            return;
        }
        m_selected = {index};
        m_currentIndex = index;
        selectionChanged();
    }
}

void AbstractItemView::toggleSelection(const ModelIndex &index) {
    if(!isIndexValid(index)) {
        return;
    }

    auto it = std::find(m_selected.begin(), m_selected.end(), index);
    if(it != m_selected.end()) {
        if(m_selected.size() > 1) {
            m_selected.erase(it);
            selectionChanged();
        }
    } else {
        m_selected.push_back(index);
        m_currentIndex = index;
        selectionChanged();
    }
}

void AbstractItemView::selectRange(const ModelIndex &from, const ModelIndex &to) {
    if(!isIndexValid(from) || !isIndexValid(to)) {
        return;
    }

    int fromRow = from.row();
    int toRow = to.row();
    if(fromRow > toRow) {
        std::swap(fromRow, toRow);
    }

    if(m_selectionMode == SingleSelection) {
        selectItem(to);
        return;
    }

    m_selected.clear();
    for(int row = fromRow; row <= toRow; ++row) {
        ModelIndex idx = m_model->index(row, 0, m_rootIndex);
        if (idx.isValid()) {
            m_selected.push_back(idx);
        }
    }
    m_currentIndex = to;
    selectionChanged();
}

void AbstractItemView::selectAll() {
    if(!m_model || m_selectionMode == NoSelection) {
        return;
    }

    m_selected.clear();
    int count = m_model->rowCount(m_rootIndex);
    for(int i = 0; i < count; ++i) {
        ModelIndex idx = m_model->index(i, 0, m_rootIndex);
        if(idx.isValid()) {
            m_selected.push_back(idx);
        }
    }
    if(!m_selected.empty()) {
        m_currentIndex = m_selected.back();
    }
    selectionChanged();
}

bool AbstractItemView::isIndexSelected(const ModelIndex &index) const {
    if(!index.isValid()) {
        return false;
    }
    return std::find(m_selected.begin(), m_selected.end(), index) != m_selected.end();
}

void AbstractItemView::setCurrentIndex(const ModelIndex &index) {
    if(!isIndexValid(index)) {
        return;
    }
    m_currentIndex = index;
}

void AbstractItemView::clearSelection() {
    m_selected.clear();
    m_currentIndex = ModelIndex();
    selectionChanged();
}

std::list<ModelIndex> AbstractItemView::selectedIndexes() const {
    return m_selected;
}

void AbstractItemView::setSelectedIndexes(const std::list<ModelIndex> &indexes) {
    m_selected.clear();
    for(const ModelIndex &index : indexes) {
        if(isIndexValid(index) &&
           std::find(m_selected.begin(), m_selected.end(), index) == m_selected.end()) {
            m_selected.push_back(index);
        }
    }
    m_currentIndex = m_selected.empty() ? ModelIndex() : m_selected.back();
    selectionChanged();
}

void AbstractItemView::setDragEnabled(bool enabled) {
    m_dragEnabled = enabled;
}

void AbstractItemView::setAcceptDrops(bool enabled) {
    m_acceptDrops = enabled;
}

void AbstractItemView::setDragMimeType(const TString &mimeType) {
    m_dragMimeType = mimeType;
}

void AbstractItemView::setDragDataRole(int role) {
    m_dragDataRole = role;
}

void AbstractItemView::setDropMimeTypes(const StringList &mimeTypes) {
    m_dropMimeTypes = mimeTypes;
}

void AbstractItemView::prepareDrag(const ModelIndex &index) {
    if(!m_dragEnabled || !m_model || !index.isValid() || m_dragMimeType.isEmpty()) {
        return;
    }

    std::list<ModelIndex> indexes = selectedIndexes();
    if(std::find(indexes.begin(), indexes.end(), index) == indexes.end()) {
        indexes.push_back(index);
    }

    TString data;
    for(const ModelIndex &selected : indexes) {
        TString value = m_model->data(selected, m_dragDataRole).toString();
        if(value.isEmpty()) {
            continue;
        }
        if(!data.isEmpty()) {
            data += ';';
        }
        data += value;
    }
    Widget::setDragCandidate(this, m_dragMimeType, data);
}

bool AbstractItemView::onDrop(const TString &mimeType, const TString &data, int x, int y) {
    if(!m_acceptDrops || data.isEmpty() ||
       std::find(m_dropMimeTypes.begin(), m_dropMimeTypes.end(), mimeType) == m_dropMimeTypes.end()) {
        return false;
    }
    dropped(mimeType, data, modelIndexAtPosition(Vector2(static_cast<float>(x), static_cast<float>(y))));
    return true;
}

ModelIndex AbstractItemView::modelIndexAtPosition(const Vector2 &position) {
    A_UNUSED(position);
    return ModelIndex();
}

bool AbstractItemView::dispatchContextMenuEvent(const Vector2 &pos) {
    if(!isEnabled() || !isVisible() || !isHovered(pos)) {
        return false;
    }

    Vector2 localPosition = mapFromGlobal(pos.x, pos.y);
    Vector2 eventPosition(localPosition.x, size().y - localPosition.y);
    onContextMenuRequested(eventPosition);
    contextMenuRequested(localPosition);
    return true;
}

void AbstractItemView::onContextMenuRequested(const Vector2 &position) {
    if(!m_model) {
        return;
    }

    ModelIndex index = modelIndexAtPosition(position);
    if(index.isValid()) {
        selectItem(index);
    } else if(!m_selected.empty()) {
        clearSelection();
    }

    updateSelectionVisuals();
}

void AbstractItemView::updateSelectionVisuals() {
    repaint();
}

void AbstractItemView::onDragFinished(bool dropped) {
    A_UNUSED(dropped);
}

Vector2 AbstractItemView::cellSize() const {
    return Vector2();
}

void AbstractItemView::activateCurrentItem() {
    if(m_currentIndex.isValid()) {
        activated(m_currentIndex);
    }
}

AbstractItemView::IconPosition AbstractItemView::iconPosition() const {
    return m_iconPosition;
}

bool AbstractItemView::isIndexValid(const ModelIndex &index) const {
    return index.isValid() && index.model() == m_model;
}

void AbstractItemView::activated(const ModelIndex &index) {
    emitSignal(_SIGNAL(activated(ModelIndex)), index);
}

void AbstractItemView::clicked(const ModelIndex &index) {
    emitSignal(_SIGNAL(clicked(ModelIndex)), index);
}

void AbstractItemView::pressed(const ModelIndex &index) {
    emitSignal(_SIGNAL(pressed(ModelIndex)), index);
}

void AbstractItemView::selectionChanged() {
    emitSignal(_SIGNAL(selectionChanged()));
}

void AbstractItemView::dropped(const TString &mimeType, const TString &data, const ModelIndex &index) {
    emitSignal(_SIGNAL(dropped(TString,TString,ModelIndex)), mimeType, data, index);
}
