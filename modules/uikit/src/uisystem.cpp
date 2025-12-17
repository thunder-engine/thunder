#include "uisystem.h"

#include <components/actor.h>
#include <systems/resourcesystem.h>
#include <commandbuffer.h>

#include "components/recttransform.h"
#include "components/widget.h"
#include "components/image.h"
#include "components/label.h"
#include "components/button.h"
#include "components/switch.h"
#include "components/checkbox.h"
#include "components/progressbar.h"
#include "components/frame.h"
#include "components/menu.h"
#include "components/lineedit.h"
#include "components/floatinput.h"
#include "components/toolbutton.h"
#include "components/foldout.h"
#include "components/uiloader.h"
#include "components/slider.h"
#include "components/scrollbar.h"

#include "pipelinetasks/guilayer.h"

std::list<Widget *> UiSystem::m_uiComponents;
std::mutex UiSystem::m_mutex;

UiSystem::UiSystem() :
        System() {

    PROFILE_FUNCTION();

    StyleSheet::registerClassFactory(Engine::resourceSystem());
    UiDocument::registerClassFactory(Engine::resourceSystem());

    RectTransform::registerClassFactory(this);

    Widget::registerClassFactory(this);
    Image::registerClassFactory(this);
    Frame::registerClassFactory(this);
    Label::registerClassFactory(this);

    AbstractButton::registerClassFactory(this);
    Button::registerClassFactory(this);
    Switch::registerClassFactory(this);
    CheckBox::registerClassFactory(this);

    ProgressBar::registerClassFactory(this);
    AbstractSlider::registerClassFactory(this);
    Slider::registerClassFactory(this);
    ScrollBar::registerClassFactory(this);

    Menu::registerClassFactory(this);

    LineEdit::registerClassFactory(this);
    FloatInput::registerClassFactory(this);

    ToolButton::registerClassFactory(this);

    Foldout::registerClassFactory(this);

    GuiLayer::registerClassFactory(this);

    UiLoader::registerClassFactory(this);

    setName("Ui");
}

UiSystem::~UiSystem() {
    PROFILE_FUNCTION();

    RectTransform::unregisterClassFactory(this);

    Widget::unregisterClassFactory(this);
    Image::unregisterClassFactory(this);
    Frame::unregisterClassFactory(this);
    Label::unregisterClassFactory(this);

    AbstractButton::unregisterClassFactory(this);
    Button::unregisterClassFactory(this);
    Switch::unregisterClassFactory(this);

    ProgressBar::unregisterClassFactory(this);
    AbstractSlider::unregisterClassFactory(this);
    Slider::unregisterClassFactory(this);
    ScrollBar::unregisterClassFactory(this);

    Menu::unregisterClassFactory(this);

    LineEdit::unregisterClassFactory(this);
    FloatInput::unregisterClassFactory(this);

    ToolButton::unregisterClassFactory(this);

    Foldout::unregisterClassFactory(this);

    UiLoader::unregisterClassFactory(this);

    StyleSheet::unregisterClassFactory(Engine::resourceSystem());
    UiDocument::unregisterClassFactory(Engine::resourceSystem());

    m_uiComponents.clear();
}

void UiSystem::update(World *) {
    PROFILE_FUNCTION();

}

int UiSystem::threadPolicy() const {
    return Main;
}

void UiSystem::addWidget(Widget *widget) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_uiComponents.push_back(widget);
}

void UiSystem::removeWidget(Widget *widget) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_uiComponents.remove(widget);
}

void UiSystem::riseWidget(Widget *widget) {
    if(widget) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_uiComponents.remove(widget);
        m_uiComponents.push_back(widget);
    }
}

void UiSystem::lowerWidget(Widget *widget) {
    if(widget) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_uiComponents.remove(widget);
        m_uiComponents.push_front(widget);
    }
}

std::list<Widget *> UiSystem::widgets() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_uiComponents;
}

void UiSystem::composeComponent(Component *component) const {
    component->composeComponent();
}
