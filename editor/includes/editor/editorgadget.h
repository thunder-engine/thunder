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
#ifndef EDITORGADGET_H
#define EDITORGADGET_H

#include <QWidget>

#include <editor.h>

class AssetEditor;

class EDITOR_EXPORT EditorGadget : public QWidget {
    Q_OBJECT

public:
    explicit EditorGadget(QWidget *parent = nullptr);

    virtual void setCurrentEditor(AssetEditor *editor) {};

signals:
    void updated();

    void objectsSelected(const Object::ObjectList &objects, bool force);
    void objectsChanged(const Object::ObjectList &objects, const TString &property, Variant value);

public slots:
    virtual void onUpdated() = 0;

    virtual void onSelectionChanged() = 0;
    virtual void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) = 0;

};

#endif // EDITORGADGET_H
