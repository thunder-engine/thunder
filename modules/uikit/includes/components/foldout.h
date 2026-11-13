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

#include <widget.h>

class CheckBox;
class Frame;
class Label;

class UIKIT_EXPORT Foldout : public Widget {
    A_OBJECT(Foldout, Widget, UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, Foldout::text, Foldout::setText),
        A_PROPERTYEX(Frame, container, Foldout::container, Foldout::setContainer, "editor=Component"),
        A_PROPERTYEX(CheckBox, header, Foldout::header, Foldout::setHeader, "editor=Component")
    )
    A_METHODS(
        A_SLOT(Foldout::onExpand),
        A_SIGNAL(Foldout::expandedChanged),
        A_SIGNAL(Foldout::dropped),
        A_SIGNAL(Foldout::headerClicked)
    )
    A_NOENUMS()

public:
    Foldout();

    CheckBox *header() const;

    void insertHeaderWidget(int index, Widget *widget);

    void insertWidget(int index, Widget *widget);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    TString text() const;
    void setText(const TString &text);

    Frame *container() const;
    void setContainer(Frame *container);

    void setHeader(CheckBox *header);

    Label *label() const;
    void setLabel(Label *label);

    void onExpand();

    void expandedChanged(bool expanded);
    void dropped(const TString &mimeType, const TString &data);
    void headerClicked();

protected:
    void composeComponent() override;

    void childAdded(Widget *widget) override;
    bool onDrop(const TString &mimeType, const TString &data, int x, int y) override;

private:
    Widget *m_contentArea;

    CheckBox *m_header;
};

#endif // FOLDOUT_H
