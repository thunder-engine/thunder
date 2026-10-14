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
        m_icon = findChild<Image *>();
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

                Widget *rect = m_icon;
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
        m_label = findChild<Label *>();
        onHierarchyUpdated();
    }
    if(m_label) {
        m_label->setText(m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DisplayRole).toString());

        float textWidth = m_label->textWidth();
        float contentOffset = showIcon ? iconSize.x + 4.0f : 2.0f;
        float totalWidth = std::max(contentOffset + textWidth + 2.0f, std::max(cellSize.x, 1.0f));

        Widget *rect = m_label;
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

        Widget *delegateRect = this;
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

    Widget *iconActor = Widget::createWidget<Image>("icon", this);
    m_icon = dynamic_cast<Image *>(iconActor);
    if(m_icon) {
        Widget *rect = m_icon;
        if(rect) {
            rect->setAnchors(Vector2(0, 0.5f), Vector2(0, 0.5f));
            rect->setPivot(Vector2(0, 0.5f));
            rect->setPosition(Vector3(2, 0, 0));
        }
    }

    Widget *labelActor = Widget::createWidget<Label>("label", this);
    m_label = dynamic_cast<Label *>(labelActor);
    if(m_label) {
        Widget *rect = m_label;
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
