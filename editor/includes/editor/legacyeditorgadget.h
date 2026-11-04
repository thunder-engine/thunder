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
#ifndef LEGACYEDITORGADGET_H
#define LEGACYEDITORGADGET_H

#include <QWidget>

#include <editor.h>

class LegacyAssetEditor;

class EDITOR_EXPORT LegacyEditorGadget : public QWidget {
    Q_OBJECT

public:
    explicit LegacyEditorGadget(QWidget *parent = nullptr);

    LegacyAssetEditor *currentEditor();
    virtual void setCurrentEditor(LegacyAssetEditor *editor);

    virtual Object::ObjectList selected();

    static TString propertyWithObjectTypes(const Object::ObjectList &objects, const TString &property);
    static Object::ObjectList objectsForProperty(const Object::ObjectList &objects, const TString &property, TString &name);

signals:
    void updated();

    void objectsSelected(bool force);
    void objectsChanged(const TString &property, Variant value);

public slots:
    virtual void onUpdated() = 0;

    virtual void onSelectionChanged() = 0;
    virtual void onObjectsChanged(const TString &property, Variant value) {}

protected:
    LegacyAssetEditor *m_currentEditor;

};

#endif // LEGACYEDITORGADGET_H
