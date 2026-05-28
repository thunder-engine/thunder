#include "components/foldout.h"

#include "components/checkbox.h"
#include "components/label.h"
#include "components/recttransform.h"
#include "components/layout.h"

namespace {
    const char *gLabel("label");
    const char *gIndicator("indicator");
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
        m_indicator(nullptr) {

}
/*!
    Inserts \a widget to the foldout's container, at given position \a index.
    Effectively placing it inside the foldout's expanded content area.
*/
void Foldout::insertWidget(int index, Widget *widget) {
    insertRect(index, widget->rectTransform());
}
/*!
    Returns true id foldout is currently expanded; otherwise returns false.
*/
bool Foldout::isExpanded() const {
    if(m_contentArea) {
        return m_contentArea->actor()->isEnabled();
    }
    return false;
}
/*!
    Expands or collapses the foldout based on the \a expanded parameter.
*/
void Foldout::setExpanded(bool expanded) {
    if(m_contentArea) {
        m_contentArea->actor()->setEnabled(expanded);
        repaint();
    }
}
/*!
    Returns the current text of the foldout's label.
*/
TString Foldout::text() const {
    Label *label = static_cast<Label *>(subWidget(gLabel));
    if(label) {
        return label->text();
    }
    return TString();
}
/*!
    Sets the label \a text for the foldout.
*/
void Foldout::setText(const TString &text) {
    Label *label = static_cast<Label *>(subWidget(gLabel));
    if(label) {
        label->setText(text);
    } else {
        Actor *labelActor = Engine::composeActor<Label>(gLabel, m_indicator->actor());
        Label *label = labelActor->getComponent<Label>();
        label->setText(text);
        label->setAlign(Alignment::Left | Alignment::Middle);
        setSubWidget(label);

        RectTransform *rect = label->rectTransform();
        rect->setPadding(Vector4(0.0f, 0.0f, 0.0f, 20.0f));
        rect->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
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

    m_contentArea = container->rectTransform();
    if(m_contentArea && !m_contentArea->layout()) {
        m_contentArea->setHorizontalPolicy(RectTransform::Expanding);
        m_contentArea->setVerticalPolicy(RectTransform::Preferred);

        Layout *contentLayout = new Layout();
        contentLayout->setOrientation(Widget::Vertical);
        m_contentArea->setLayout(contentLayout);
    }
}
/*!
    Returns indicator button to fold and unfold container with content.
*/
CheckBox *Foldout::indicator() const {
    return static_cast<CheckBox *>(subWidget(gIndicator));
}
/*!
    Sets \a indicator button to fold and unfold container with content.
*/
void Foldout::setIndicator(CheckBox *indicator) {
    setSubWidget(indicator);

    RectTransform *rect = indicator->rectTransform();
    rect->setHorizontalPolicy(RectTransform::Expanding);

    m_indicator = indicator;
}
/*!
    Toggles the expanded state of the foldout when the indicator is clicked.
*/
void Foldout::onExpand() {
    setExpanded(!isExpanded());
}
/*!
    \internal
*/
void Foldout::childAdded(RectTransform *rect) {
    if(rect && rect != m_indicator->rectTransform() && rect != m_contentArea) {
        insertRect(-1, rect);
    }
}
/*!
    \internal
*/
void Foldout::insertRect(int index, RectTransform *content) {
    content->setHorizontalPolicy(RectTransform::Expanding);
    content->setVerticalPolicy(RectTransform::Fixed);

    m_contentArea->layout()->addTransform(content);
    repaint();
}
/*!
    \internal
*/
void Foldout::composeComponent() {
    Actor *actor = Widget::actor();
    // Fold button
    Actor *indicatorActor = Engine::composeActor<CheckBox>(gIndicator, actor);
    CheckBox *indicator = indicatorActor->getComponent<CheckBox>();

    indicator->setIndicator(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
    indicator->setIndicatorSize(Vector2(16.0f, 8.0f));
    indicator->setFoldMode(true);
    setIndicator(indicator);

    Object::connect(indicator, _SIGNAL(clicked()), this, _SLOT(onExpand()));

    // Content container
    Actor *containerActor = Engine::composeActor<Frame>(gContainer, actor);
    setContainer(containerActor->getComponent<Frame>());

    // Need to call it after sub widgets creation
    Widget::composeComponent();

    Layout *mainLayout = new Layout();
    mainLayout->setOrientation(Widget::Vertical);

    RectTransform *rect = rectTransform();
    rect->setLayout(mainLayout);
    rect->setVerticalPolicy(RectTransform::Preferred);

    // Initially add tab bar and content area to layout
    mainLayout->addTransform(indicator->rectTransform());
    mainLayout->addTransform(m_contentArea);

    rect->setSize(Vector2(100.0f, 20.0f));
}
