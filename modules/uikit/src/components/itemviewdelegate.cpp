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
#include "components/itemviewdelegate.h"

#include "image.h"
#include "label.h"
#include "listview.h"
#include "abstractitemview.h"

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

bool ItemViewDelegate::beginEditing(AbstractItemView *view) {
    if(!view || !m_label || !m_modelIndex.isValid() ||
       !(m_modelIndex.model()->flags(m_modelIndex) & AbstractItemModel::ItemIsEditable)) {
        return false;
    }
    return view->beginEditing(m_modelIndex, m_label);
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
        m_icon = findChild<Image *>();
    }

    Vector2 cellSize;
    if(view) {
        cellSize = view->cellSize();
    }
    ListView *listView = dynamic_cast<ListView *>(view);
    bool iconMode = view && (view->iconPosition() == AbstractItemView::TextUnderIcon ||
                             (listView && listView->viewMode() == ListView::IconMode));
    float iconExtent = cellSize.x > 0.0f ? cellSize.x : 32.0f;
    if(iconMode) {
        iconExtent = std::max(1.0f, std::min(cellSize.x - 8.0f, cellSize.y - 28.0f));
    }
    Vector2 iconSize(iconExtent);

    if(m_icon) {
        Variant value = m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DecorationRole);
        if(value.isValid()) {
            Sprite *icon = value.value<Sprite *>();
            if(icon) {
                showIcon = true;
                m_icon->setSprite(icon);
                m_icon->setEnabled(showIcon);

                if(iconMode) {
                    m_icon->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
                    m_icon->setPivot(Vector2(0.5f, 1.0f));
                    m_icon->setPosition(Vector3(0.0f, -2.0f, 0.0f));
                } else {
                    m_icon->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
                    m_icon->setPivot(Vector2(0.0f, 0.5f));
                    m_icon->setPosition(Vector3(2.0f, 0.0f, 0.0f));
                }
                m_icon->setSize(iconSize);
            }
        }
    }

    if(m_icon) {
        m_icon->setEnabled(showIcon);
    }

    if(m_label == nullptr) {
        m_label = findChild<Label *>();
        onHierarchyUpdated();
    }
    if(m_label) {
        m_label->setVisible(true);
        m_label->setText(m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DisplayRole).toString());
        if(iconMode) {
            m_label->setAlign(Alignment::Center | Alignment::Bottom);
        } else {
            m_label->setAlign(Alignment::Left | Alignment::Middle);
        }

        float textWidth = m_label->textWidth();
        float contentOffset = showIcon ? iconSize.x + 4.0f : 2.0f;
        float totalWidth = std::max(contentOffset + textWidth + 2.0f, std::max(cellSize.x, 1.0f));

        if(iconMode) {
            m_label->setAnchors(Vector2(0.5f, 1.0f), Vector2(0.5f, 1.0f));
            m_label->setPivot(Vector2(0.5f, 1.0f));
            m_label->setPosition(Vector3(0.0f, -(iconSize.y + 4.0f), 0.0f));
            m_label->setSize(Vector2(std::max(cellSize.x - 4.0f, 1.0f), 16.0f));
        } else {
            m_label->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
            m_label->setPivot(Vector2(0.0f, 0.5f));
            m_label->setPosition(Vector3(contentOffset, 0, 0));
            m_label->setSize(Vector2(totalWidth, std::max(cellSize.y, 1.0f)));
        }

        if(iconMode) {
            setSize(Vector2(std::max(cellSize.x, 1.0f), std::max(cellSize.y, 1.0f)));
        } else {
            setSize(Vector2(totalWidth, std::max(cellSize.y, 1.0f)));
        }
    }
}

void ItemViewDelegate::composeComponent() {
    Widget::composeComponent();

    m_icon = Widget::createWidget<Image>("icon", this);
    if(m_icon) {
        m_icon->setAnchors(Vector2(0, 0.5f), Vector2(0, 0.5f));
        m_icon->setPivot(Vector2(0, 0.5f));
        m_icon->setPosition(Vector3(2, 0, 0));
    }

    m_label = Widget::createWidget<Label>("label", this);
    if(m_label) {
        m_label->setAnchors(Vector2(0, 0.5f), Vector2(0, 0.5f));
        m_label->setPivot(Vector2(0, 0.5f));

        m_label->setAlign(Alignment::Left | Alignment::Middle);
        m_label->setWordWrap(false);
        m_label->setClip(true);
    }

    updateStyle();
}
