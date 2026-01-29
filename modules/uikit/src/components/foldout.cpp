#include "components/foldout.h"

#include "components/label.h"
#include "components/frame.h"
#include "components/image.h"
#include "components/button.h"
#include "components/recttransform.h"
#include "components/layout.h"

#include <components/textrender.h>

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

Foldout::Foldout() {

}
/*!
    Inserts \a widget to the foldout's container, at given position \a index. Effectively placing it inside the foldout's expanded content area.
*/
void Foldout::insertWidget(int index, Widget *widget) {
    Frame *container = Foldout::container();
    if(container) {
        RectTransform *rect = container->rectTransform();
        widget->rectTransform()->setParentTransform(rect);

        Layout *layout = rect->layout();
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

        Button *indicator = Foldout::indicator();
        if(indicator) {
            Image *icon = indicator->icon();
            if(icon) {
                RectTransform *rect = icon->rectTransform();
                if(rect) {
                    rect->setRotation(Vector3(0.0f, 0.0f, expanded ? 0.0f : 90.0f));
                }
            }

            RectTransform *rect = rectTransform();
            Layout *layout = rect->layout();

            layout->invalidate();
        }
    }
}
/*!
    Returns the current text of the foldout's label.
*/
TString Foldout::text() const {
    Label *label = Foldout::label();
    if(label) {
        return label->text();
    }
    return TString();
}
/*!
    Sets the label \a text for the foldout.
*/
void Foldout::setText(const TString text) {
    Label *label = Foldout::label();
    if(label) {
        label->setText(text);
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
Button *Foldout::indicator() const {
    return static_cast<Button *>(subWidget(gIndicator));
}
/*!
    Sets \a indicator button to fold and unfold container with content.
*/
void Foldout::setIndicator(Button *indicator) {
    setSubWidget(indicator);
}
/*!
    Returns a text label represents foldout header.
*/
Label *Foldout::label() const {
    return static_cast<Label *>(subWidget(gLabel));
}
/*!
    Sets a text \a label represents foldout header.
*/
void Foldout::setLabel(Label *label) {
    setSubWidget(label);
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

    Actor *containerActor = Engine::composeActor<Frame>(gContainer, actor());
    Frame *container = containerActor->getComponent<Frame>();
    container->setColor(Vector4(0.0f, 0.0f, 0.0f, 0.25f));
    setContainer(container);

    RectTransform *containerRect = container->rectTransform();
    containerRect->setAnchors(0.0f, 1.0f);
    containerRect->setPivot(Vector2(0.0f, 0.0f));
    containerRect->setVerticalPolicy(RectTransform::Preferred);

    Layout *containerLayout = new Layout;
    containerRect->setLayout(containerLayout);

    Actor *indicatorActor = Engine::composeActor<Button>(gIndicator, actor());
    Button *indicator = indicatorActor->getComponent<Button>();

    indicator->setText("");
    indicator->setIconSize(Vector2(16.0f, 8.0f));
    indicator->setColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    setIndicator(indicator);

    Image *icon = indicator->icon();
    if(icon) {
        icon->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png/Arrow"));
    }

    Object::connect(indicator, _SIGNAL(clicked()), this, _SLOT(onExpand()));

    RectTransform *indicatorRect = indicator->rectTransform();
    indicatorRect->setSize(20);

    Actor *labelActor = Engine::composeActor<Label>(gLabel, actor());
    Label *label = labelActor->getComponent<Label>();
    label->setAlign(Alignment::Top | Alignment::Left);
    setLabel(label);

    RectTransform *labelRect = label->rectTransform();

    Layout *horizontalLauout = new Layout;
    horizontalLauout->setDirection(Layout::Horizontal);
    horizontalLauout->setSpacing(8.0f);

    Layout *layout = new Layout;

    RectTransform *rect = rectTransform();
    rect->setLayout(layout);
    rect->setVerticalPolicy(RectTransform::Preferred);

    horizontalLauout->addTransform(indicatorRect);
    horizontalLauout->addTransform(labelRect);

    layout->addLayout(horizontalLauout);
    layout->addTransform(containerRect);
}
