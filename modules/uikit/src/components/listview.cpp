#include "components/listview.h"

#include "components/recttransform.h"
#include "components/scrollbar.h"
#include "components/frame.h"
#include "components/listviewdelegate.h"

#include <abstractitemmodel.h>
#include <algorithm>
#include <input.h>

ListView::ListView() :
        m_gridSize(80, 100),
        m_highlightFrame(nullptr),
        m_delegate(nullptr),
        m_rowHeight(20),
        m_viewMode(ListMode),
        m_highlightIndex(-1),
        m_firstVisibleIndex(0),
        m_cachedColumns(1),
        m_cachedRows(1),
        m_selectedIndex(-1),
        m_isPressed(false) {

}

ListView::~ListView() {

}

void ListView::setModel(AbstractItemModel *model) {
    AbstractItemView::setModel(model);
    m_selectedIndex = -1;
    m_dirtyItems = true;
}

int ListView::viewMode() const {
    return m_viewMode;
}

void ListView::setViewMode(int mode) {
    if (m_viewMode == mode) {
        return;
    }

    m_viewMode = mode;
    m_dirtyItems = true;
    calculateGridParams();
    updateHighlight(m_selectedIndex >= 0 ? m_selectedIndex : m_highlightIndex);
    repaint();
}

int ListView::rowHeight() const {
    return m_rowHeight;
}

void ListView::setRowHeight(int height) {
    m_rowHeight = height;
    if(m_vScroll) {
        m_vScroll->setSingleStep((m_viewMode == ListMode) ? m_rowHeight : m_gridSize.y);
    }
    if(m_hScroll) {
        m_hScroll->setSingleStep((m_viewMode == ListMode) ? m_rowHeight : m_gridSize.x);
    }
    m_dirtyItems = true;
}

Vector2 ListView::gridSize() const {
    return m_gridSize;
}

void ListView::setGridSize(const Vector2 &size) {
    m_gridSize = size;
    if(m_vScroll) {
        m_vScroll->setSingleStep((m_viewMode == ListMode) ? m_rowHeight : m_gridSize.y);
    }
    if(m_hScroll) {
        m_hScroll->setSingleStep((m_viewMode == ListMode) ? m_rowHeight : m_gridSize.x);
    }
    m_dirtyItems = true;
}

ListViewDelegate *ListView::delegate() const {
    return m_delegate;
}

void ListView::setDelegate(ListViewDelegate *delegate) {
    if(m_delegate) {
        delete m_delegate;
    }

    m_delegate = delegate;
    m_dirtyItems = true;
}

void ListView::composeComponent() {
    AbstractItemView::composeComponent();

    Actor *widgetActor = Engine::composeActor<Widget>("content", actor());
    setContent(widgetActor->getComponent<Widget>());

    Actor *hlActor = Engine::composeActor<Frame>("highlight", m_content->actor());
    m_highlightFrame = hlActor->getComponent<Frame>();
    if(m_highlightFrame) {
        RectTransform *rect = m_highlightFrame->rectTransform();
        if(rect) {
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
            rect->setPivot(Vector2(0.0f, 1.0f));
            rect->setSize(Vector2(0.0f, m_rowHeight));
        }
        m_highlightFrame->setBackgroundColor(Vector4(0.0f, 0.0f, 0.0f, 0.5f));
        m_highlightFrame->setEnabled(false);
        setSubWidget(m_highlightFrame);
    }

    Actor *delegateActor = Engine::composeActor<ListViewDelegate>("delegate");
    ListViewDelegate *delegate = delegateActor->getComponent<ListViewDelegate>();
    if(delegate) {
        setDelegate(delegate);
    }

    setRowHeight(m_rowHeight);
}

bool ListView::onMouseDown(int x, int y) {
    if(!m_model || m_model->rowCount() == 0) {
        return false;
    }

    int index = indexAtPosition(Vector2(x, y));
    if(index >= 0 && index < m_model->rowCount()) {
        m_isPressed = true;
        handleItemClick(index);
        return true;
    }

    return false;
}

bool ListView::onMouseUp(int x, int y) {
    if(!m_isPressed) {
        return false;
    }

    m_isPressed = false;

    int index = indexAtPosition(Vector2(x, y));
    if(index >= 0 && index < m_model->rowCount()) {
        ModelIndex idx = m_model->index(index, 0);
        if(idx.isValid()) {
            clicked(idx);
            return true;
        }
    }

    return false;
}

bool ListView::onMouseMove(int x, int y) {
    int newHighlight = indexAtPosition(Vector2(x, y));
    if(newHighlight != m_highlightIndex) {
        updateHighlight(newHighlight);
        repaint();
    }

    return true;
}

