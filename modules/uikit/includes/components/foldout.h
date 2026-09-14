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
#ifndef FOLDOUT_H
#define FOLDOUT_H

#include "widget.h"

class CheckBox;
class Frame;
class Label;

class UIKIT_EXPORT Foldout : public Widget {
    A_OBJECT(Foldout, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, Foldout::text, Foldout::setText),
        A_PROPERTYEX(Frame, container, Foldout::container, Foldout::setContainer, "editor=Component"),
        A_PROPERTYEX(CheckBox, indicator, Foldout::indicator, Foldout::setIndicator, "editor=Component")
    )
    A_METHODS(
        A_SLOT(Foldout::onExpand)
    )
    A_NOENUMS()

public:
    Foldout();

    void insertWidget(int index, Widget *widget);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    TString text() const;
    void setText(const TString &text);

    Frame *container() const;
    void setContainer(Frame *container);

    CheckBox *indicator() const;
    void setIndicator(CheckBox *indicator);

    Label *label() const;
    void setLabel(Label *label);

    void onExpand();

protected:
    void insertRect(int index, RectTransform *content);

    void composeComponent() override;

    void childAdded(RectTransform *rect) override;

private:
    RectTransform *m_contentArea;

    CheckBox *m_indicator;
};

#endif // FOLDOUT_H
