#include "components/foldout.h"

#include "components/frame.h"
#include "components/image.h"
#include "components/checkbox.h"
#include "components/recttransform.h"
#include "components/layout.h"

#include <components/textrender.h>

namespace {
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

Foldout::Foldout() {

}
/*!
    Inserts \a widget to the foldout's container, at given position \a index. Effectively placing it inside the foldout's expanded content area.
*/
void Foldout::insertWidget(int index, Widget *widget) {
    Frame *container = Foldout::container();
    if(container) {
        RectTransform *parentRect = container->rectTransform();
        Layout *layout = parentRect->layout();
        if(layout) {
            layout->insertTransform(index, widget->rectTransform());
        }
    }
}
/*!
    Returns true id foldout is currently expanded; otherwise returns false.
*/
bool Foldout::isExpanded() const {
    Frame *container = Foldout::container();
    if(container) {
        return container->actor()->isEnabled();
    }
    return false;
}
/*!
    Expands or collapses the foldout based on the \a expanded parameter.
*/
void Foldout::setExpanded(bool expanded) {
    Frame *container = Foldout::container();
    if(container) {
        container->actor()->setEnabled(expanded);
    }
}
/*!
    Returns the current text of the foldout's label.
*/
TString Foldout::text() const {
    CheckBox *button = Foldout::indicator();
    if(button) {
        return button->text();
    }
    return TString();
}
/*!
    Sets the label \a text for the foldout.
*/
void Foldout::setText(const TString text) {
    CheckBox *button = Foldout::indicator();
    if(button) {
        button->setText(text);
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
void Foldout::composeComponent() {
    Widget::composeComponent();

    RectTransform *rect = rectTransform();
    rect->setLayout(new Layout);
    rect->setVerticalPolicy(RectTransform::Preferred);

    // Fold button
    Actor *indicatorActor = Engine::composeActor<CheckBox>(gIndicator, actor());
    CheckBox *indicator = indicatorActor->getComponent<CheckBox>();

    indicator->setText("");
    indicator->setIconSize(Vector2(16.0f, 8.0f));
    indicator->setColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    indicator->setFoldMode(true);
    setIndicator(indicator);

    Image *icon = indicator->knobGraphic();
    if(icon) {
        icon->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
    }

    RectTransform *indicatorRect = indicator->rectTransform();
    indicatorRect->setSize(20);
    indicatorRect->setPivot(Vector2(0.0f, 1.0f));

    Object::connect(indicator, _SIGNAL(clicked()), this, _SLOT(onExpand()));

    // Content container
    Actor *containerActor = Engine::composeActor<Frame>(gContainer, actor());
    Frame *container = containerActor->getComponent<Frame>();
    container->setColor(Vector4(0.0f, 0.0f, 0.0f, 0.25f));
    setContainer(container);

    RectTransform *containerRect = container->rectTransform();
    containerRect->setPivot(Vector2(0.0f, 1.0f));
    containerRect->setHorizontalPolicy(RectTransform::Expanding);
    containerRect->setVerticalPolicy(RectTransform::Preferred);
    containerRect->setLayout(new Layout);

    rect->setSize(Vector2(100.0f, 20.0f));
}