bool ListView::onMouseDoubleClick(int x, int y) {
    if(!m_model || m_model->rowCount() == 0) {
        return false;
    }

    int index = indexAtPosition(Vector2(x, y));
    if(index >= 0 && index < m_model->rowCount()) {
        handleItemDoubleClick(index);
        return true;
    }

    return false;
}

bool ListView::onMouseWheel(int delta, bool horizontal) {
    ScrollBar *vbar = verticalScrollBar();
    if(!vbar) {
        return false;
    }

    if(m_viewMode == IconMode && horizontal) {
        ScrollBar *hbar = horizontalScrollBar();
        if(hbar) {
            int newValue = hbar->value() - delta;
            hbar->setValue(std::max(hbar->minimum(), std::min(hbar->maximum(), newValue)));
            return true;
        }
        return false;
    }

    int step = (m_viewMode == ListMode) ? m_rowHeight : m_gridSize.y;
    int newValue = vbar->value() - static_cast<int>(delta * step);
    vbar->setValue(std::max(vbar->minimum(), std::min(vbar->maximum(), newValue)));
    return true;
}

bool ListView::onKeyPress(KeyEvent *event) {
    if(!m_model || m_model->rowCount() == 0) {
        return false;
    }

    int totalItems = m_model->rowCount();
    int columns = (m_viewMode == IconMode) ? m_cachedColumns : 1;
    int current = m_selectedIndex;
    if (current < 0 && totalItems > 0) {
        current = 0;
    }
    if (current < 0 || current >= totalItems) {
        return false;
    }

    int newIndex = -1;
    bool needScroll = false;

    switch(event->keyCode()) {
        case Input::KEY_UP: {
            if (current >= columns) {
                newIndex = current - columns;
                needScroll = true;
            }
        } break;
        case Input::KEY_DOWN: {
            if(current + columns < totalItems) {
                newIndex = current + columns;
                needScroll = true;
            }
        } break;
        case Input::KEY_LEFT: {
            if(current > 0 && (current % columns) > 0) {
                newIndex = current - 1;
                needScroll = true;
            }
        } break;
        case Input::KEY_RIGHT: {
            if(current < totalItems - 1 && (current % columns) < columns - 1) {
                newIndex = current + 1;
                needScroll = true;
            }
        } break;
        case Input::KEY_HOME: {
            if(totalItems > 0) {
                newIndex = 0;
                needScroll = true;
            }
        } break;
        case Input::KEY_END: {
            if(totalItems > 0) {
                newIndex = totalItems - 1;
                needScroll = true;
            }
        } break;
        case Input::KEY_PAGE_UP: {
            int step = (m_viewMode == ListMode) ? m_rowHeight : m_gridSize.y;
            ScrollBar* vbar = verticalScrollBar();
            if(vbar) {
                int newValue = vbar->value() - step * columns;
                vbar->setValue(std::max(vbar->minimum(), std::min(vbar->maximum(), newValue)));
                return true;
            }
        } break;
        case Input::KEY_PAGE_DOWN: {
            int step = (m_viewMode == ListMode) ? m_rowHeight : m_gridSize.y;
            ScrollBar* vbar = verticalScrollBar();
            if(vbar) {
                int newValue = vbar->value() + step * columns;
                vbar->setValue(std::max(vbar->minimum(), std::min(vbar->maximum(), newValue)));
                return true;
            }
            break;
        }
        case Input::KEY_ENTER: {
            activateCurrentItem();
            return true;
        }
        case Input::KEY_ESCAPE: {
            m_selectedIndex = -1;
            m_highlightIndex = -1;
            updateHighlight(m_highlightIndex);
            repaint();
            return true;
        }
        default: break;
    }

    if(newIndex >= 0 && newIndex < totalItems) {
        m_selectedIndex = newIndex;
        m_highlightIndex = newIndex;
        updateHighlight(m_highlightIndex);
        repaint();

        ModelIndex idx = m_model->index(newIndex, 0);
        if(idx.isValid()) {
            selectItem(idx);
        }

        if(needScroll) {
            scrollToItem(newIndex);
        }

        return true;
    }

    return false;
}

