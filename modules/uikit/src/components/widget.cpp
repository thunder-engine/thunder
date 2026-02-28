#include "components/widget.h"

#include "components/recttransform.h"

#include "stylesheet.h"
#include "uisystem.h"

#include <components/actor.h>

Widget *Widget::m_focusWidget = nullptr;

/*!
    \module Gui

    \title Graphical User Interface for Thunder Engine SDK

    \brief Contains classes related to Graphical User Interfaces.
*/

/*!
    \class Widget
    \brief The Widget class is the base class of all user interface objects.
    \inmodule Gui

    The Widget class serves as the base class for all user interface objects, providing basic functionality for handling updates, drawing, and interaction.
    Internal methods are marked as internal and are intended for use within the framework rather than by external code.
*/

Widget::Widget() :
        m_parent(nullptr),
        m_transform(nullptr),
        m_subWidget(false) {

}

Widget::~Widget() {
    if(m_transform) {
        m_transform->unsubscribe(this);
    }

    for(auto it : m_childWidgets) {
        it->m_parent = nullptr;
    }

    if(m_parent) {
        m_parent->m_childWidgets.remove(this);
    }

    static_cast<UiSystem *>(system())->removeWidget(this);
}
/*!
    Returns a textual description of widget style.
*/
TString Widget::style() const {
    TString result;

    for(auto &it : m_styleRules) {
        if(it.second.first == 1000) {
            result += TString(it.first) + ": " + it.second.second + ";";
        }
    }

    return result;
}
/*!
    Returns a list of stylesheet class names attached to this widget.
*/
const StringList &Widget::classes() const {
    return m_classes;
}
/*!
    Adds a stylesheet class \a name attached to this widget.
*/
void Widget::addClass(const TString &name) {
    m_classes.push_back(name);
}
/*!
    \internal
    Internal method called to draw the widget using the provided command buffer.
*/
void Widget::draw(CommandBuffer &buffer) {
    drawSub(buffer);

    for(auto it : m_childWidgets) {
        if(!it->m_subWidget && it->actor()->isEnabled() && it->isEnabled()) {
            it->draw(buffer);
        }
    }
}
/*!
    \internal
    Internal method called to draw the sub widget using the provided command buffer.
*/
void Widget::drawSub(CommandBuffer &buffer) {
    for(auto it : m_childWidgets) {
        if(it->m_subWidget && it->actor()->isEnabled() && it->isEnabled()) {
            it->draw(buffer);
        }
    }
}
/*!
    Lowers the widget to the bottom of the widget's stack.

    \sa raise()
*/
void Widget::lower() {
    for(auto it : childWidgets()) {
        it->lower();
    }

    UiSystem::lowerWidget(this);
}
/*!
    Raises this widget to the top of the widget's stack.

    \sa lower()
*/
void Widget::raise() {
    UiSystem::riseWidget(this);

    for(auto it : childWidgets()) {
        it->raise();
    }
}
/*!
    Callback to respond to changes in the widget's \a size.
*/
void Widget::boundChanged(const Vector2 &size) {
    A_UNUSED(size);
}
/*!
    Applies style settings assigned to widget.
*/
void Widget::applyStyle() {
    blockSignals(true);
    if(m_transform) {
        m_transform->applyStyle();
    }
    blockSignals(false);

    for(auto it : m_childWidgets) {
        it->applyStyle();
    }
}
/*!
    Returns the parent Widget.
*/
Widget *Widget::parentWidget() const {
    return m_parent;
}
/*!
    Returns a list of child widgets;
*/
std::list<Widget *> &Widget::childWidgets() {
    return m_childWidgets;
}
/*!
    Returns RectTransform component attached to parent Actor.
*/
RectTransform *Widget::rectTransform() {
    if(m_transform == nullptr) {
        setRectTransform(dynamic_cast<RectTransform *>(transform()));
    }
    return m_transform;
}
/*!
    Returns true if provided \a widget is a part of complex widget; otherwise returns false.
*/
bool Widget::isSubWidget() const {
    return m_subWidget;
}
/*!
    Returns the application widget that has the keyboard input focus, or nullptr if no widget in this application has the focus.
*/
Widget *Widget::focusWidget() {
    return m_focusWidget;
}
/*!
    Return a sub widget (a part of more complex widget) with specified \a name.
*/
Widget *Widget::subWidget(const TString &name) const {
    for(auto it : m_childWidgets) {
        if(it->m_subWidget && it->actor()->name() == name) {
            return it;
        }
    }
    return nullptr;
}
/*!
    \internal
    Marks \a widget as a part of more complex widget.
*/
void Widget::setSubWidget(Widget *widget) {
    Widget *current = subWidget(widget->actor()->name());
    if(current != widget) {
        widget->m_subWidget = true;

        if(current) {
            current->m_subWidget = false;
        }
    }
}
/*!
    \internal
    Internal method to set the widget that has the keyboard input focus.
*/
void Widget::setFocusWidget(Widget *widget) {
    m_focusWidget = widget;
}
/*!
    \internal
    Internal method to set the RectTransform component for the widget.
*/
void Widget::setRectTransform(RectTransform *transform) {
    if(m_transform != transform) {
        if(m_transform) {
            m_transform->unsubscribe(this);
        }

        m_transform = transform;
        if(m_transform) {
            m_transform->subscribe(this);
            onHierarchyUpdated();
        }
    }
}
/*!
    \internal
    Internal method called when the parent actor of the widget changes.
*/
void Widget::onHierarchyUpdated() {
    Actor *object = actor();
    if(object) {
        m_childWidgets.clear();
        for(auto it : object->getChildren()) {
            Actor *childActor = dynamic_cast<Actor *>(it);
            if(childActor) {
                for(auto widgetIt : childActor->components("Widget")) {
                    Widget *widget = static_cast<Widget *>(widgetIt);
                    widget->m_parent = this;
                    m_childWidgets.push_back(widget);
                }
            }
        }
    }
}
/*!
    \internal
    Internal method to compose the widget component, creating and setting the RectTransform.
*/
void Widget::composeComponent() {
    Actor *object = Widget::actor();
    if(object) {
        Transform *transform = object->transform();

        RectTransform *rect = dynamic_cast<RectTransform *>(transform);
        if(rect == nullptr) {
            if(transform) {
                delete transform;
            }

            rect = Engine::objectCreate<RectTransform>("RectTransform", object);
            object->setTransform(rect);

            setRectTransform(rect);
        }
    }
}
/*!
    \internal
    Internal method to set the system for the widget, adding it to the render system.
*/
void Widget::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    UiSystem *render = static_cast<UiSystem *>(system);
    render->addWidget(this);
}
/*!
    \internal
    Applies a new stylesheet \a rules to the widget.
    A \a wieght parameter required to select rules between new one and existant.
*/
void Widget::addStyleRules(const std::map<TString, TString> &rules, uint32_t weight) {
    for(auto rule : rules) {
        auto it = m_styleRules.find(rule.first);
        if(it == m_styleRules.end() || it->second.first <= weight) {
            m_styleRules[rule.first] = std::make_pair(weight, rule.second);
        }
    }

    for(auto it : m_childWidgets) {
        if(it->m_subWidget) {
            it->addStyleRules(rules, weight);
        }
    }
}
/*!
    \internal
    Returns length for the stylesheet \a property.
    Default \a value will be used in case of property will not be found.
    Parameter \a pixels contains a definition of unit of measurement.
*/
float Widget::styleLength(const TString &property, float value, bool &pixels) {
    auto it = m_styleRules.find(property);
    if(it != m_styleRules.end()) {
        return StyleSheet::toLength(it->second.second, pixels);
    } else {
        pixels = true;
    }
    return value;
}
/*!
    \internal
    Returns length block for the stylesheet \a property.
    Default \a value will be used in case of property will not be found.
    Parameter \a pixels contains a definition of unit of measurement.
*/
Vector2 Widget::styleBlock2Length(const TString &property, const Vector2 &value, bool &pixels) {
    Vector2 result(value);

    auto it = m_styleRules.find(property);
    if(it != m_styleRules.end()) {
        auto list = it->second.second.split(' ');

        Vector2 value;
        if(list.size() == 1) {
            result.x = value.y = list.front().toFloat();
        } else {
            result.x = list.front().toFloat();
            result.y = list.back().toFloat();
        }
    }

    return result;
}
/*!
    \internal
    Returns length block for the stylesheet \a property.
    Default \a value will be used in case of property will not be found.
    Parameter \a pixels contains a definition of unit of measurement.
*/
Vector4 Widget::styleBlock4Length(const TString &property, const Vector4 &value, bool &pixels) {
    Vector4 result(value);

    auto it = m_styleRules.find(property);
    if(it != m_styleRules.end()) {
        auto array = it->second.second.split(' ');
        switch(array.size()) {
            case 1: {
                result = Vector4(StyleSheet::toLength(array.front(), pixels));
            } break;
            case 2: {
                result.x = result.z = StyleSheet::toLength(array.front(), pixels);
                result.y = result.w = StyleSheet::toLength(*std::next(array.begin(), 1), pixels);
            } break;
            case 3: {
                result.y = StyleSheet::toLength(array.front(), pixels);
                result.z = result.x = StyleSheet::toLength(*std::next(array.begin(), 1), pixels);
                result.w = StyleSheet::toLength(*std::next(array.begin(), 2), pixels);
            } break;
            case 4: {
                result.x = StyleSheet::toLength(array.front(), pixels);
                result.y = StyleSheet::toLength(*std::next(array.begin(), 1), pixels);
                result.z = StyleSheet::toLength(*std::next(array.begin(), 2), pixels);
                result.w = StyleSheet::toLength(*std::next(array.begin(), 3), pixels);
            } break;
            default: break;
        }
    }

    return result;
}

void Widget::updateStyleProperty(const TString &name, const float *v, int32_t size) {
    if(!isSignalsBlocked()) {
        TString data;
        for(uint32_t i = 0; i < size; i++) {
            data += TString::number((int)v[i]) + "px ";
        }
        data.removeLast();
        StyleSheet::setStyleProperty(this, name, data);
    }
}
