#include "components/abstractitemview.h"

#include <abstractitemmodel.h>
#include <input.h>

#include <algorithm>

AbstractItemView::AbstractItemView() :
        m_model(nullptr),
        m_rootIndex(),
        m_selectionMode(SingleSelection),
        m_iconPosition(TextBesideIcon) {
}

AbstractItemView::~AbstractItemView() {
}

void AbstractItemView::setModel(AbstractItemModel *model) {
    m_model = model;
}

AbstractItemModel *AbstractItemView::model() const {
    return m_model;
}

void AbstractItemView::setRootIndex(const ModelIndex &index) {
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
        ModelIndex idx = m_model->index(row, 0);
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
    int count = m_model->rowCount();
    for(int i = 0; i < count; ++i) {
        ModelIndex idx = m_model->index(i, 0);
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