void ListView::update(const Vector2 &pos) {
    AbstractItemView::update(pos);

    if(m_dirtyItems) {
        rebuildItems();
        m_dirtyItems = false;
    }

    if(isHovered(pos)) {
        float delta = Input::mouseScrollDelta();
        ScrollBar *vbar = verticalScrollBar();
        if(delta != 0.0f && vbar) {
            int step = (m_viewMode == ListMode) ? m_rowHeight : (m_gridSize.y > 0 ? static_cast<int>(m_gridSize.y) : m_rowHeight);
            int nv = vbar->value() - static_cast<int>(delta * step);
            nv = MAX(vbar->minimum(), MIN(vbar->maximum(), nv));
            vbar->setValue(nv);
        }
    }
}

Vector2 ListView::viewportSize() {
    return rectTransform()->size();
}

void ListView::calculateGridParams() {
    if(m_viewMode != IconMode || !m_model) {
        return;
    }

    if(m_gridSize.x <= 0 || m_gridSize.y <= 0) {
        m_cachedColumns = 1;
        m_cachedRows = 1;
        return;
    }

    int totalItems = m_model->rowCount();
    if(totalItems <= 0) {
        m_cachedColumns = 1;
        m_cachedRows = 1;
        return;
    }

    Vector2 viewportSize(ListView::viewportSize());
    m_cachedColumns = std::max(1, static_cast<int>(viewportSize.x / m_gridSize.x));
    m_cachedColumns = std::max(1, std::min(m_cachedColumns, totalItems));
    m_cachedRows = std::max(1, (totalItems + m_cachedColumns - 1) / m_cachedColumns);
}

void ListView::boundChanged(const Vector2 &size) {
    AbstractItemView::boundChanged(size);

    m_dirtyItems = true;
}

void ListView::onVScrollChanged(int value) {
    AbstractItemView::onVScrollChanged(value);

    RectTransform *rect = m_content->rectTransform();
    if(rect) {
        Vector3 pos(rect->position());
        int visibleIndex = m_firstVisibleIndex;

        if(m_viewMode == ListMode) {
            int visibleStep = m_rowHeight > 0 ? m_rowHeight : 1;
            visibleIndex = int(pos.y / visibleStep);
        } else {
            calculateGridParams();
            if(m_cachedColumns > 0) {
                float scrollOffset = pos.y;
                int row = static_cast<int>(scrollOffset / m_gridSize.y);
                visibleIndex = std::max(0, row * m_cachedColumns);
            }
        }

        if(visibleIndex != m_firstVisibleIndex) {
            if(m_firstVisibleIndex > visibleIndex) {
                m_items.splice(m_items.begin(), m_items, std::prev(m_items.end()));
            } else {
                m_items.splice(m_items.end(), m_items, m_items.begin());
            }
            m_firstVisibleIndex = visibleIndex;
            m_dirtyItems = true;
        }
    }
}

void ListView::activateCurrentItem() {
    if(m_selectedIndex < 0 || m_selectedIndex >= m_model->rowCount()) {
        return;
    }

    ModelIndex idx = m_model->index(m_selectedIndex, 0);
    if(idx.isValid()) {
        AbstractItemView::activateCurrentItem();
    }
}

void ListView::rebuildItems() {
    if(m_model && m_content) {
        int totalItemCount = 0;
        Vector2 viewportSize(ListView::viewportSize());
        if(m_viewMode == ListMode) {
            totalItemCount = viewportSize.y / m_rowHeight + 1;
        } else {
            totalItemCount = viewportSize.x / m_gridSize.x;
            totalItemCount *= int(viewportSize.y / m_gridSize.y) + 1;
        }

        if(totalItemCount > m_items.size()) {
            m_items.resize(totalItemCount);
        }

        for(auto it = m_items.begin(); it != m_items.end(); ++it) {
            if(*it) {
                (*it)->setEnabled(false);
            }
        }

        float maxWidth = 0;
        int rowCount = MIN(m_model->rowCount(), m_firstVisibleIndex + totalItemCount);
        auto it = m_items.begin();
        for(int i = m_firstVisibleIndex; i < rowCount; ++i) {
            ListViewDelegate *delegate = *it;
            if(delegate == nullptr && m_delegate) {
                Actor *itemActor = static_cast<Actor *>(m_delegate->actor()->clone(m_content->actor()));
                itemActor->setName(itemActor->name() + TString::number(i));
                delegate = itemActor->getComponent<ListViewDelegate>();
                RectTransform *rect = delegate->rectTransform();
                rect->setPivot(Vector2(0.0f, 1.0f));
                if(m_viewMode == ListMode) {
                    rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
                    rect->setSize(Vector2(0.0f, m_rowHeight));
                } else {
                    rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
                    rect->setSize(m_gridSize);
                }
                rect->setParentTransform(m_content->rectTransform(), true); // required to update child widgets

                *it = delegate;
                m_content->setSubWidget(delegate);
            }

            if(delegate) {
                delegate->setEnabled(true);
                delegate->bind(this, m_model->index(i, 0));

                RectTransform *rect = delegate->rectTransform();
                rect->setPosition(Vector3(positionAtIndex(i), 0.0f));

                maxWidth = MAX(maxWidth, rect->size().x);
            }

            ++it;
        }

        RectTransform *contentRect = m_content->rectTransform();
        if(contentRect) {
            float totalHeight;
            if(m_viewMode == ListMode) {
                totalHeight = m_model->rowCount() * m_rowHeight;
            } else {
                calculateGridParams();
                totalHeight = m_cachedRows * m_gridSize.y;
            }
            contentRect->setSize(Vector2(std::max(maxWidth, viewportSize.x), totalHeight));
        }
        updateHighlight(m_highlightIndex);

        updateScrollRange();
    }
}

