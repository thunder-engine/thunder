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
#include "components/progressbar.h"
#include "components/frame.h"
#include "components/menu.h"
#include "components/textinput.h"
#include "components/floatinput.h"
#include "components/toolbutton.h"
#include "components/uiloader.h"

#include "pipelinetasks/guilayer.h"

list<Widget *> UiSystem::m_uiComponents;

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

    ProgressBar::registerClassFactory(this);

    Menu::registerClassFactory(this);

    TextInput::registerClassFactory(this);
    FloatInput::registerClassFactory(this);

    ToolButton::registerClassFactory(this);

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

    Menu::unregisterClassFactory(this);

    TextInput::unregisterClassFactory(this);
    FloatInput::unregisterClassFactory(this);

    ToolButton::unregisterClassFactory(this);

    UiLoader::unregisterClassFactory(this);

    StyleSheet::unregisterClassFactory(Engine::resourceSystem());
    UiDocument::unregisterClassFactory(Engine::resourceSystem());
}

bool UiSystem::init() {
    PROFILE_FUNCTION();

    return true;
}

void UiSystem::update(World *) {
    PROFILE_FUNCTION();

}

int UiSystem::threadPolicy() const {
    return Main;
}

Object *UiSystem::instantiateObject(const MetaObject *meta, const string &name, Object *parent) {
    Object *result = ObjectSystem::instantiateObject(meta, name, parent);
    Widget *widget = dynamic_cast<Widget *>(result);
    if(widget) {
        widget->actor()->setLayers(CommandBuffer::UI | CommandBuffer::RAYCAST);
    }
    return result;
}

void UiSystem::addWidget(Widget *widget) {
    m_uiComponents.push_back(widget);
}

void UiSystem::removeWidget(Widget *widget) {
    m_uiComponents.remove(widget);
}

list<Widget *> &UiSystem::widgets() {
    return m_uiComponents;
}

void UiSystem::composeComponent(Component *component) const {
    component->composeComponent();
}
