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
#include "layout.h"

#include <abstractitemmodel.h>
#include <engine.h>
#include <resources/sprite.h>
#include <algorithm>

ItemViewDelegate::ItemViewDelegate() :
        m_view(nullptr),
        m_arrowIndent(nullptr),
        m_arrow(nullptr),
        m_icon(nullptr),
        m_label(nullptr),
        m_selected(false),
        m_hovered(false),
        m_arrowTabSize(0.0f) {
}

ItemViewDelegate::~ItemViewDelegate() {

}

void ItemViewDelegate::bind(AbstractItemView *view, const ModelIndex &index) {
    m_view = view;
    m_modelIndex = index;

    updateData(view);
}

bool ItemViewDelegate::beginEditing(AbstractItemView *view) {
    Label *label = labelWidget();
    if(!view || !label || !m_modelIndex.isValid() ||
       !(m_modelIndex.model()->flags(m_modelIndex) & AbstractItemModel::ItemIsEditable)) {
        return false;
    }
    return view->beginEditing(m_modelIndex, label);
}

int ItemViewDelegate::index() const {
    return m_modelIndex.row();
}

const ModelIndex &ItemViewDelegate::modelIndex() const {
    return m_modelIndex;
}

bool ItemViewDelegate::isArrowVisible() const {
    Image *arrow = arrowWidget();
    return arrow && arrow->isVisible();
}

void ItemViewDelegate::setArrowVisible(bool visible) {
    if(Image *arrow = arrowWidget()) {
        arrow->setVisible(visible);
    }
}

bool ItemViewDelegate::handleClick(const Vector2 &position, const Vector2 &viewOffset) {
    if(!m_view || !m_modelIndex.isValid()) {
        return false;
    }

    const float localX = position.x - viewOffset.x - this->position().x;
    for(auto it = m_clickAreas.rbegin(); it != m_clickAreas.rend(); ++it) {
        Widget *widget = it->widget;
        if(!widget || !widget->isVisible() || !widget->isEnabled()) {
            continue;
        }

        const float left = widget->position().x;
        const float right = left + widget->size().x;
        if(localX >= left && localX <= right) {
            m_view->onDelegateAreaClicked(m_modelIndex, it->name);
            return true;
        }
    }
    return false;
}

void ItemViewDelegate::setArrowTabSize(float size) {
    m_arrowTabSize = std::max(0.0f, size);
}

void ItemViewDelegate::setArrowExpanded(bool expanded) {
    if(Image *arrow = arrowWidget()) {
        arrow->setRotation(expanded ? 0.0f : 90.0f);
    }
}

Widget *ItemViewDelegate::arrowIndentWidget() const {
    return m_arrowIndent;
}

void ItemViewDelegate::setArrowIndentWidget(Widget *indent) {
    m_arrowIndent = indent;
}

Image *ItemViewDelegate::arrowWidget() const {
    return m_arrow;
}

void ItemViewDelegate::setArrowWidget(Image *arrow) {
    m_arrow = arrow;
}

Image *ItemViewDelegate::iconWidget() const {
    return m_icon;
}

void ItemViewDelegate::setIconWidget(Image *icon) {
    m_icon = icon;
}

Label *ItemViewDelegate::labelWidget() const {
    return m_label;
}

void ItemViewDelegate::setLabelWidget(Label *label) {
    m_label = label;
}

void ItemViewDelegate::addClickArea(const TString &name, Widget *widget) {
    if(!widget) {
        return;
    }
    for(ClickArea &area : m_clickAreas) {
        if(area.name == name) {
            area.widget = widget;
            return;
        }
    }
    m_clickAreas.push_back({name, widget});
}

