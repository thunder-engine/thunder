#include "components/abstractitemview.h"

#include <abstractitemmodel.h>

AbstractItemView::AbstractItemView() :
        m_model(nullptr),
        m_rootIndex(),
        m_selectionMode(SingleSelection) {
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
}

std::list<ModelIndex> AbstractItemView::selectedIndexes() const {
    return m_selected;
}

void AbstractItemView::selectItem(const ModelIndex &index) {
    if(!isIndexValid(index)) {
        return;
    }

    switch(m_selectionMode) {
        case SingleSelection: {
                if(!m_selected.empty() && m_selected.front() == index) {
                    return;
                }
                m_selected = {index};
                m_currentIndex = index;
                selectionChanged();
        } break;
        case MultiSelection: {
            for(auto it = m_selected.begin(); it != m_selected.end(); ++it) {
                if(*it == index) {
                    m_selected.erase(it);
                    selectionChanged();
                    return;
                }
            }
            m_selected.push_back(index);
            m_currentIndex = index;
            selectionChanged();
        } break;
        default: break;
    }
}

void AbstractItemView::activateCurrentItem() {
    if(m_currentIndex.isValid()) {
        activated(m_currentIndex);
    }
}

bool AbstractItemView::isIndexValid(const ModelIndex &index) const {
    return index.isValid() && index.model() == m_model;
}

void AbstractItemView::clearSelection() {
    m_selected.clear();
    m_currentIndex = ModelIndex();
    selectionChanged();
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
