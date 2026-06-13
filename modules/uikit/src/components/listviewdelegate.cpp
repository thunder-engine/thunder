#include "components/listviewdelegate.h"

#include "image.h"
#include "label.h"
#include "listview.h"
#include "recttransform.h"

#include <abstractitemmodel.h>
#include <algorithm>

ListViewDelegate::ListViewDelegate() :
    m_icon(nullptr),
    m_label(nullptr) {
}

ListViewDelegate::~ListViewDelegate() {

}

void ListViewDelegate::bind(ListView *view, const ModelIndex &index) {
    m_modelIndex = index;

    updateData(view);
}

void ListViewDelegate::updateData(ListView *view) {
    if(!m_modelIndex.isValid()) {
        return;
    }

    bool showIcon = false;
    if(m_icon == nullptr) {
        m_icon = actor()->findChild<Image *>();
    }

    bool iconMode = view && view->viewMode() == ListView::IconMode;
    float cellWidth = iconMode ? (view ? view->gridSize().x : 0.0f) : (view ? static_cast<float>(view->rowHeight()) : 0.0f);
    float cellHeight = iconMode ? (view ? view->gridSize().y : 0.0f) : (view ? static_cast<float>(view->rowHeight()) : 0.0f);
    float iconHeight = iconMode ? (cellHeight > 0.0f ? std::min(cellHeight, 80.0f) : 80.0f) : (cellHeight > 0.0f ? cellHeight : 16.0f);
    Vector2 iconSize(cellWidth > 0.0f ? cellWidth : 32.0f);

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
    }
    if(m_label) {
        m_label->setText(m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DisplayRole).toString());

        float textWidth = m_label->textWidth();
        float labelHeight = std::max(16.0f, m_label->textWidth() > 0.0f ? 16.0f : 16.0f);
        float contentOffset = showIcon ? iconSize.x + 4.0f : 2.0f;
        float totalWidth = std::max(contentOffset + textWidth + 2.0f, std::max(cellWidth, 1.0f));

        RectTransform *rect = m_label->rectTransform();
        if(rect) {
            if(iconMode) {
                rect->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
                rect->setPivot(Vector2(0.5f, 1.0f));
                rect->setPosition(Vector3(0.0f, -(iconHeight + 4.0f), 0.0f));
                rect->setSize(Vector2(std::min(textWidth, std::max(cellWidth, 1.0f)), std::max(labelHeight, 16.0f)));
            } else {
                rect->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
                rect->setPivot(Vector2(0.0f, 0.5f));
                rect->setPosition(Vector3(contentOffset, 0, 0));
                rect->setSize(Vector2(totalWidth, std::max(cellHeight, 1.0f)));
            }
        }

        RectTransform *delegateRect = rectTransform();
        if(delegateRect) {
            if(iconMode) {
                delegateRect->setSize(Vector2(std::max(cellWidth, 1.0f), std::max(cellHeight, 1.0f)));
            } else {
                delegateRect->setSize(Vector2(totalWidth, std::max(cellHeight, 1.0f)));
            }
        }
    }
}

void ListViewDelegate::composeComponent() {
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
}