void ListView::updateHighlight(int index) {
    if(!m_highlightFrame || !m_model) {
        return;
    }

    m_highlightIndex = index;

    if(index >= 0 && index < m_model->rowCount()) {
        RectTransform *rect = m_highlightFrame->rectTransform();
        if(rect) {
            rect->setPosition(Vector3(positionAtIndex(index), 0.0f));

            if (m_viewMode == ListMode) {
                rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
                rect->setPivot(Vector2(0.0f, 1.0f));
                rect->setSize(Vector2(viewportSize().x, static_cast<float>(m_rowHeight)));
            } else {
                rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
                rect->setPivot(Vector2(0.0f, 1.0f));
                rect->setSize(m_gridSize);
            }
        }
        m_highlightFrame->setEnabled(true);
    } else {
        m_highlightFrame->setEnabled(false);
    }
}

void ListView::handleItemDoubleClick(int index) {
    if(index < 0 || index >= m_model->rowCount()) {
        return;
    }

    ModelIndex idx = m_model->index(index, 0);
    if(!idx.isValid()) {
        return;
    }

    handleItemClick(index);
    activateCurrentItem();
}

void ListView::handleItemClick(int index) {
    if(index < 0 || index >= m_model->rowCount()) {
        return;
    }

    ModelIndex idx = m_model->index(index, 0);
    if(!idx.isValid()) {
        return;
    }

    m_selectedIndex = index;
    m_highlightIndex = index;
    updateHighlight(m_highlightIndex);
    repaint();

    selectItem(idx);
    pressed(idx);
}

int ListView::indexAtPosition(const Vector2 &pos) {
    if(!m_model || m_model->rowCount() == 0) {
        return -1;
    }

    Vector2 localPos(pos);
    if(m_content) {
        RectTransform *contentRect = m_content->rectTransform();
        localPos += contentRect->position();
    }

    if(localPos.x < 0 || localPos.y < 0) {
        return -1;
    }

    int index = -1;
    if(m_viewMode == ListMode) {
        index = static_cast<int>(localPos.y / m_rowHeight);
    } else {
        calculateGridParams();

        int col = static_cast<int>(localPos.x / m_gridSize.x);
        int row = static_cast<int>(localPos.y / m_gridSize.y);

        if(col >= 0 && col < m_cachedColumns && row >= 0) {
            index = row * m_cachedColumns + col;
        }
    }

    if(index >= m_firstVisibleIndex && index < m_model->rowCount()) {
        return index;
    }

    return -1;
}

Vector2 ListView::positionAtIndex(int index) {
    if(m_viewMode == ListMode) {
        return Vector2(0.0f, index * -m_rowHeight);
    } else {
        calculateGridParams();

        int row = index / m_cachedColumns;
        int col = index % m_cachedColumns;

        return Vector2(col * m_gridSize.x, row * -m_gridSize.y);
    }
}

void ListView::scrollToItem(int index) {
    if(index < 0 || index >= m_model->rowCount()) {
        return;
    }

    Vector2 pos = positionAtIndex(index);
    float targetY = -pos.y;

    ScrollBar *vbar = verticalScrollBar();
    if(!vbar) {
        return;
    }

    float viewportHeight = viewportSize().y;
    float itemHeight = (m_viewMode == ListMode) ? m_rowHeight : m_gridSize.y;
    float scrollPos = targetY - viewportHeight / 2 + itemHeight / 2;

    RectTransform *contentRect = m_content->rectTransform();
    if (contentRect) {
        float maxScroll = contentRect->size().y - viewportHeight;
        scrollPos = std::max(0.0f, std::min(scrollPos, maxScroll));
    }

    vbar->setValue(static_cast<int>(scrollPos));
}
