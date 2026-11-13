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
#include "foldout.h"

#include "checkbox.h"
#include "label.h"
#include "layout.h"

namespace {
    const char *gLabel("label");
    const char *gHeader("header");
    const char *gContainer("container");
}

/*!
    \class Foldout
    \brief The Foldout class that implements a UI element which allows for expanding and collapsing content.
    \inmodule Gui

    The Foldout class manages a UI widget that can show or hide additional content based on its expanded state.
    It includes functionality for adding widgets to the foldout, toggling its expanded state.
*/

Foldout::Foldout() :
        m_contentArea(nullptr),
        m_header(nullptr) {

}
/*!
    Returns the foldout header button.
*/
CheckBox *Foldout::header() const {
    return m_header;
}
/*!
    Inserts \a widget into the foldout header at given position \a index.
*/
void Foldout::insertHeaderWidget(int index, Widget *widget) {
    if(m_header && m_header->layout()) {
        m_header->layout()->insertWidget(index, widget);
    }
}
/*!
    Inserts \a widget to the foldout's container, at given position \a index.
    Effectively placing it inside the foldout's expanded content area.
*/
void Foldout::insertWidget(int index, Widget *widget) {
    widget->setHorizontalPolicy(Widget::Expanding);
    widget->setVerticalPolicy(Widget::Fixed);

    if(m_contentArea) {
        m_contentArea->layout()->addWidget(widget);
    }
    repaint();
}
/*!
    Returns true id foldout is currently expanded; otherwise returns false.
*/
bool Foldout::isExpanded() const {
    if(m_contentArea) {
        return m_contentArea->isVisible();
    }
    return false;
}
/*!
    Expands or collapses the foldout based on the \a expanded parameter.
*/
void Foldout::setExpanded(bool expanded) {
    if(m_contentArea) {
        const bool changed = isExpanded() != expanded;
        m_contentArea->setVisible(expanded);
        repaint();
        if(changed) {
            expandedChanged(expanded);
        }
    }
}
/*!
    Returns the current text of the foldout's label.
*/
TString Foldout::text() const {
    Label *label = this->label();
    if(label) {
        return label->text();
    }
    return TString();
}
/*!
    Sets the label \a text for the foldout.
*/
void Foldout::setText(const TString &text) {
    Label *label = this->label();
    if(label) {
        label->setText(text);
    } else {
        if(!m_header) {
            return;
        }
        Label *label = Widget::createWidget<Label>(gLabel, m_header);
        m_header->setSubWidget(label);
        label->setText(text);
        label->setAlign(Alignment::Left | Alignment::Middle);
        label->setPadding(Vector4(0.0f, 0.0f, 0.0f, 20.0f));
        label->setSize(Vector2(0.0f, 20.0f));
        label->setHorizontalPolicy(Widget::Expanding);
        label->setVerticalPolicy(Widget::Fixed);
    }
}
/*!
    Returns the label displayed in the header.
*/
Label *Foldout::label() const {
    return m_header ? static_cast<Label *>(m_header->subWidget(gLabel)) : nullptr;
}
/*!
    Sets the label displayed in the header.
*/
void Foldout::setLabel(Label *label) {
    if(m_header && label) {
        m_header->setSubWidget(label);
        label->setAlign(Alignment::Left | Alignment::Middle);
        label->setPadding(Vector4(0.0f, 0.0f, 0.0f, 20.0f));
        label->setSize(Vector2(0.0f, 20.0f));
        label->setHorizontalPolicy(Widget::Expanding);
        label->setVerticalPolicy(Widget::Fixed);
        if(m_header->layout() && m_header->layout()->indexOf(label) < 0) {
            m_header->layout()->addWidget(label);
        }
    }
}
/*!
    Returns container component attached to this widget.
*/
Frame *Foldout::container() const {
    return static_cast<Frame *>(subWidget(gContainer));
}
/*!
    Sets \a container component attached to this widget.
*/
void Foldout::setContainer(Frame *container) {
    setSubWidget(container);

    m_contentArea = container;
    if(m_contentArea && !m_contentArea->layout()) {
        m_contentArea->setHorizontalPolicy(Widget::Expanding);
        m_contentArea->setVerticalPolicy(Widget::Preferred);

        Layout *contentLayout = new Layout();
        contentLayout->setOrientation(Widget::Vertical);
        m_contentArea->setLayout(contentLayout);
    }
}
/*!
    Sets \a header button used to fold and unfold the content.
*/
void Foldout::setHeader(CheckBox *header) {
    if(header) {
        m_header = header;
        setSubWidget(header);
        if(!header->layout()) {
            header->setLayout(new Layout());
        }
        header->layout()->setOrientation(Widget::Horizontal);
        header->setSize(Vector2(100.0f, 20.0f));
        header->setPadding(Vector4(0.0f));
        header->setHorizontalPolicy(Widget::Expanding);
        header->setVerticalPolicy(Widget::Fixed);
    }
}
/*!
    Toggles the expanded state of the foldout when the header is clicked.
*/
void Foldout::onExpand() {
    setExpanded(!isExpanded());
    headerClicked();
}
/*!
    Notifies listeners that the expanded state has changed.
*/
void Foldout::expandedChanged(bool expanded) {
    emitSignal(_SIGNAL(expandedChanged(bool)), Variant(expanded));
}
/*!
    Notifies listeners that a drag-and-drop operation targets this foldout.
*/
void Foldout::dropped(const TString &mimeType, const TString &data) {
    emitSignal(_SIGNAL(dropped(TString,TString)), Variant(mimeType), Variant(data));
}
/*!
    Notifies listeners that the foldout header has been clicked.
*/
void Foldout::headerClicked() {
    emitSignal(_SIGNAL(headerClicked()));
}
/*!
    \internal
*/
void Foldout::childAdded(Widget *widget) {
    if(widget && widget != m_header && widget != m_contentArea) {
        insertWidget(-1, widget);
    }
}
/*!
    \internal
*/
bool Foldout::onDrop(const TString &mimeType, const TString &data, int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    dropped(mimeType, data);
    return true;
}
void Foldout::composeComponent() {
    CheckBox *header = Widget::createWidget<CheckBox>(gHeader, this);

    header->setIndicator(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
    header->setIndicatorSize(Vector2(16.0f, 8.0f));
    header->setFoldMode(true);
    setHeader(header);

    Object::connect(header, _SIGNAL(clicked()), this, _SLOT(onExpand()));

    // Content container
    Widget *containerActor = Widget::createWidget<Frame>(gContainer, this);
    setContainer(dynamic_cast<Frame *>(containerActor));
    if(m_contentArea == nullptr) {
        return;
    }

    // Need to call it after sub widgets creation
    Widget::composeComponent();

    Layout *mainLayout = new Layout();
    mainLayout->setOrientation(Widget::Vertical);

    setLayout(mainLayout);
    setVerticalPolicy(Widget::Preferred);

    mainLayout->addWidget(m_header);
    mainLayout->addWidget(m_contentArea);

    setSize(Vector2(100.0f, 20.0f));
}
