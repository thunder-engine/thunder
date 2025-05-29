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

    const char *gButtonClass("Button");
    const char *gFrameClass("Frame");
    const char *gLabelClass("Label");
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
    Adds a \a widget to the foldout's container, effectively placing it inside the foldout's expanded content area.
*/
void Foldout::addWidget(Widget *widget) {
    Frame *container = Foldout::container();
    if(container) {
        RectTransform *rect = container->rectTransform();
        widget->rectTransform()->setParentTransform(rect);

        Layout *layout = rect->layout();
        if(layout) {
            layout->addTransform(widget->rectTransform());
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
std::string Foldout::text() const {
    Label *label = Foldout::label();
    if(label) {
        return label->text();
    }
    return std::string();
}
/*!
    Sets the label \a text for the foldout.
*/
void Foldout::setText(const std::string text) {
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
    setSubWidget(gContainer, container);
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
    setSubWidget(gIndicator, indicator);
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
    setSubWidget(gLabel, label);
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

    Actor *containerActor = Engine::composeActor(gFrameClass, "Container", actor());
    Frame *container = containerActor->getComponent<Frame>();
    container->setColor(Vector4(0.0f, 0.0f, 0.0f, 0.25f));
    setContainer(container);

    RectTransform *containerRect = container->rectTransform();
    containerRect->setAnchors(0.0f, 1.0f);
    containerRect->setPivot(Vector2(0.0f, 0.0f));
    containerRect->setVerticalPolicy(RectTransform::Preferred);

    Layout *containerLayout = new Layout;
    containerRect->setLayout(containerLayout);

    Actor *indicatorActor = Engine::composeActor(gButtonClass, "Indicator", actor());
    Button *indicator = indicatorActor->getComponent<Button>();

    indicator->setText("");
    indicator->setIconSize(Vector2(16.0f, 8.0f));
    indicator->setNormalColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    setIndicator(indicator);

    Image *icon = indicator->icon();
    if(icon) {
        icon->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
        icon->setItem("Arrow");
    }

    Object::connect(indicator, _SIGNAL(clicked()), this, _SLOT(onExpand()));

    RectTransform *indicatorRect = indicator->rectTransform();
    indicatorRect->setSize(20);

    Actor *labelActor = Engine::composeActor(gLabelClass, gLabelClass, actor());
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
