#include "components/treeview.h"

#include "components/itemviewdelegate.h"
#include "components/recttransform.h"
#include "components/scrollbar.h"
#include "components/canvas.h"

#include <abstractitemmodel.h>
#include <algorithm>
#include <input.h>
#include <resources/material.h>
#include <resources/mesh.h>
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

    m_visibleIndexes.clear();
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
        m_visibleIndexes.push_back(index);
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
    if(row < 0 || row >= static_cast<int>(m_visibleIndexes.size())) {
        return;
    }
    const ModelIndex index = m_visibleIndexes[row];
    if(m_model->rowCount(index) > 0) {
        setExpanded(index, !isExpanded(index));
        rebuildItems();
        m_dirtyItems = false;
        repaint();
    }
}

void TreeView::clearDelegates() {
    for(auto delegate : m_items) {
        if(delegate && delegate->actor()) {
            delegate->actor()->deleteLater();
        }
    }
    m_items.clear();
}

void TreeView::buildArrowInstances() {
    if(!m_arrowMaterial || !m_arrowMesh || !m_arrowSprite) {
        return;
    }

    RectTransform *contentRect = m_content->rectTransform();
    Canvas *canvas = TreeView::canvas();
    if(!canvas || !contentRect) {
        return;
    }

    Matrix4 world = contentRect->worldTransform();
    Vector3 contentPos = contentRect->position();
    contentPos.y += contentRect->size().y;

    //Vector4 bounds = m_arrowSprite->bounds();
    //float spriteWidth = bounds.z - bounds.x;
    //float spriteHeight = bounds.w - bounds.y;

    Vector4 color(1.0f);

    static Matrix4 rotation;
    if(rotation[4] != -1) {
        rotation[0] = 0.0f;
        rotation[1] = 1.0f;
        rotation[4] = -1.0f;
        rotation[5] = 0.0f;
    }

    size_t instanceSize = m_arrowMaterial->instanceSize();
    m_instanceBuffer.resize(instanceSize * m_items.size());

    uint8_t *data = m_instanceBuffer.data();
    int instanceIndex = 0;

    for(auto it : m_items) {
        const ModelIndex &index = it->modelIndex();
        if(m_model->rowCount(index) == 0) {
            continue;
        }

        //int depth = m_visibleDepths[i];
        //float arrowX = contentPos.x + depth * m_indentation + 4.0f + m_rowHeight * 0.5f;
        //float arrowY = contentRect->size().y - yPos + m_rowHeight * 0.5f;

        RectTransform *rect = it->rectTransform();

        Matrix4 transform(rect->worldTransform());
        //transform[0] = m_rowHeight / spriteWidth;
        //transform[5] = m_rowHeight / spriteHeight;
        //transform[12] += 0;
        //transform[13] -= it->rectTransform()->position().y;

        if(isExpanded(index)) {
            transform = transform * rotation;
        }

        memcpy(data + instanceIndex * instanceSize, &transform, sizeof(Matrix4));
        memcpy(data + instanceIndex * instanceSize + sizeof(Matrix4), &color, sizeof(Vector4));

        instanceIndex++;
    }
    m_instanceBuffer.resize(instanceSize * instanceIndex);

    m_arrowMaterial->setInstanceBuffer(&m_instanceBuffer);
}

void TreeView::drawSub() {
    AbstractItemView::drawSub();

    buildArrowInstances();

    Canvas *canvas = TreeView::canvas();
    if(canvas && m_arrowMaterial && m_arrowMesh) {
        canvas->drawMesh(m_arrowMesh, m_arrowMaterial);
    }
    if(m_arrowMaterial) {
        m_arrowMaterial->setInstanceBuffer(nullptr);
    }
}

