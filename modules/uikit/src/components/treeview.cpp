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
#include "treeview.h"

#include "itemviewdelegate.h"
#include "scrollbar.h"
#include "canvas.h"

#include <input.h>
#include <resources/material.h>
#include <resources/mesh.h>

#include <abstractitemmodel.h>

#include <algorithm>
#include <string.h>

TreeView::TreeView() :
        m_delegate(nullptr),
        m_rowHeight(20),
        m_indentation(16),
        m_firstVisibleIndex(0),
        m_isPressed(false),
        m_dirtyItems(true),
        m_arrowMaterial(nullptr),
        m_arrowSprite(nullptr),
        m_arrowMesh(nullptr) {
}

TreeView::~TreeView() {
    clearDelegates();
    if(m_delegate) {
        delete m_delegate;
    }
    if(m_arrowMaterial) {
        m_arrowMaterial->material()->unsubscribe(this);
        delete m_arrowMaterial;
    }
    if(m_arrowSprite) {
        m_arrowSprite->unsubscribe(this);
    }
}

void TreeView::setModel(AbstractItemModel *model) {
    AbstractItemView::setModel(model);

    clearDelegates();

    m_itemsData.clear();
    m_expandedIndexes.clear();
    m_firstVisibleIndex = 0;
    m_dirtyItems = true;
}

void TreeView::refresh() {
    m_dirtyItems = true;
}

bool TreeView::edit(const ModelIndex &index) {
    if(!m_model || !index.isValid() || index.model() != m_model || index.column() != 0) {
        return false;
    }

    expandToIndex(index);
    if(!m_content) {
        return false;
    }

    rebuildItems();
    m_dirtyItems = false;
    for(size_t row = 0; row < m_itemsData.size(); ++row) {
        if(m_itemsData[row].index == index) {
            scrollToItem(static_cast<int>(row));
            rebuildItems();
            m_dirtyItems = false;
            ItemData *data = getItemData(static_cast<int>(row));
            return data && data->delegate && data->delegate->isEnabled() &&
                   data->delegate->beginEditing(this);
        }
    }
    return false;
}

void TreeView::expandToDepth(int depth) {
    if(!m_model || depth < 0) {
        return;
    }
    auto expand = [this, depth](const auto &self, const ModelIndex &parent, int level) -> void {
        const int count = m_model->rowCount(parent);
        for(int row = 0; row < count; ++row) {
            ModelIndex index = m_model->index(row, 0, parent);
            if(!index.isValid()) {
                continue;
            }
            if(level <= depth && m_model->rowCount(index) > 0) {
                setExpanded(index, true);
            }
            if(level < depth) {
                self(self, index, level + 1);
            }
        }
    };
    expand(expand, m_rootIndex, 0);
}

void TreeView::expandToIndex(const ModelIndex &index) {
    if(!m_model || !index.isValid() || index.model() != m_model) {
        return;
    }
    std::vector<ModelIndex> ancestors;
    ModelIndex ancestor = index.parent();
    while(ancestor.isValid()) {
        ancestors.push_back(ancestor);
        ancestor = ancestor.parent();
    }
    for(auto it = ancestors.rbegin(); it != ancestors.rend(); ++it) {
        setExpanded(*it, true);
    }
}

int TreeView::rowHeight() const {
    return m_rowHeight;
}

void TreeView::setRowHeight(int height) {
    m_rowHeight = std::max(1, height);
    if(m_vScroll) {
        m_vScroll->setSingleStep(m_rowHeight);
    }
    m_dirtyItems = true;
}

int TreeView::indentation() const {
    return m_indentation;
}

void TreeView::setIndentation(int indentation) {
    m_indentation = std::max(0, indentation);
    m_dirtyItems = true;
}

ItemViewDelegate *TreeView::delegate() const {
    return m_delegate;
}

void TreeView::setDelegate(ItemViewDelegate *delegate) {
    clearDelegates();
    if(m_delegate) {
        delete m_delegate;
    }
    m_delegate = delegate;
    m_dirtyItems = true;
}

