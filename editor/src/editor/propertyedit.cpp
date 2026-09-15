/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "editor/propertyedit.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

std::list<PropertyEdit::UserTypeCallback> PropertyEdit::m_userCallbacks;

/*!
    \class PropertyEdit
    \brief Provides a widget for editing object properties in the editor.
    \inmodule Editor

    \fn void PropertyEdit::dataChanged()

    Emitted when the edited data changes.

    \fn void PropertyEdit::editFinished()

    Emitted when editing is finished.
*/

/*!
    Constructs a property editor with the specified Qt \a parent.
*/
PropertyEdit::PropertyEdit(QWidget *parent) :
        QWidget(parent),
    m_object(nullptr),
    m_mixedValue(false) {

}

PropertyEdit::~PropertyEdit() {

}

/*!
    Returns the value currently displayed by the editor.
*/
Variant PropertyEdit::data() const {
    return Variant();
}

/*!
    Sets the value displayed by the editor.
*/
void PropertyEdit::setData(const Variant &data) {
    A_UNUSED(data);
}

/*!
    Sets whether the editor displays a mixed value from multiple objects.
*/
void PropertyEdit::setMixedValue(bool mixed) {
    m_mixedValue = mixed;
    if(!mixed) {
        return;
    }

    for(QLineEdit *line : findChildren<QLineEdit *>()) {
        line->clear();
        line->setPlaceholderText("--");
    }
    for(QCheckBox *check : findChildren<QCheckBox *>()) {
        check->setTristate(true);
        check->setCheckState(Qt::PartiallyChecked);
    }
    for(QComboBox *combo : findChildren<QComboBox *>()) {
        combo->setCurrentIndex(-1);
    }
    for(QAbstractButton *button : findChildren<QAbstractButton *>()) {
        if(button->isCheckable() && !qobject_cast<QCheckBox *>(button)) {
            button->setChecked(false);
        }
    }
}

/*!
    Returns true if the editor currently displays a mixed value.
*/
bool PropertyEdit::isMixedValue() const {
    return m_mixedValue;
}

/*!
    Applies an editor hint that controls the property editor presentation.
*/
void PropertyEdit::setEditorHint(const TString &hint) {
    A_UNUSED(hint);
}

/*!
    Assigns the object and property edited by this widget.
*/
void PropertyEdit::setObject(Object *object, const TString &property) {
    A_UNUSED(property);
    m_object = object;
}

/*!
    Registers a factory \a callback for custom user-type editors.
*/
void PropertyEdit::registerEditorFactory(UserTypeCallback callback) {
    m_userCallbacks.push_back(callback);
}

/*!
    Unregisters a previously registered custom editor factory \a callback.
*/
void PropertyEdit::unregisterEditorFactory(UserTypeCallback callback) {
    m_userCallbacks.remove(callback);
}

/*!
    Constructs a custom \a editor for a user \a type, or returns nullptr when none
    of the registered factories accepts it.
*/
PropertyEdit *PropertyEdit::constructEditor(int type, QWidget *parent, const TString &editor) {
    PropertyEdit *result = nullptr;
    if(!m_userCallbacks.empty()) {
        auto iter = m_userCallbacks.begin();
        while(result == nullptr && iter != m_userCallbacks.end() ) {
            result = (*iter)(type, parent, editor);
            ++iter;
        }
    }
    return result;
}
