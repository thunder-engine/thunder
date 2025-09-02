#include "components/slider.h"

#include "components/progressbar.h"
#include "components/recttransform.h"

#include <timer.h>
#include <input.h>

namespace {
    const char *gBackground("background");
    const char *gKnob("knob");

    const char *gProgressBarClass("ProgressBar");
    const char *gFrameClass("Frame");

    const float gFadeDuration = 0.2f;
}

Slider::Slider() :
        m_normalColor(0.008f, 0.467f, 0.741f, 1.0f),
        m_highlightedColor(0.0078f, 0.533f, 0.819f, 1.0f) {

}

void Slider::update() {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }

    RectTransform *rect = rectTransform();

    bool hover = rect->isHovered(pos.x, pos.y);
    if(m_hovered != hover) {
        m_currentFade = 0.0f;
        m_hovered = hover;
    }

    Vector4 color(m_normalColor);

    if(m_hovered) {
        color = Vector4(m_highlightedColor);

        if(Input::isMouseButtonDown(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;

            Widget::setFocusWidget(this);

            emitSignal(_SIGNAL(pressed()));
        }

        if(Input::isMouseButton(Input::MOUSE_LEFT) || Input::touchCount() > 0) {
            Vector3 min, max;
            rect->bound().box(min, max);
            Vector2 local(Vector2(pos.x, pos.y) - min);
            Vector2 factor(local.x / (max.x - min.x), local.y / (max.y - min.y));

            float f = (m_orientation == Horizontal) ? factor.x : factor.y;
            int newValue = round(MIX(m_minimum, m_maximum, f));
            if(newValue != m_value) {
                setValue(newValue);

                emitSignal(_SIGNAL(valueChanged(int)), newValue);
            }
        }
    }

    if(m_currentFade < 1.0f) {
        m_currentFade += 1.0f / gFadeDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

        ProgressBar *bar = background();
        if(bar) {
            bar->setProgressColor(MIX(bar->progressColor(), color, m_currentFade));
        }
    }
}

ProgressBar *Slider::background() const {
    return static_cast<ProgressBar *>(subWidget(gBackground));
}

void Slider::setBackground(ProgressBar *background) {
    setSubWidget(background);

    if(background) {
        background->setFrom(m_minimum);
        background->setTo(m_maximum);
        background->setValue(m_value);
    }
}

void Slider::setValue(int value) {
    AbstractSlider::setValue(value);

    Widget *knob = AbstractSlider::knob();
    if(knob) {
        float factor = value != 0 ? static_cast<float>(value) / static_cast<float>(m_maximum-m_minimum) : 0.0f;
        factor = CLAMP(factor, 0.0f, 1.0f);
        if(m_orientation == Horizontal) {
            knob->rectTransform()->setAnchors(Vector2(factor, 0.5f), Vector2(factor, 0.5f));
        } else {
            knob->rectTransform()->setAnchors(Vector2(0.5f, factor), Vector2(0.5f, factor));
        }
    }

    ProgressBar *bar = background();
    if(bar) {
        bar->setValue(value);
    }
}

void Slider::setMinimum(int value) {
    AbstractSlider::setMinimum(value);

    ProgressBar *bar = background();
    if(bar) {
        bar->setFrom(value);
    }
}

void Slider::setMaximum(int value) {
    AbstractSlider::setMaximum(value);

    ProgressBar *bar = background();
    if(bar) {
        bar->setTo(value);
    }
}

void Slider::setOrientation(int value) {
    AbstractSlider::setOrientation(value);

    ProgressBar *bar = background();
    if(bar) {
        bar->setOrientation(value);

        RectTransform *rectBar = bar->rectTransform();
        if(m_orientation == Horizontal) {
            rectBar->setSize(Vector2(0.0f, 10.0f));
            rectBar->setAnchors(Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
        } else {
            rectBar->setSize(Vector2(10.0f, 0.0f));
            rectBar->setAnchors(Vector2(0.5f, 0.0f), Vector2(0.5f, 1.0f));
        }
    }

    setValue(m_value);
}

void Slider::composeComponent() {
    AbstractSlider::composeComponent();

    // Add background
    Actor *barActor = Engine::composeActor(gProgressBarClass, gBackground, actor());
    ProgressBar *bar = barActor->getComponent<ProgressBar>();
    bar->setValue(m_value);
    bar->setProgressColor(m_normalColor);

    setBackground(bar);

    setOrientation(Horizontal);

    // Add knob
    Actor *knobActor = Engine::composeActor(gFrameClass, gKnob, actor());
    Frame *knob = knobActor->getComponent<Frame>();
    knob->setCorners(Vector4(8.0f));
    knob->setColor(Vector4(1.0f));

    RectTransform *rectKnob = knob->rectTransform();
    rectKnob->setSize(Vector2(16.0f, 16.0f));
    rectKnob->setAnchors(Vector2(0.5f), Vector2(0.5f));

    setKnob(knob);

    setValue(m_value);

    if(m_orientation == Horizontal) {
        rectTransform()->setSize(Vector2(100.0f, 20.0f));
    } else {
        rectTransform()->setSize(Vector2(20.0f, 100.0f));
    }
}