void TreeView::composeComponent() {
    AbstractItemView::composeComponent();

    setContent(Widget::createWidget<Widget>("content", this));
    setDelegate(Widget::createWidget<ItemViewDelegate>("delegate"));
    m_delegate->setEnabled(false);
    m_delegate->setVisible(false);

    setRowHeight(m_rowHeight);

    m_arrowSprite = Engine::loadResource<Sprite>(".embedded/ui.png/Arrow");
    if(m_arrowSprite) {
        m_arrowSprite->subscribe(&TreeView::arrowUpdated, this);
        m_arrowMesh = m_arrowSprite->mesh();

        Material *defaultMaterial = Engine::loadResource<Material>(".embedded/DefaultUI.shader");
        if(defaultMaterial) {
            m_arrowMaterial = defaultMaterial->createInstance();
            m_arrowMaterial->setTexture("mainTexture", m_arrowSprite->texture());
            defaultMaterial->subscribe(&TreeView::materialUpdated, this);
        }
    }
}

void TreeView::appendVisible(const ModelIndex &parent, int depth) {
    if(!m_model) {
        return;
    }
    int count = m_model->rowCount(parent);
    for(int row = 0; row < count; ++row) {
        ModelIndex index = m_model->index(row, 0, parent);
        if(!index.isValid()) continue;
        m_itemsData.push_back(ItemData(index, depth));
        if(isExpanded(index)) {
            appendVisible(index, depth + 1);
        }
    }
}

bool TreeView::isExpanded(const ModelIndex &index) const {
    return std::find(m_expandedIndexes.begin(), m_expandedIndexes.end(), index) != m_expandedIndexes.end();
}

void TreeView::setExpanded(const ModelIndex &index, bool expanded) {
    auto it = std::find(m_expandedIndexes.begin(), m_expandedIndexes.end(), index);
    if(expanded && it == m_expandedIndexes.end()) {
        m_expandedIndexes.push_back(index);
    }
    if(!expanded && it != m_expandedIndexes.end()) {
        m_expandedIndexes.erase(it);
    }
    m_dirtyItems = true;
}

void TreeView::toggleExpanded(int row) {
    ItemData *data = getItemData(row);
    if(!data) {
        return;
    }
    if(m_model->rowCount(data->index) > 0) {
        setExpanded(data->index, !isExpanded(data->index));
    }
}

void TreeView::clearDelegates() {
    for(auto &data : m_itemsData) {
        if(data.delegate) {
            data.delegate->setEnabled(false);
            data.delegate->setVisible(false);
            data.delegate->deleteLater();
        }
        data.delegate = nullptr;
    }
}

TreeView::ItemData* TreeView::getItemData(int row) {
    if(row < 0 || row >= static_cast<int>(m_itemsData.size())) {
        return nullptr;
    }
    return &m_itemsData[row];
}

const TreeView::ItemData* TreeView::getItemData(int row) const {
    if(row < 0 || row >= static_cast<int>(m_itemsData.size())) {
        return nullptr;
    }
    return &m_itemsData[row];
}

float TreeView::getArrowSize() const {
    return static_cast<float>(m_rowHeight) * 0.5f;
}

float TreeView::getArrowWidth() const {
    if(!m_arrowSprite) return getArrowSize();

    Vector4 bounds = m_arrowSprite->bounds();
    float spriteWidth = bounds.z - bounds.x;
    float spriteHeight = bounds.w - bounds.y;
    float aspectRatio = spriteWidth / spriteHeight;

    return getArrowSize() * aspectRatio;
}

float TreeView::getArrowHeight() const {
    return getArrowSize();
}

float TreeView::getArrowOffset() const {
    return m_rowHeight * 0.5f;
}

void TreeView::arrowUpdated(int state, void *ptr) {
    TreeView *treeView = static_cast<TreeView *>(ptr);
    if(!treeView) {
        return;
    }

    if(state == Resource::Ready && treeView->m_arrowSprite) {
        treeView->m_arrowMesh = treeView->m_arrowSprite->mesh();
        if(treeView->m_arrowMaterial) {
            treeView->m_arrowMaterial->setTexture("mainTexture", treeView->m_arrowSprite->texture());
        }
        treeView->repaint();
    } else if(state == Resource::ToBeDeleted) {
        if(treeView->m_arrowSprite) {
            treeView->m_arrowSprite->unsubscribe(treeView);
        }
        treeView->m_arrowSprite = nullptr;
        treeView->m_arrowMesh = nullptr;
        if(treeView->m_arrowMaterial) {
            treeView->m_arrowMaterial->setTexture("mainTexture", nullptr);
        }
        treeView->repaint();
    }
}

