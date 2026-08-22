#include "components/itemviewdelegate.h"

#include "image.h"
#include "label.h"
#include "listview.h"
#include "recttransform.h"

#include <abstractitemmodel.h>
#include <algorithm>

ItemViewDelegate::ItemViewDelegate() :
        m_icon(nullptr),
        m_label(nullptr),
        m_selected(false),
        m_hovered(false) {
}

ItemViewDelegate::~ItemViewDelegate() {

}

void ItemViewDelegate::bind(AbstractItemView *view, const ModelIndex &index) {
    m_modelIndex = index;

    updateData(view);
}

int ItemViewDelegate::index() const {
    return m_modelIndex.row();
}

const ModelIndex &ItemViewDelegate::modelIndex() const {
    return m_modelIndex;
}

void ItemViewDelegate::setSelected(bool selected) {
    if(m_selected != selected) {
        m_selected = selected;
        updateStyle();
    }
}

void ItemViewDelegate::setHovered(bool hovered) {
    if(m_hovered != hovered) {
        m_hovered = hovered;
        updateStyle();
    }
}

void ItemViewDelegate::updateStyle() {
    if(m_selected) {
        setBackgroundColor(Vector4(0.01f, 0.6f, 0.9f, 0.8f));
    } else if(m_hovered) {
        setBackgroundColor(Vector4(0.0f, 0.0f, 0.0f, 0.25f));
    } else {
        setBackgroundColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

void ItemViewDelegate::updateData(AbstractItemView *view) {
    if(!m_modelIndex.isValid()) {
        return;
    }

    bool showIcon = false;
    if(m_icon == nullptr) {
        m_icon = actor()->findChild<Image *>();
    }

    Vector2 cellSize;
    if(view) {
        cellSize = view->cellSize();
    }
    bool iconMode = view && view->iconPosition() == AbstractItemView::TextUnderIcon;
    Vector2 iconSize(cellSize.x > 0.0f ? cellSize.x : 32.0f);

    if(m_icon) {
        Variant value = m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DecorationRole);
        if(value.isValid()) {
            Sprite *icon = value.value<Sprite *>();
            if(icon) {
                showIcon = true;
                m_icon->setSprite(icon);
                m_icon->setEnabled(showIcon);

                RectTransform *rect = m_icon->rectTransform();
                if(rect) {
                    if(iconMode) {
                        rect->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
                        rect->setPivot(Vector2(0.5f, 1.0f));
                        rect->setPosition(Vector3(0.0f, -2.0f, 0.0f));
                    } else {
                        rect->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
                        rect->setPivot(Vector2(0.0f, 0.5f));
                        rect->setPosition(Vector3(2.0f, 0.0f, 0.0f));
                    }
                    rect->setSize(iconSize);
                }
            }
        }
    }

    if(m_icon) {
        m_icon->setEnabled(showIcon);
    }

    if(m_label == nullptr) {
        m_label = actor()->findChild<Label *>();
        onHierarchyUpdated();
    }
    if(m_label) {
        m_label->setText(m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DisplayRole).toString());

        float textWidth = m_label->textWidth();
        float contentOffset = showIcon ? iconSize.x + 4.0f : 2.0f;
        float totalWidth = std::max(contentOffset + textWidth + 2.0f, std::max(cellSize.x, 1.0f));

        RectTransform *rect = m_label->rectTransform();
        if(rect) {
            if(iconMode) {
                rect->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
                rect->setPivot(Vector2(0.5f, 1.0f));
                rect->setPosition(Vector3(0.0f, -(cellSize.y + 4.0f), 0.0f));
                rect->setSize(Vector2(std::min(textWidth, std::max(cellSize.x, 1.0f)), 16.0f));
            } else {
                rect->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
                rect->setPivot(Vector2(0.0f, 0.5f));
                rect->setPosition(Vector3(contentOffset, 0, 0));
                rect->setSize(Vector2(totalWidth, std::max(cellSize.y, 1.0f)));
            }
        }

        RectTransform *delegateRect = rectTransform();
        if(delegateRect) {
            if(iconMode) {
                delegateRect->setSize(Vector2(std::max(cellSize.x, 1.0f), std::max(cellSize.y, 1.0f)));
            } else {
                delegateRect->setSize(Vector2(totalWidth, std::max(cellSize.y, 1.0f)));
            }
        }
    }
}

void ItemViewDelegate::composeComponent() {
    Widget::composeComponent();

    Actor *iconActor = Engine::composeActor<Image>("icon", actor());
    m_icon = iconActor->getComponent<Image>();
    if(m_icon) {
        RectTransform *rect = m_icon->rectTransform();
        if(rect) {
            rect->setAnchors(Vector2(0, 0.5f), Vector2(0, 0.5f));
            rect->setPivot(Vector2(0, 0.5f));
            rect->setPosition(Vector3(2, 0, 0));
        }
    }

    Actor *labelActor = Engine::composeActor<Label>("label", actor());
    m_label = labelActor->getComponent<Label>();
    if(m_label) {
        RectTransform *rect = m_label->rectTransform();
        if(rect) {
            rect->setAnchors(Vector2(0, 0.5f), Vector2(0, 0.5f));
            rect->setPivot(Vector2(0, 0.5f));
        }
        m_label->setAlign(Alignment::Left | Alignment::Middle);
        m_label->setWordWrap(false);
        m_label->setClip(true);
    }

    updateStyle();
}
