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
#ifndef EDITORSETTINGS_H
#define EDITORSETTINGS_H

#include <QLocale>

#include <editor.h>

class QTranslator;

class EDITOR_EXPORT EditorSettings : public Object {
    A_OBJECT(EditorSettings, Object, Editor)

    A_METHODS(
        A_SIGNAL(EditorSettings::updated)
    )

public:
    EditorSettings();

    void registerValue(const TString &name, const Variant &value, const TString &annotation = TString());

    Variant value(const TString &name);
    void setValue(const TString &name, const Variant &value);

    void setLanguage(const QLocale &language);

    void setProperty(const char *name, const Variant &value) override;

signals:
    void updated();

public slots:
    void loadSettings();
    void saveSettings();

private:
    TString propertyTag(const TString &hint, const TString &tag) const;

private:
    static EditorSettings *m_pInstance;

    QLocale m_locale;

    QTranslator *m_translator;

};

#endif // EDITORSETTINGS_H
