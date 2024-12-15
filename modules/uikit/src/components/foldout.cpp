#include "components/foldout.h"

#include "components/label.h"
#include "components/frame.h"
#include "components/image.h"
#include "components/button.h"
#include "components/recttransform.h"
#include "components/layout.h"

#include <components/textrender.h>

namespace {
    const char *gButton("Button");
    const char *gFrame("Frame");
    const char *gLabel("Label");
}

Foldout::Foldout() :
        m_container(nullptr),
        m_indicator(nullptr),
        m_label(nullptr) {

}

void Foldout::addWidget(Widget *widget) {
    if(m_container) {
        widget->rectTransform()->setParentTransform(m_container->rectTransform());

        Layout *layout = m_container->rectTransform()->layout();
        if(layout) {
            layout->addTransform(widget->rectTransform());
        }
    }
}

bool Foldout::isExpanded() const {
    if(m_container) {
        return m_container->actor()->isEnabled();
    }
    return false;
}

void Foldout::setExpanded(bool expanded) {
    if(m_container) {
        m_container->actor()->setEnabled(expanded);

        Image *icon = m_indicator->icon();
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

std::string Foldout::text() const {
    if(m_label) {
        return m_label->text();
    }
    return std::string();
}

void Foldout::setText(const std::string text) {
    if(m_label) {
        m_label->setText(text);
    }
}

void Foldout::onExpand() {
    setExpanded(!isExpanded());
}

void Foldout::composeComponent() {
    Widget::composeComponent();

    Actor *container = Engine::composeActor(gFrame, "Container", actor());
    m_container = container->getComponent<Frame>();
    m_container->setColor(Vector4(0.0f, 0.0f, 0.0f, 0.25f));

    RectTransform *containerRect = m_container->rectTransform();
    containerRect->setAnchors(0.0f, 1.0f);
    containerRect->setPivot(Vector2(0.0f, 0.0f));
    containerRect->setVerticalPolicy(RectTransform::Preferred);

    Layout *containerLayout = new Layout;
    containerRect->setLayout(containerLayout);

    Actor *indicator = Engine::composeActor(gButton, "Indicator", actor());
    m_indicator = indicator->getComponent<Button>();

    m_indicator->setText("");
    m_indicator->setIconSize(Vector2(16.0f, 8.0f));
    m_indicator->setNormalColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));

    Image *icon = m_indicator->icon();
    if(icon) {
        icon->setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
        icon->setItem("Arrow");
    }

    Object::connect(m_indicator, _SIGNAL(clicked()), this, _SLOT(onExpand()));

    RectTransform *indicatorRect = m_indicator->rectTransform();
    indicatorRect->setSize(20);

    Actor *label = Engine::composeActor(gLabel, gLabel, actor());
    m_label = label->getComponent<Label>();
    m_label->setAlign(Alignment::Top | Alignment::Left);

    RectTransform *labelRect = m_label->rectTransform();

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
