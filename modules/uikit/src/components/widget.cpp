#include "components/widget.h"

#include "components/recttransform.h"
#include "components/layout.h"

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
        m_transform(nullptr) {

}

Widget::~Widget() {
    if(m_transform) {
        m_transform->unsubscribe(this);
    }

    for(auto it : m_childWidgets) {
        it->m_parent = nullptr;
    }

    for(auto it : m_subWidgets) {
        it->m_parent = nullptr;
    }

    if(m_parent) {
        m_parent->m_childWidgets.remove(this);
        m_parent->m_subWidgets.remove(this);
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
        if(it->actor()->isEnabled() && it->isEnabled()) {
            it->draw(buffer);
        }
    }
}
/*!
    \internal
    Internal method called to draw the sub widget using the provided command buffer.
*/
void Widget::drawSub(CommandBuffer &buffer) {
    for(auto it : m_subWidgets) {
        if(it && it->actor()->isEnabled() && it->isEnabled()) {
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
    // Size
    bool pixels;
    Vector2 size = m_transform->size();

    size = styleBlock2Length("-uikit-size", size, pixels);

    size.x = styleLength("width", size.x, pixels);
    size.y = styleLength("height", size.y, pixels);

    m_transform->setSize(size);

    // Pivot point
    Vector2 pivot = m_transform->pivot();
    pivot = styleBlock2Length("-uikit-pivot", pivot, pixels);
    m_transform->setPivot(pivot);

    // Anchors
    Vector2 minAnchors = m_transform->minAnchors();
    minAnchors = styleBlock2Length("-uikit-min-anchors", minAnchors, pixels);
    m_transform->setMinAnchors(minAnchors);

    Vector2 maxAnchors = m_transform->maxAnchors();
    maxAnchors = styleBlock2Length("-uikit-max-anchors", maxAnchors, pixels);
    m_transform->setMaxAnchors(maxAnchors);

    // Border width
    Vector4 border(m_transform->border());
    border = styleBlock4Length("border-width", border, pixels);

    border.x = styleLength("border-top-width", border.x, pixels);
    border.y = styleLength("border-right-width", border.y, pixels);
    border.z = styleLength("border-bottom-width", border.z, pixels);
    border.w = styleLength("border-left-width", border.w, pixels);

    m_transform->setBorder(border);

    // Margins
    Vector4 margin(m_transform->margin());
    margin = styleBlock4Length("margin", margin, pixels);

    margin.x = styleLength("margin-top", margin.x, pixels);
    margin.y = styleLength("margin-right", margin.y, pixels);
    margin.z = styleLength("margin-bottom", margin.z, pixels);
    margin.w = styleLength("margin-left", margin.w, pixels);

    m_transform->setMargin(margin);

    // Padding
    Vector4 padding(m_transform->padding());
    padding = styleBlock4Length("padding", padding, pixels);

    padding.x = styleLength("padding-top", padding.x, pixels);
    padding.y = styleLength("padding-right", padding.y, pixels);
    padding.z = styleLength("padding-bottom", padding.z, pixels);
    padding.w = styleLength("padding-left", padding.w, pixels);

    m_transform->setPadding(padding);

    // Display
    auto it = m_styleRules.find("display");
    if(it != m_styleRules.end()) {
        TString layoutMode = it->second.second;
        if(layoutMode == "none") {
            actor()->setEnabled(false);
        } else {
            Layout *layout = m_transform->layout();
            if(layout == nullptr) {
                layout = new Layout;
            }

            if(layoutMode == "block") {
                layout->setOrientation(Vertical);
            } else if(layoutMode == "inline") {
                layout->setOrientation(Horizontal);
            }

            m_transform->setLayout(layout);

            for(auto it : m_childWidgets) {
                layout->addTransform(it->rectTransform());
            }
        }
    }

    for(auto it : m_childWidgets) {
        it->applyStyle();
    }

    for(auto it : m_subWidgets) {
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
RectTransform *Widget::rectTransform() const {
    return m_transform;
}
/*!
    Returns true if provided \a widget is a part of complex widget; otherwise returns false.
*/
bool Widget::isSubWidget(Widget *widget) const {
    for(auto it : m_subWidgets) {
        if(it == widget) {
            return true;
        }
    }
    return false;
}
/*!
    Returns the application widget that has the keyboard input focus, or nullptr if no widget in this application has the focus.
*/
Widget *Widget::focusWidget() {
    return m_focusWidget;
}

Widget *Widget::subWidget(const TString &name) const {
    for(auto it : m_subWidgets) {
        if(it->actor()->name() == name) {
            return it;
        }
    }
    return nullptr;
}

void Widget::setSubWidget(Widget *widget) {
    Widget *current = subWidget(widget->actor()->name());
    if(current != widget) {
        if(current) {
            m_subWidgets.remove(current);
            m_childWidgets.push_back(current);
        }

        if(widget) {
            m_subWidgets.push_back(widget);
            m_childWidgets.remove(widget);
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
    if(m_transform) {
        m_transform->unsubscribe(this);
    }

    m_transform = transform;
    if(m_transform) {
        m_transform->subscribe(this);
    }
}
/*!
    \internal
     Internal method to set the parent of the widget and handle changes.
*/
void Widget::setParent(Object *parent, int32_t position, bool force) {
    NativeBehaviour::setParent(parent, position, force);

    actorParentChanged();
}
/*!
    \internal
    Internal method called when the parent actor of the widget changes.
*/
void Widget::actorParentChanged() {
    Actor *object = actor();
    if(object) {
        setRectTransform(dynamic_cast<RectTransform *>(object->transform()));

        object = dynamic_cast<Actor *>(object->parent());
        if(object) {
            if(m_parent) {
                m_parent->m_childWidgets.remove(this);
            }

            m_parent = object->getComponent<Widget>();
            if(m_parent) {
                m_parent->m_childWidgets.push_back(this);
            }
        }
    }
}
/*!
    \internal
    Internal method to compose the widget component, creating and setting the RectTransform.
*/
void Widget::composeComponent() {
    Actor *a = actor();

    if(a) {
        Transform *transform = a->transform();

        RectTransform *rect = dynamic_cast<RectTransform *>(transform);
        if(rect == nullptr) {
            if(transform) {
                delete transform;
            }

            rect = Engine::objectCreate<RectTransform>("RectTransform", a);
            a->setTransform(rect);

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

    for(auto it : m_subWidgets) {
        it->addStyleRules(rules, weight);
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
