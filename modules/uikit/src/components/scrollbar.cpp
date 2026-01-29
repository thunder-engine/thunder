#include "components/scrollbar.h"

#include "components/frame.h"

#include <components/recttransform.h>
#include <components/button.h>
#include <components/image.h>
#include <components/label.h>

namespace {
    const char *gKnob("knob");
    const char *gBackArrow("backArrow");
    const char *gFrontArrow("frontArrow");

    const char *gSpriteArrow(".embedded/ui.png/Arrow");
}

ScrollBar::ScrollBar() :
        m_pageStep(2),
        m_singleStep(1) {

    m_areaGap = 20.0f;
}

void ScrollBar::setValue(int value) {
    AbstractSlider::setValue(value);

    recalcKnob();
}

void ScrollBar::setMinimum(int value) {
    AbstractSlider::setMinimum(value);

    recalcKnob();
}

void ScrollBar::setMaximum(int value) {
    AbstractSlider::setMaximum(value);

    recalcKnob();
}

void ScrollBar::setOrientation(int value) {
    AbstractSlider::setOrientation(value);

    setBackArrow(backArrow());
    setFrontArrow(frontArrow());

    recalcKnob();
}

int ScrollBar::pageStep() const {
    return m_pageStep;
}

void ScrollBar::setPageStep(int page) {
    m_pageStep = page;

    recalcKnob();
}

int ScrollBar::singleStep() const {
    return m_singleStep;
}

void ScrollBar::setSingleStep(int step) {
    m_singleStep = step;
}

void ScrollBar::stepBack() {
    setValue(m_value - m_singleStep);
}

void ScrollBar::stepFront() {
    setValue(m_value + m_singleStep);
}

Widget *ScrollBar::backArrow() const {
    return subWidget(gBackArrow);
}

void ScrollBar::setBackArrow(Widget *arrow) {
    Button *button = dynamic_cast<Button *>(backArrow());
    if(button) {
        disconnect(button, _SIGNAL(pressed()), this, _SLOT(stepBack()));
    }

    setSubWidget(arrow);

    if(arrow) {
        RectTransform *rect = arrow->rectTransform();
        if(m_orientation == Horizontal) {
            rect->setPivot(Vector2(0.0f, 0.5f));
            rect->setSize(Vector2(m_areaGap, 0.0f));
            rect->setAnchors(Vector2(0.0f, 0.0f), Vector2(0.0, 1.0f));
        } else {
            rect->setPivot(Vector2(0.5f, 1.0f));
            rect->setSize(Vector2(0.0f, m_areaGap));
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0, 1.0f));
        }
    }

    button = dynamic_cast<Button *>(arrow);
    if(button) {
        connect(button, _SIGNAL(pressed()), this, _SLOT(stepBack()));

        RectTransform *rectIcon = button->icon()->rectTransform();
        if(m_orientation == Horizontal) {
            rectIcon->setRotation(Vector3(0.0f, 0.0f,-90.0f));
        } else {
            rectIcon->setRotation(Vector3(0.0f, 0.0f, 180.0f));
        }
    }
}

Widget *ScrollBar::frontArrow() const {
    return subWidget(gFrontArrow);
}

void ScrollBar::setFrontArrow(Widget *arrow) {
    Button *button = dynamic_cast<Button *>(frontArrow());
    if(button) {
        disconnect(button, _SIGNAL(pressed()), this, _SLOT(stepFront()));
    }

    setSubWidget(arrow);

    if(arrow) {
        RectTransform *rect = arrow->rectTransform();
        if(m_orientation == Horizontal) {
            rect->setPivot(Vector2(1.0f, 0.5f));
            rect->setSize(Vector2(m_areaGap, 0.0f));
            rect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0, 1.0f));
        } else {
            rect->setPivot(Vector2(0.5f, 0.0f));
            rect->setSize(Vector2(0.0f, m_areaGap));
            rect->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0, 0.0f));
        }
    }

    button = dynamic_cast<Button *>(arrow);
    if(button) {
        connect(button, _SIGNAL(pressed()), this, _SLOT(stepFront()));

        RectTransform *rectIcon = button->icon()->rectTransform();
        if(m_orientation == Horizontal) {
            rectIcon->setRotation(Vector3(0.0f, 0.0f, 90.0f));
        } else {
            rectIcon->setRotation(Vector3(0.0f, 0.0f, 0.0f));
        }
    }
}

