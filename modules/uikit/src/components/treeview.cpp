#include "components/treeview.h"

#include "components/itemviewdelegate.h"
#include "components/recttransform.h"
#include "components/scrollbar.h"
#include "components/canvas.h"

#include <algorithm>
#include <string.h>

#include <input.h>
#include <resources/material.h>
#include <resources/mesh.h>

#include <abstractitemmodel.h>
#include <pipelinecontext.h>
#include <commandbuffer.h>

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
        delete m_arrowMaterial;
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
    Actor *widgetActor = Engine::composeActor<Widget>("content", actor());
    setContent(widgetActor->getComponent<Widget>());

    Actor *delegateActor = Engine::composeActor<ItemViewDelegate>("delegate");
    setDelegate(delegateActor->getComponent<ItemViewDelegate>());

    setRowHeight(m_rowHeight);

    m_arrowSprite = Engine::loadResource<Sprite>(".embedded/ui.png/Arrow");
    if(m_arrowSprite) {
        m_arrowMesh = m_arrowSprite->mesh();

        Material *defaultMaterial = Engine::loadResource<Material>(".embedded/DefaultUI.shader");
        if(defaultMaterial) {
            m_arrowMaterial = defaultMaterial->createInstance();
            m_arrowMaterial->setTexture("mainTexture", m_arrowSprite->texture());
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
        rebuildItems();
        m_dirtyItems = false;
        repaint();
    }
}

void TreeView::clearDelegates() {
    for(auto &data : m_itemsData) {
        if(data.delegate && data.delegate->actor()) {
            data.delegate->actor()->deleteLater();
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
                delegate->setEnabled(false);
            }
        }
        RectTransform *contentRect = m_content->rectTransform();
        if(contentRect) {
            contentRect->setSize(Vector2(viewportSize().x, 0));
        }
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
            Actor *itemActor = static_cast<Actor *>(m_delegate->actor()->clone(m_content->actor()));
            delegate = itemActor->getComponent<ItemViewDelegate>();
            if(delegate) {
                m_content->setSubWidget(delegate);
            }
        }

        data.delegate = delegate;
        if(delegate) {
            delegate->setEnabled(true);
            delegate->bind(this, data.index);

            RectTransform *rect = delegate->rectTransform();
            if(rect) {
                rect->setAnchors(Vector2(0, 1), Vector2(0, 1));
                rect->setPivot(Vector2(0, 1));

                float leftPadding = data.depth * m_indentation + arrowWidth;
                rect->setSize(Vector2(viewportSize().x - leftPadding, m_rowHeight));
                rect->setPosition(Vector3(positionAtIndex(row) + Vector2(leftPadding, 0), 0));
            }
        }
    }

    for(int i = delegateIndex; i < static_cast<int>(existingDelegates.size()); ++i) {
        if(existingDelegates[i]) {
            existingDelegates[i]->setEnabled(false);
        }
    }

    RectTransform *contentRect = m_content->rectTransform();
    if(contentRect) {
        contentRect->setSize(Vector2(viewportSize().x, m_itemsData.size() * m_rowHeight));
    }

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

        RectTransform *rect = data.delegate->rectTransform();
        if(!rect) continue;

        Vector3 pos = rect->position();
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

        RectTransform *rect = itemData.delegate->rectTransform();
        if(!rect) continue;

        Matrix4 transform = rect->worldTransform();
        Vector2 scale = rect->worldScale();

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

    RectTransform *rect = data->delegate->rectTransform();
    if(!rect) {
        return Vector4();
    }

    Vector2 localPos = rect->position();
    Vector2 scale = rect->worldScale();

    return Vector4(localPos.x - m_rowHeight * scale.x, -localPos.y, m_rowHeight * scale.x, m_rowHeight * scale.y);
}

void TreeView::drawSub() {
    AbstractItemView::drawSub();

    buildArrowInstances();

    Canvas *canvas = TreeView::canvas();
    if(canvas && m_arrowMaterial && m_arrowMesh) {
        canvas->setClipRegion(rectTransform()->clipRegion());
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
        RectTransform *contentRect = m_content->rectTransform();
        if(contentRect) {
            local += contentRect->position();
        }
    }
    int row = static_cast<int>(local.y / m_rowHeight);
    return row >= 0 && row < static_cast<int>(m_itemsData.size()) ? row : -1;
}

Vector2 TreeView::positionAtIndex(int row) const {
    return Vector2(0, -row * m_rowHeight);
}

Vector2 TreeView::viewportSize() {
    RectTransform *rect = rectTransform();
    return rect ? rect->size() : Vector2();
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

}

bool TreeView::onMouseDown(int x, int y) {
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
    return true;
}

bool TreeView::onMouseUp(int x, int y) {
    if(!m_isPressed) {
        return false;
    }
    m_isPressed = false;
    int row = indexAtPosition(Vector2(x, y));
    if(row >= 0 && row < static_cast<int>(m_itemsData.size())) {
        clicked(m_itemsData[row].index);
        return true;
    }
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

bool TreeView::onMouseDoubleClick(int x, int y) {
    int row = indexAtPosition(Vector2(x, y));
    if(row < 0) {
        return false;
    }

    if(isClickOnIndicator(row, Vector2(x, y))) {
        toggleExpanded(row);
        return true;
    }

    ItemData *data = getItemData(row);
    if(data == nullptr) {
        return false;
    }
    handleItemClick(row);

    if(m_model->rowCount(data->index) == 0) {
        activateCurrentItem();
    }
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
            rebuildItems();
            m_dirtyItems = false;

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
            rebuildItems();
            m_dirtyItems = false;
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
