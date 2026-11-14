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
#ifndef ITEMVIEWDELEGATE_H
#define ITEMVIEWDELEGATE_H

#include <frame.h>
#include <image.h>
#include <label.h>
#include <vector>

class AbstractItemView;

class UIKIT_EXPORT ItemViewDelegate : public Frame {
    A_OBJECT(ItemViewDelegate, Frame, Delegates)

    A_PROPERTIES(
        A_PROPERTY(Widget *, arrowIndentWidget, ItemViewDelegate::arrowIndentWidget, ItemViewDelegate::setArrowIndentWidget),
        A_PROPERTY(Image *, arrowWidget, ItemViewDelegate::arrowWidget, ItemViewDelegate::setArrowWidget),
        A_PROPERTY(Image *, iconWidget, ItemViewDelegate::iconWidget, ItemViewDelegate::setIconWidget),
        A_PROPERTY(Label *, labelWidget, ItemViewDelegate::labelWidget, ItemViewDelegate::setLabelWidget)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    ItemViewDelegate();
    ~ItemViewDelegate();

    virtual void bind(AbstractItemView *view, const ModelIndex &index);
    bool beginEditing(AbstractItemView *view);

    int index() const;
    const ModelIndex &modelIndex() const;

    bool isArrowVisible() const;
    void setArrowVisible(bool visible);
    bool handleClick(const Vector2 &position, const Vector2 &viewOffset);

    void setArrowTabSize(float size);
    void setArrowExpanded(bool expanded);

    void setSelected(bool selected);
    void setHovered(bool hovered);

    Widget *arrowIndentWidget() const;
    void setArrowIndentWidget(Widget *indent);

    Image *arrowWidget() const;
    void setArrowWidget(Image *arrow);

    Image *iconWidget() const;
    void setIconWidget(Image *icon);

    Label *labelWidget() const;
    void setLabelWidget(Label *label);

protected:
    struct ClickArea {
        TString name;
        Widget *widget;
    };

    void addClickArea(const TString &name, Widget *widget);
    void clearClickAreas();

    void updateStyle();

    virtual void updateData(AbstractItemView *view);

    void composeComponent() override;

protected:
    AbstractItemView *m_view;
    ModelIndex m_modelIndex;

    Widget *m_arrowIndent;
    Image *m_arrow;
    Image *m_icon;

    mutable Label *m_label;

    bool m_selected;

    bool m_hovered;

    float m_arrowTabSize;

    std::vector<ClickArea> m_clickAreas;

};

#endif // ITEMVIEWDELEGATE_H