void TreeView::materialUpdated(int state, void *ptr) {
    TreeView *treeView = static_cast<TreeView *>(ptr);
    if(!treeView) {
        return;
    }

    if(state == Material::ToBeDeleted) {
        if(treeView->m_arrowMaterial) {
            delete treeView->m_arrowMaterial;
            treeView->m_arrowMaterial = nullptr;
        }
    }

    treeView->repaint();
}

void TreeView::rebuildItems() {
    if(!m_model || !m_content) {
        return;
    }

    std::vector<ItemViewDelegate*> existingDelegates;
    for(auto &data : m_itemsData) {
        if(data.delegate) {
            existingDelegates.push_back(data.delegate);
        }
    }

    m_itemsData.clear();
    appendVisible(m_rootIndex, 0);

    if(m_itemsData.empty()) {
        for(auto delegate : existingDelegates) {
            if(delegate) {
                delegate->setVisible(false);
                delegate->setEnabled(false);
            }
        }
        m_content->setSize(Vector2(viewportSize().x, 0));
        updateScrollRange();
        return;
    }

    m_firstVisibleIndex = std::min(m_firstVisibleIndex, std::max(0, static_cast<int>(m_itemsData.size()) - 1));

    int visibleCount = std::max(1, static_cast<int>(viewportSize().y / m_rowHeight) + 2);

    int delegateIndex = 0;
    float arrowWidth = getArrowWidth();

    for(int row = m_firstVisibleIndex; row < m_firstVisibleIndex + visibleCount && row < static_cast<int>(m_itemsData.size()); ++row) {
        ItemData &data = m_itemsData[row];

        ItemViewDelegate *delegate = nullptr;
        if(delegateIndex < static_cast<int>(existingDelegates.size())) {
            delegate = existingDelegates[delegateIndex];
            delegateIndex++;
        } else if(m_delegate) {
            delegate = static_cast<ItemViewDelegate *>(m_delegate->clone(m_content));
            if(delegate) {
                m_content->setSubWidget(delegate);
            }
        }

        data.delegate = delegate;
        if(delegate) {
            delegate->setEnabled(true);
            delegate->setVisible(true);
            delegate->bind(this, data.index);

            delegate->setAnchors(Vector2(0, 1), Vector2(0, 1));
            delegate->setPivot(Vector2(0, 1));

            float leftPadding = data.depth * m_indentation + arrowWidth;
            delegate->setSize(Vector2(viewportSize().x - leftPadding, m_rowHeight));
            delegate->setPosition(Vector3(positionAtIndex(row) + Vector2(leftPadding, 0), 0));
        }
    }

    for(int i = delegateIndex; i < static_cast<int>(existingDelegates.size()); ++i) {
        if(existingDelegates[i]) {
            existingDelegates[i]->setEnabled(false);
            existingDelegates[i]->setVisible(false);
        }
    }

    m_content->setSize(Vector2(viewportSize().x, m_itemsData.size() * m_rowHeight));

    updateDelegatesStates();
    updateScrollRange();
    repaint();
}

void TreeView::updateDelegatesStates() {
    for(auto &data : m_itemsData) {
        if(data.delegate && data.delegate->isEnabled()) {
            data.delegate->setSelected(isIndexSelected(data.index));
        }
    }
}

void TreeView::updateSelectionVisuals() {
    updateDelegatesStates();
    repaint();
}