Vector4 TreeView::getArrowRect(int row) const {
    if(row < 0 || row >= static_cast<int>(m_visibleIndexes.size())) {
        return Vector4(0, 0, 0, 0);
    }

    const ModelIndex &index = m_visibleIndexes[row];
    if(m_model->rowCount(index) == 0) {
        return Vector4(0, 0, 0, 0);
    }

    RectTransform *contentRect = m_content->rectTransform();
    if(!contentRect) {
        return Vector4(0, 0, 0, 0);
    }

    Vector3 contentPos = contentRect->worldPosition();
    float scrollOffset = m_vScroll ? -m_vScroll->value() : 0;

    int depth = 0;//m_visibleDepths[row];
    float yPos = row * m_rowHeight + scrollOffset;

    float arrowX = contentPos.x + depth * m_indentation + 4.0f;
    float arrowY = contentPos.y + yPos + (m_rowHeight - m_rowHeight) / 2.0f;

    return Vector4(arrowX, arrowY, m_rowHeight, m_rowHeight);
}

bool TreeView::isPointInRect(const Vector2 &point, const Vector4 &rect) const {
    return point.x >= rect.x && point.x <= rect.x + rect.z &&
           point.y >= rect.y && point.y <= rect.y + rect.w;
}

bool TreeView::isClickOnIndicator(int row, const Vector2 &pos) const {
    Vector4 arrowRect = getArrowRect(row);
    return isPointInRect(pos, arrowRect);
}

void TreeView::rebuildItems() {
    if(!m_model || !m_content) {
        return;
    }

    m_visibleIndexes.clear();
    appendVisible(m_rootIndex, 0);

    if(m_visibleIndexes.empty()) {
        for(auto item : m_items) {
            if(item) {
                item->setEnabled(false);
            }
        }
        RectTransform *contentRect = m_content->rectTransform();
        if(contentRect) {
            contentRect->setSize(Vector2(viewportSize().x, 0));
        }
        updateScrollRange();
        return;
    }

    m_firstVisibleIndex = std::min(m_firstVisibleIndex, std::max(0, static_cast<int>(m_visibleIndexes.size()) - 1));

    int visibleCount = std::max(1, static_cast<int>(viewportSize().y / m_rowHeight) + 2);

    while(static_cast<int>(m_items.size()) < visibleCount && m_delegate) {
        Actor *itemActor = static_cast<Actor *>(m_delegate->actor()->clone(m_content->actor()));
        ItemViewDelegate *delegate = itemActor->getComponent<ItemViewDelegate>();
        if(delegate) {
            m_items.push_back(delegate);
            m_content->setSubWidget(delegate);
        }
    }

    for(auto item : m_items) {
        if(item) {
            item->setEnabled(false);
        }
    }

    auto item = m_items.begin();
    int end = std::min(static_cast<int>(m_visibleIndexes.size()), m_firstVisibleIndex + visibleCount);
    for(int row = m_firstVisibleIndex; row < end && item != m_items.end(); ++row, ++item) {
        ItemViewDelegate *delegate = *item;
        if(delegate) {
            delegate->setEnabled(true);
            delegate->bind(this, m_visibleIndexes[row]);

            RectTransform *rect = delegate->rectTransform();
            if(rect) {
                rect->setAnchors(Vector2(0, 1), Vector2(0, 1));
                rect->setPivot(Vector2(0, 1));
                rect->setSize(Vector2(viewportSize().x, m_rowHeight));
                rect->setPosition(Vector3(positionAtIndex(row), 0));
            }
        }
    }

    RectTransform *contentRect = m_content->rectTransform();
    if(contentRect) {
        contentRect->setSize(Vector2(viewportSize().x, m_visibleIndexes.size() * m_rowHeight));
    }

    updateDelegatesStates();
    updateScrollRange();
    repaint();
}

void TreeView::updateDelegatesStates() {
    for(auto item : m_items) {
        if(item && item->isEnabled()) {
            const ModelIndex &idx = item->modelIndex();
            if(idx.isValid()) {
                item->setSelected(isIndexSelected(idx));
            }
        }
    }
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
    return row >= 0 && row < static_cast<int>(m_visibleIndexes.size()) ? row : -1;
}

Vector2 TreeView::positionAtIndex(int row) const {
    return Vector2(0, -row * m_rowHeight);
}

Vector2 TreeView::viewportSize() {
    RectTransform *rect = rectTransform();
    return rect ? rect->size() : Vector2();
}

void TreeView::handleItemClick(int row) {
    if(row < 0 || row >= static_cast<int>(m_visibleIndexes.size())) {
        return;
    }
    selectItem(m_visibleIndexes[row]);
    m_currentIndex = m_visibleIndexes[row];
    updateDelegatesStates();
    repaint();
    pressed(m_currentIndex);
}

