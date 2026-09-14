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

EditorSettings::EditorSettings() :
        m_translator(new QTranslator()) {

}

void EditorSettings::registerValue(const TString &name, const Variant &value, const TString &annotation) {
    blockSignals(true);
    setProperty(name.data(), value);
    setDynamicPropertyInfo(name.data(), annotation.data());
    blockSignals(false);
}

Variant EditorSettings::value(const TString &name) {
    return property(name.data());
}

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

void EditorSettings::setLanguage(const QLocale &locale) {
    if(m_translator && m_locale != locale) {
        m_locale = locale;
        QCoreApplication::removeTranslator(m_translator);
        m_translator->load(locale, QString(), QString(), ":/Translations");
        QCoreApplication::installTranslator(m_translator);
    }
}

void EditorSettings::setProperty(const char *name, const Variant &value) {
    Object::setProperty(name, value);

    TString editor = propertyTag(dynamicPropertyInfo(name), "editor=");
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
