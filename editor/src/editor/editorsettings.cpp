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
#include "editorsettings.h"

#include <QCoreApplication>
#include <QTranslator>

#include <log.h>

namespace {
    const char *gEditorSettings("EditorSettings");
}

/*!
    \class EditorSettings
    \brief Stores and manages editor-specific settings and translations.
    \inmodule Editor

    \fn void EditorSettings::updated()

    Emitted when an editor setting changes.
*/

EditorSettings::EditorSettings() :
        m_translator(new QTranslator()) {

}

/*!
    Registers a setting named \a name with its initial \a value and optional
    editor \a annotation.
*/
void EditorSettings::registerValue(const TString &name, const Variant &value, const TString &annotation) {
    blockSignals(true);
    setProperty(name.data(), value);
    setDynamicPropertyInfo(name.data(), annotation.data());
    blockSignals(false);
}

/*!
    Returns the value of the setting named \a name.
*/
Variant EditorSettings::value(const TString &name) {
    return property(name.data());
}

/*!
    Sets the \a value of the setting named \a name.
*/
void EditorSettings::setValue(const TString &name, const Variant &value) {
    Variant current = EditorSettings::value(name);
    if(current != value) {
        setProperty(name.data(), value);
    }
}

void EditorSettings::loadSettings() {
    blockSignals(true);

    if(Engine::value(gEditorSettings).isValid()) {
        VariantMap data = Engine::value(gEditorSettings).toMap();

        for(const TString &name : dynamicPropertyNames()) {
            auto it = data.find(name);
            if(it != data.end()) {
                setProperty(name.data(), it->second);
            }
        }
    } else {
        aWarning() << "Unable to load settings";
    }

    blockSignals(false);
}

void EditorSettings::saveSettings() {
    if(isSignalsBlocked()) {
        return;
    }

    VariantMap data;
    for(const TString &it : dynamicPropertyNames()) {
        data[it] = property(it.data());
    }

    Engine::setValue(gEditorSettings, data);
    Engine::syncValues();
}

/*!
    Changes the interface language to \a locale and reloads translations.
*/
void EditorSettings::setLanguage(const QLocale &locale) {
    if(m_translator && m_locale != locale) {
        m_locale = locale;
        QCoreApplication::removeTranslator(m_translator);
        m_translator->load(locale, QString(), QString(), ":/Translations");
        QCoreApplication::installTranslator(m_translator);
    }
}
    /*!
        Loads all registered settings from the engine configuration.
    */

/*!
    Sets a \a value for the property with \a name and emits updated() after persistence.
*/
void EditorSettings::setProperty(const char *name, const Variant &value) {
    Object::setProperty(name, value);

    TString editor = propertyTag(dynamicPropertyInfo(name), "editor=");
/*!
    Saves all registered settings to the engine configuration.
*/
    if(editor == "Locale") {
        setLanguage(QLocale(value.toString().data()));
    }

    saveSettings();
    updated();
}

void EditorSettings::updated() {
    emitSignal(_SIGNAL(updated()));
}

TString EditorSettings::propertyTag(const TString &hint, const TString &tag) const {
    StringList list(hint.split(','));
    for(TString it : list) {
        int index = it.indexOf(tag);
        if(index > -1) {
            return it.remove(tag);
        }
    }
    return TString();
}