void TreeView::handleItemDoubleClick(int row) {
    if(row < 0 || row >= static_cast<int>(m_visibleIndexes.size())) {
        return;
    }
    handleItemClick(row);
    toggleExpanded(row);
    if(m_model->rowCount(m_visibleIndexes[row]) == 0) {
        activateCurrentItem();
    }
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

    // Проверяем клик по стрелке
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
    if(row >= 0 && row < static_cast<int>(m_visibleIndexes.size())) {
        clicked(m_visibleIndexes[row]);
        return true;
    }
    return false;
}

bool TreeView::onMouseMove(int x, int y) {
    int row = indexAtPosition(Vector2(x, y));
    for(auto item : m_items) {
        if(item && item->isEnabled()) {
            const ModelIndex &idx = item->modelIndex();
            item->setHovered(row >= 0 && idx.isValid() && idx == m_visibleIndexes[row]);
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

    // Проверяем, не кликнули ли по стрелке
    if(isClickOnIndicator(row, Vector2(x, y))) {
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
    if(m_visibleIndexes.empty()) {
        return false;
    }

    int current = -1;
    for(size_t i = 0; i < m_visibleIndexes.size(); ++i) {
        if(m_visibleIndexes[i] == m_currentIndex) {
            current = i;
            break;
        }
    }
    if(current < 0) {
        current = 0;
        selectItem(m_visibleIndexes[current]);
        m_currentIndex = m_visibleIndexes[current];
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
        if(current < static_cast<int>(m_visibleIndexes.size()) - 1) {
            next = current + 1;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_HOME: {
        if(!m_visibleIndexes.empty()) {
            next = 0;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_END: {
        if(!m_visibleIndexes.empty()) {
            next = m_visibleIndexes.size() - 1;
            scrollToItem(next);
        } else {
            handled = false;
        }
    } break;

    case Input::KEY_RIGHT: {
        if(m_model->rowCount(m_visibleIndexes[current]) > 0) {
            setExpanded(m_visibleIndexes[current], true);
            rebuildItems();
            m_dirtyItems = false;

            ModelIndex child = m_model->index(0, 0, m_visibleIndexes[current]);
            if(child.isValid()) {
                for(size_t i = 0; i < m_visibleIndexes.size(); ++i) {
                    if(m_visibleIndexes[i] == child) {
                        next = i;
                        break;
                    }
                }
            }
            scrollToItem(next);
        } else {
            if(current < static_cast<int>(m_visibleIndexes.size()) - 1) {
                next = current + 1;
                scrollToItem(next);
            } else {
                handled = false;
            }
        }
    } break;

    case Input::KEY_LEFT: {
        if(isExpanded(m_visibleIndexes[current])) {
            setExpanded(m_visibleIndexes[current], false);
            rebuildItems();
            m_dirtyItems = false;
            next = current;
            scrollToItem(next);
        } else {
            ModelIndex parent = m_model->parent(m_visibleIndexes[current]);
            if(parent.isValid()) {
                for(size_t i = 0; i < m_visibleIndexes.size(); ++i) {
                    if(m_visibleIndexes[i] == parent) {
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

    if(handled && next >= 0 && next < static_cast<int>(m_visibleIndexes.size())) {
        bool ctrlPressed = event->isControlPressed();
        bool shiftPressed = event->isShiftPressed();

        if(shiftPressed && m_currentIndex.isValid()) {
            selectRange(m_currentIndex, m_visibleIndexes[next]);
        } else if(ctrlPressed) {
            toggleSelection(m_visibleIndexes[next]);
        } else {
            selectItem(m_visibleIndexes[next]);
        }

        m_currentIndex = m_visibleIndexes[next];
        updateDelegatesStates();
        repaint();
        return true;
    }

    return handled;
}

void TreeView::scrollToItem(int row) {
    if(!m_vScroll || row < 0 || row >= static_cast<int>(m_visibleIndexes.size())) {
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

        int maxIndex = std::max(0, static_cast<int>(m_visibleIndexes.size()) - 1);
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