void ScrollBar::recalcKnob() {
    Widget *knob = ScrollBar::knob();
    if(knob) {
        RectTransform *rect = rectTransform();
        Vector2 size(rect->size());

        int scrollingRange = m_maximum - m_minimum;

        float pageFactor = m_pageStep != 0 ? static_cast<float>(m_pageStep) / static_cast<float>(scrollingRange) : 1.0f;
        pageFactor = CLAMP(pageFactor, 0.0f, 1.0f);

        float factor = m_value != 0 ? static_cast<float>(m_value - m_minimum) / static_cast<float>(scrollingRange) : 0.0f;
        factor = CLAMP(factor * (1.0 - pageFactor), 0.0f, 1.0f);

        RectTransform *knobRect = knob->rectTransform();
        if(m_orientation == Horizontal) {
            float normalGap = m_areaGap / size.x;
            float scaledFactor = normalGap + factor * (1.0f - normalGap * 2.0f);
            float scaledPage = pageFactor * (1.0f - normalGap * 2.0f);
            knobRect->setPivot(Vector2(0.0f, 0.5f));
            knobRect->setAnchors(Vector2(scaledFactor, 0.5f), Vector2(scaledFactor, 0.5f));
            knobRect->setSize(Vector2(MIX(0.0f, size.x, scaledPage), m_hovered ? 8.0f : 2.0f));
        } else {
            float normalGap = m_areaGap / size.y;
            float scaledFactor = normalGap + factor * (1.0f - normalGap * 2.0f);
            float scaledPage = pageFactor * (1.0f - normalGap * 2.0f);
            knobRect->setPivot(Vector2(0.5f, 1.0f));
            knobRect->setAnchors(Vector2(0.5f, 1.0f - scaledFactor), Vector2(0.5f, 1.0f - scaledFactor));
            knobRect->setSize(Vector2(m_hovered ? 8.0f : 2.0f, MIX(0.0f, size.y, scaledPage)));
        }
    }
}

void ScrollBar::boundChanged(const Vector2 &size) {
    AbstractSlider::boundChanged(size);

    recalcKnob();
}

void ScrollBar::update() {
    bool hovered = m_hovered;

    AbstractSlider::update();

    if(hovered != m_hovered) {
        Widget *knob = ScrollBar::knob();
        if(knob) {
            Frame *frame = dynamic_cast<Frame *>(knob);
            if(frame) {
                frame->setCorners(m_hovered ? Vector4(5.0f) : Vector4(0.0f));
            }

            RectTransform *knobRect = knob->rectTransform();

            Vector2 size(knobRect->size());
            if(m_orientation == Horizontal) {
                knobRect->setSize(Vector2(size.x, m_hovered ? 8.0f : 2.0f));
            } else {
                knobRect->setSize(Vector2(m_hovered ? 8.0f : 2.0f, size.y));
            }
        }
    }
}

void ScrollBar::composeComponent() {
    AbstractSlider::composeComponent();

    // Add knob
    Actor *knobActor = Engine::composeActor<Frame>(gKnob, actor());
    Frame *knob = knobActor->getComponent<Frame>();
    knob->setCorners(Vector4(0.0f));
    knob->setColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

    RectTransform *rectKnob = knob->rectTransform();

    if(m_orientation == Horizontal) {
        rectKnob->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
    } else {
        rectKnob->setAnchors(Vector2(0.5f, 0.0f), Vector2(0.5f, 0.0f));
    }
    setKnob(knob);

    // Add back arrow
    Actor *backActor = Engine::composeActor<Button>(gBackArrow, actor());
    Button *back = backActor->getComponent<Button>();
    back->setText(TString());

    Image *backIcon = back->icon();
    if(backIcon) {
        backIcon->setSprite(Engine::loadResource<Sprite>(gSpriteArrow));
        RectTransform *rectIcon = backIcon->rectTransform();
        if(rectIcon) {
            rectIcon->setSize(Vector2(16.0f, 8.0f));
        }
    }
    setBackArrow(back);

    // Add front arrow
    Actor *frontActor = Engine::composeActor<Button>(gFrontArrow, actor());
    Button *front = frontActor->getComponent<Button>();
    front->setText(TString());

    Image *frontIcon = front->icon();
    if(frontIcon) {
        frontIcon->setSprite(Engine::loadResource<Sprite>(gSpriteArrow));
        RectTransform *rectIcon = frontIcon->rectTransform();
        if(rectIcon) {
            rectIcon->setSize(Vector2(16.0f, 8.0f));
        }
    }
    setFrontArrow(front);

    setValue(m_value);

    if(m_orientation == Horizontal) {
        rectTransform()->setSize(Vector2(100.0f, 20.0f));
    } else {
        rectTransform()->setSize(Vector2(20.0f, 100.0f));
    }
}
