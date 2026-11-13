/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "propertyedit.h"

#include <label.h>
#include <layout.h>

#include <editor/pluginmanager.h>

#include <algorithm>
#include <cctype>

std::list<PropertyEdit::UserTypeCallback> PropertyEdit::m_userCallbacks;

PropertyEdit::PropertyEdit() :
        m_object(nullptr),
        m_mixedValue(false),
        m_label(nullptr),
        m_editorLayout(nullptr) {

    setName("PropertyEditor");
    setHorizontalPolicy(Widget::Expanding);
    setVerticalPolicy(Widget::Preferred);
    setSize(Vector2(0.0f, 28.0f));

    m_editorLayout = new Layout;
    m_editorLayout->setOrientation(Widget::Horizontal);
    m_editorLayout->setSpacing(4);
    setLayout(m_editorLayout);

    m_label = Widget::createWidget<Label>("PropertyName", this);
    if(m_label) {
        m_label->setSize(Vector2(144.0f, 24.0f));
        m_label->setAlign(Alignment::Right | Alignment::Middle);
        m_label->setHorizontalPolicy(Widget::Fixed);
        m_label->setVerticalPolicy(Widget::Fixed);
        m_editorLayout->addWidget(m_label);
    }
}

PropertyEdit::~PropertyEdit() = default;

Variant PropertyEdit::data() const {
    return Variant();
}

void PropertyEdit::setData(const Variant &data) {
    A_UNUSED(data);
}

void PropertyEdit::setEditorHint(const TString &hint) {
    A_UNUSED(hint);
}

void PropertyEdit::setObject(Object *object, const TString &property) {
    m_object = object;
    m_property = property;
    m_objects.clear();
    if(object) {
        m_objects.push_back(object);
    }
    if(m_label) {
        m_label->setText(propertyDisplayName(property));
        m_label->setVisible(!property.isEmpty());
    }
}

void PropertyEdit::setObjects(const Object::ObjectList &objects, const TString &property) {
    setObject(objects.empty() ? nullptr : objects.front(), property);
    m_objects = objects;
}

void PropertyEdit::setMixedValue(bool mixed) {
    m_mixedValue = mixed;
}

bool PropertyEdit::setSelectedObject(Object *object) {
    A_UNUSED(object);
    return false;
}

TString PropertyEdit::propertyName() const {
    return m_property;
}

Object *PropertyEdit::editedObject() const {
    return m_object;
}

const Object::ObjectList &PropertyEdit::editedObjects() const {
    return m_objects;
}

void PropertyEdit::dataChanged() {
    emitSignal(_SIGNAL(dataChanged()));
}

void PropertyEdit::editFinished() {
    emitSignal(_SIGNAL(editFinished()));
}

void PropertyEdit::selectionRequested() {
    emitSignal(_SIGNAL(selectionRequested()));
}

void PropertyEdit::registerEditorFactory(UserTypeCallback callback) {
    if(callback && std::find(m_userCallbacks.begin(), m_userCallbacks.end(), callback) == m_userCallbacks.end()) {
        m_userCallbacks.push_back(callback);
    }
}

void PropertyEdit::unregisterEditorFactory(UserTypeCallback callback) {
    m_userCallbacks.remove(callback);
}

PropertyEdit *PropertyEdit::constructEditor(uint32_t userType, Widget *parent, const TString &editor) {
    A_UNUSED(parent);
    for(UserTypeCallback callback : m_userCallbacks) {
        if(PropertyEdit *result = callback(userType, parent, editor)) {
            return result;
        }
    }

    for(auto &it : Editor::plugins()->extensions("property")) {
        if(it == editor) {
            PropertyEdit *edit = reinterpret_cast<PropertyEdit *>(Editor::plugins()->getPluginObject(it));
            if(edit) {
                return edit;
            }
        }
    }

    return nullptr;
}

void PropertyEdit::addEditor(Widget *editor) {
    if(m_editorLayout && editor) {
        editor->setHorizontalPolicy(Widget::Expanding);
        editor->setVerticalPolicy(Widget::Fixed);
        m_editorLayout->addWidget(editor);
    }
}

Layout *PropertyEdit::editorLayout() const {
    return m_editorLayout;
}

bool PropertyEdit::parseFloat(const TString &text, float &value) {
    TString input = text.trimmed();
    if(input.isEmpty()) {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    value = std::strtof(input.data(), &end);
    return end != input.data() && *end == '\0' && errno != ERANGE && std::isfinite(value);
}

bool PropertyEdit::parseInteger(const TString &text, int &value) {
    TString input = text.trimmed();
    if(input.isEmpty()) {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    const long parsed = std::strtol(input.data(), &end, 10);
    if(end == input.data() || *end != '\0' || errno == ERANGE ||
        parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

TString PropertyEdit::propertyDisplayName(const TString &name) {
    TString result = name;
    result.replace('_', ' ');
    if(!result.isEmpty()) {
        result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result.front())));
    }
    return result;
}