void ItemViewDelegate::clearClickAreas() {
    m_clickAreas.clear();
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
    clearClickAreas();
    if(!m_modelIndex.isValid()) {
        return;
    }

    if(!layout()) {
        Layout *itemLayout = new Layout;
        itemLayout->setOrientation(Widget::Horizontal);
        setLayout(itemLayout);
    }

    const AbstractItemModel *model = m_modelIndex.model();
    setArrowVisible(model->rowCount(m_modelIndex) > 0);
    if(isArrowVisible()) {
        addClickArea("Expand", arrowWidget());
    }

    int depth = 0;
    ModelIndex parent = m_modelIndex.parent();
    ModelIndex root = view ? view->rootIndex() : ModelIndex();
    while(parent.isValid() && parent != root) {
        ++depth;
        parent = parent.parent();
    }

    Widget *arrowIndent = arrowIndentWidget();
    if(arrowIndent) {
        float indent = depth * m_arrowTabSize;
        if(isArrowVisible()) {
            Image *arrow = arrowWidget();
            if(arrow) {
                Vector4 margin = arrow->margin();
                indent -= arrow->size().x + margin.y + margin.w;
            }
        }
        arrowIndent->setSize(Vector2(std::max(0.0f, indent), view ? view->cellSize().y : 0.0f));
        arrowIndent->setVisible(depth > 0);
    }

    bool showIcon = false;

    Vector2 cellSize;
    if(view) {
        cellSize = view->cellSize();
    }
    ListView *listView = dynamic_cast<ListView *>(view);
    bool iconMode = view && (view->iconPosition() == AbstractItemView::TextUnderIcon ||
                             (listView && listView->viewMode() == ListView::IconMode));
    if(layout()) {
        layout()->setOrientation(iconMode ? Widget::Vertical : Widget::Horizontal);
        layout()->setSpacing(iconMode ? 4 : 0);
        layout()->setCrossAlignment(Layout::CrossCenter);
    }

    float iconExtent = cellSize.x > 0.0f ? cellSize.x : 32.0f;
    if(iconMode) {
        iconExtent = std::max(1.0f, std::min(cellSize.x - 8.0f, cellSize.y - 28.0f));
    } else {
        iconExtent -= 4.0f;
    }
    Vector2 iconSize(iconExtent);

    if(m_icon) {
        Variant value = m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DecorationRole);
        if(value.isValid()) {
            Sprite *icon = value.value<Sprite *>();
            if(icon) {
                showIcon = true;
                m_icon->setSprite(icon);

                if(iconMode) {
                    m_icon->setSize(iconSize);
                    m_icon->setHorizontalPolicy(Widget::Fixed);
                    m_icon->setVerticalPolicy(Widget::Fixed);
                    m_icon->setMargin(Vector4(0.0f));
                } else {
                    m_icon->setSize(iconSize);
                    m_icon->setHorizontalPolicy(Widget::Fixed);
                    m_icon->setVerticalPolicy(Widget::Fixed);
                    m_icon->setMargin(Vector4(0.0f, 4.0f, 0.0f, 0.0f));
                }
            }
        }
    }

    if(m_icon) {
        m_icon->setEnabled(showIcon);
        m_icon->setVisible(showIcon);
    }

    if(m_label) {
        m_label->setVisible(true);
        m_label->setText(m_modelIndex.model()->data(m_modelIndex, AbstractItemModel::DisplayRole).toString());
        if(iconMode) {
            m_label->setAlign(Alignment::Center | Alignment::Bottom);
            m_label->setSize(Vector2(std::max(cellSize.x - 4.0f, 1.0f), 16.0f));
            m_label->setHorizontalPolicy(Widget::Fixed);
            m_label->setVerticalPolicy(Widget::Fixed);
        } else {
            m_label->setAlign(Alignment::Left | Alignment::Middle);
            m_label->setSize(Vector2(std::max(m_label->textWidth(), 1.0f), std::max(cellSize.y, 1.0f)));
            m_label->setHorizontalPolicy(Widget::Expanding);
            m_label->setVerticalPolicy(Widget::Fixed);
        }
    }

    if(iconMode) {
        setSize(Vector2(std::max(cellSize.x, 1.0f), std::max(cellSize.y, 1.0f)));
    } else {
        setSize(Vector2(std::max(layout()->sizeHint().x, 1.0f), std::max(cellSize.y, 1.0f)));
    }
}

void ItemViewDelegate::composeComponent() {
    Widget::composeComponent();

    m_arrowIndent = Widget::createWidget<Widget>("arrowIndent", this);
    if(m_arrowIndent) {
        m_arrowIndent->setSize(Vector2(0.0f));
        m_arrowIndent->setHorizontalPolicy(Widget::Fixed);
        m_arrowIndent->setVerticalPolicy(Widget::Fixed);
        m_arrowIndent->setVisible(false);
    }

    m_arrow = Widget::createWidget<Image>("arrow", this);
    if(m_arrow) {
        m_arrow->setSprite(Engine::loadResource<Sprite>(".embedded/generic.png/Down"));
        m_arrow->setMargin(Vector4(0.0f, 0.0f, 0.0f, 2.0f));
        m_arrow->setSize(Vector2(12.0f));
        m_arrow->setHorizontalPolicy(Widget::Fixed);
        m_arrow->setVerticalPolicy(Widget::Fixed);
        m_arrow->setVisible(false);
    }

    m_icon = Widget::createWidget<Image>("icon", this);
    if(m_icon) {
        m_icon->setSize(Vector2(16.0f));
        m_icon->setHorizontalPolicy(Widget::Fixed);
        m_icon->setVerticalPolicy(Widget::Fixed);
    }

    m_label = Widget::createWidget<Label>("label", this);
    if(m_label) {
        m_label->setAlign(Alignment::Left | Alignment::Middle);
        m_label->setWordWrap(false);
        m_label->setClip(true);
        m_label->setHorizontalPolicy(Widget::Expanding);
        m_label->setVerticalPolicy(Widget::Fixed);
    }

    for(Widget *child : childWidgets()) {
        setSubWidget(child);
    }

    Layout *itemLayout = new Layout;
    itemLayout->setOrientation(Widget::Horizontal);
    setLayout(itemLayout);

    updateStyle();
}