void TreeView::buildArrowInstances() {
    if(!m_arrowMaterial || !m_arrowMesh || !m_arrowSprite) {
        return;
    }

    Canvas *canvas = TreeView::canvas();
    if(!canvas) {
        return;
    }

    float arrowWidth = getArrowWidth();
    float arrowHeight = getArrowHeight();

    Vector4 bounds = m_arrowSprite->bounds();
    float spriteWidth = bounds.z - bounds.x;
    float spriteHeight = bounds.w - bounds.y;

    static Matrix4 rotation;
    if(rotation[4] != -1) {
        rotation[0] = 0.0f;
        rotation[1] = 1.0f;
        rotation[4] = -1.0f;
        rotation[5] = 0.0f;
    }

    int arrowCount = 0;
    Vector2 viewSize = viewportSize();
    float scrollOffset = m_vScroll ? -m_vScroll->value() : 0;

    for(auto &data : m_itemsData) {
        if(!data.delegate || !data.delegate->isEnabled()) continue;
        if(m_model->rowCount(data.index) == 0) continue;

        Vector3 pos = data.delegate->position();
        float yPos = -pos.y + scrollOffset;

        if(yPos >= -m_rowHeight && yPos <= viewSize.y) {
            arrowCount++;
        }
    }

    if(arrowCount == 0) {
        m_arrowMaterial->setInstanceBuffer(nullptr);
        return;
    }

    size_t instanceSize = m_arrowMaterial->instanceSize();
    m_instanceBuffer.resize(instanceSize * arrowCount);

    uint8_t *data = m_instanceBuffer.data();
    int instanceIndex = 0;

    Vector4 color(1.0f);
    for(auto &itemData : m_itemsData) {
        if(!itemData.delegate || !itemData.delegate->isEnabled()) continue;
        if(m_model->rowCount(itemData.index) == 0) continue;

        Matrix4 transform = itemData.delegate->worldTransform();
        Vector2 scale = itemData.delegate->worldScale();

        float offsetX = -(m_rowHeight * 0.5f) * scale.x;
        float offsetY = m_rowHeight * 0.5f * scale.y;

        transform[12] += offsetX;
        transform[13] += offsetY;

        transform[0] = (arrowWidth / spriteWidth) * scale.x;
        transform[5] = (arrowHeight / spriteHeight) * scale.y;

        if(!isExpanded(itemData.index)) {
            transform = transform * rotation;
        }

        memcpy(data + instanceIndex * instanceSize, &transform, sizeof(Matrix4));
        memcpy(data + instanceIndex * instanceSize + sizeof(Matrix4), &color, sizeof(Vector4));

        instanceIndex++;
    }

    m_arrowMaterial->setInstanceBuffer(&m_instanceBuffer);
}

Vector4 TreeView::getArrowRect(int row) const {
    const ItemData *data = getItemData(row);
    if(!data) {
        return Vector4();
    }

    if(m_model->rowCount(data->index) == 0) {
        return Vector4();
    }

    if(!data->delegate || !data->delegate->isEnabled()) {
        return Vector4();
    }

    Vector2 localPos = data->delegate->position();
    if(m_content) {
        localPos += m_content->position();
    }
    Vector2 scale = data->delegate->worldScale();

    return Vector4(localPos.x - m_rowHeight * scale.x, -localPos.y, m_rowHeight * scale.x, m_rowHeight * scale.y);
}

void TreeView::drawSub() {
    AbstractItemView::drawSub();

    buildArrowInstances();

    Canvas *canvas = TreeView::canvas();
    if(canvas && m_arrowMaterial && m_arrowMesh) {
        canvas->setClipRegion(clipRegion());
        canvas->drawMesh(m_arrowMesh, m_arrowMaterial);
        canvas->disableClip();

        m_arrowMaterial->setInstanceBuffer(nullptr);
    }
}

bool TreeView::isPointInRect(const Vector2 &point, const Vector4 &rect) const {
    return point.x >= rect.x && point.x <= rect.x + rect.z &&
           point.y >= rect.y && point.y <= rect.y + rect.w;
}

bool TreeView::isClickOnIndicator(int row, const Vector2 &pos) const {
    return isPointInRect(pos, getArrowRect(row));
}

int TreeView::indexAtPosition(const Vector2 &pos) const {
    Vector2 local(pos);
    if(m_content) {
        local += m_content->position();
    }
    int row = static_cast<int>(local.y / m_rowHeight);
    return row >= 0 && row < static_cast<int>(m_itemsData.size()) ? row : -1;
}

