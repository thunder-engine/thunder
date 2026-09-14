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

class AbstractItemView;

class UIKIT_EXPORT ItemViewDelegate : public Frame {
    A_OBJECT(ItemViewDelegate, Frame, Delegates)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ItemViewDelegate();
    ~ItemViewDelegate();

    virtual void bind(AbstractItemView *view, const ModelIndex &index);

    int index() const;
    const ModelIndex &modelIndex() const;

    void setSelected(bool selected);
    void setHovered(bool hovered);

protected:
    void updateStyle();

    virtual void updateData(AbstractItemView *view);

    void composeComponent() override;

protected:
    ModelIndex m_modelIndex;

    Image *m_icon;

    Label *m_label;

    bool m_selected;

    bool m_hovered;

};

#endif // ITEMVIEWDELEGATE_H