Vector2 TreeView::positionAtIndex(int row) const {
    return Vector2(0, -row * m_rowHeight);
}

Vector2 TreeView::viewportSize() {
    return size();
}

void TreeView::handleItemClick(int row) {
    ItemData *data = getItemData(row);
    if(!data) {
        return;
    }
    selectItem(data->index);
    m_currentIndex = data->index;
    updateDelegatesStates();
    repaint();
    pressed(m_currentIndex);
}

void TreeView::handleItemDoubleClick(int row) {
    ItemData *data = getItemData(row);
    if(!data) {
        return;
    }
    handleItemClick(row);
    activateCurrentItem();
    edit(data->index);
}

bool TreeView::onMouseDown(int x, int y) {
    Widget::setFocusWidget(this);
    Vector2 mousePos(x, y);
    int row = indexAtPosition(mousePos);

    if(row < 0) {
        if(!m_selected.empty()) {
            clearSelection();
            updateDelegatesStates();
            repaint();
        }
        return false;
    }

    if(isClickOnIndicator(row, mousePos)) {
        toggleExpanded(row);
        return true;
    }

    m_isPressed = true;
    handleItemClick(row);
    prepareDrag(m_itemsData[row].index);
    return true;
}

bool TreeView::onMouseUp(int x, int y) {
    if(!m_isPressed) {
        Widget::cancelDragCandidate(this);
        return false;
    }
    m_isPressed = false;
    int row = indexAtPosition(Vector2(x, y));
    if(row >= 0 && row < static_cast<int>(m_itemsData.size())) {
        clicked(m_itemsData[row].index);
        Widget::cancelDragCandidate(this);
        return true;
    }
    Widget::cancelDragCandidate(this);
    return false;
}

bool TreeView::onMouseMove(int x, int y) {
    int row = indexAtPosition(Vector2(x, y));
    for(auto &data : m_itemsData) {
        if(data.delegate && data.delegate->isEnabled()) {
            bool hovered = false;
            if(row >= 0 && row < static_cast<int>(m_itemsData.size())) {
                hovered = data.index == m_itemsData[row].index;
            }
            data.delegate->setHovered(hovered);
        }
    }
    repaint();
    return true;
}

ModelIndex TreeView::modelIndexAtPosition(const Vector2 &position) {
    int row = indexAtPosition(position);
    const ItemData *data = getItemData(row);
    return data ? data->index : ModelIndex();
}

void TreeView::onDragFinished(bool dropped) {
    AbstractItemView::onDragFinished(dropped);
    m_isPressed = false;
}

bool TreeView::onMouseDoubleClick(int x, int y) {
    int row = indexAtPosition(Vector2(x, y));
    if(row < 0) {
        return false;
    }

    ItemData *data = getItemData(row);
    if(data == nullptr) {
        return false;
    }
    handleItemDoubleClick(row);
    return true;
}

bool TreeView::onMouseWheel(int delta, bool horizontal) {
    if(horizontal || !m_vScroll) {
        return false;
    }
    int newValue = m_vScroll->value() - delta * m_rowHeight;
    newValue = std::max(m_vScroll->minimum(), std::min(m_vScroll->maximum(), newValue));
    m_vScroll->setValue(newValue);
    return true;
}

bool TreeView::onKeyPress(KeyEvent *event) {
    if(m_itemsData.empty()) {
        return false;
    }

    int current = -1;
    for(size_t i = 0; i < m_itemsData.size(); ++i) {
        if(m_itemsData[i].index == m_currentIndex) {
            current = i;
            break;
        }
    }
    if(current < 0) {
        current = 0;
        selectItem(m_itemsData[current].index);
        m_currentIndex = m_itemsData[current].index;
        updateDelegatesStates();
        repaint();
        return true;
    }

    int next = current;
    bool handled = true;

    switch(event->keyCode()) {
    case Input::KEY_UP: {
        if(current > 0) {
            next = current - 1;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_DOWN: {
        if(current < static_cast<int>(m_itemsData.size()) - 1) {
            next = current + 1;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_HOME: {
        if(!m_itemsData.empty()) {
            next = 0;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_END: {
        if(!m_itemsData.empty()) {
            next = m_itemsData.size() - 1;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_RIGHT: {
        if(m_model->rowCount(m_itemsData[current].index) > 0) {
            setExpanded(m_itemsData[current].index, true);

            ModelIndex child = m_model->index(0, 0, m_itemsData[current].index);
            if(child.isValid()) {
                for(size_t i = 0; i < m_itemsData.size(); ++i) {
                    if(m_itemsData[i].index == child) {
                        next = i;
                        break;
                    }
                }
            }
            scrollToItem(next);
        } else {
            if(current < static_cast<int>(m_itemsData.size()) - 1) {
                next = current + 1;
                scrollToItem(next);
            } else {
                handled = false;
            }
        }
    } break;

    case Input::KEY_LEFT: {
        if(isExpanded(m_itemsData[current].index)) {
            setExpanded(m_itemsData[current].index, false);
            next = current;
            scrollToItem(next);
        } else {
            ModelIndex parent = m_model->parent(m_itemsData[current].index);
            if(parent.isValid()) {
                for(size_t i = 0; i < m_itemsData.size(); ++i) {
                    if(m_itemsData[i].index == parent) {
                        next = i;
                        break;
                    }
                }
                scrollToItem(next);
            } else {
                handled = false;
            }
        }
    } break;

    case Input::KEY_ENTER: {
        if(m_currentIndex.isValid()) {
            activateCurrentItem();
            if(m_model->rowCount(m_currentIndex) > 0) {
                toggleExpanded(current);
            }
        }
        return true;
    }
    case Input::KEY_F2: {
        return edit(m_currentIndex);
    }

    default: {
        handled = false;
    } break;
    }

    if(handled && next >= 0 && next < static_cast<int>(m_itemsData.size())) {
        bool ctrlPressed = event->isControlPressed();
        bool shiftPressed = event->isShiftPressed();

        if(shiftPressed && m_currentIndex.isValid()) {
            selectRange(m_currentIndex, m_itemsData[next].index);
        } else if(ctrlPressed) {
            toggleSelection(m_itemsData[next].index);
        } else {
            selectItem(m_itemsData[next].index);
        }

        m_currentIndex = m_itemsData[next].index;
        updateDelegatesStates();
        repaint();
        return true;
    }

    return handled;
}

void TreeView::scrollToItem(int row) {
    if(!m_vScroll || row < 0 || row >= static_cast<int>(m_itemsData.size())) {
        return;
    }

    int top = row * m_rowHeight;
    int bottom = top + m_rowHeight;
    int value = m_vScroll->value();
    int height = static_cast<int>(viewportSize().y);

    if(top < value) {
        value = top;
    } else if(bottom > value + height) {
        value = bottom - height;
    }

    value = std::max(m_vScroll->minimum(), std::min(m_vScroll->maximum(), value));
    m_vScroll->setValue(value);
}

void TreeView::update(const Vector2 &pos) {
    AbstractItemView::update(pos);
    if(m_isPressed && Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        m_isPressed = false;
        Widget::cancelDragCandidate(this);
    }
    if(m_dirtyItems) {
        rebuildItems();
        m_dirtyItems = false;
    }
}

void TreeView::boundChanged(const Vector2 &size) {
    AbstractItemView::boundChanged(size);
    m_dirtyItems = true;
}

void TreeView::onVScrollChanged(int value) {
    AbstractItemView::onVScrollChanged(value);

    if(m_vScroll && m_rowHeight > 0) {
        int oldFirstVisible = m_firstVisibleIndex;
        m_firstVisibleIndex = value / m_rowHeight;

        int maxIndex = std::max(0, static_cast<int>(m_itemsData.size()) - 1);
        m_firstVisibleIndex = std::min(m_firstVisibleIndex, maxIndex);

        if(oldFirstVisible != m_firstVisibleIndex) {
            m_dirtyItems = true;
            repaint();
        }
    }
}

Vector2 TreeView::cellSize() const {
    return Vector2(m_rowHeight);
}

void TreeView::activateCurrentItem() {
    if(m_currentIndex.isValid()) {
        AbstractItemView::activateCurrentItem();
    }
}
