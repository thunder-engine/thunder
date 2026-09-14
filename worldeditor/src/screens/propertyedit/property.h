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
#ifndef PROPERTY_H
#define PROPERTY_H

#include <QVariant>

#include <engine.h>

class PropertyEdit;

class Property : public QObject {
    Q_OBJECT

public:
    explicit Property(const TString &name, Property *parent, bool root);

    void setPropertyObject(Object *propertyObject);
    void setPropertyObjects(const Object::ObjectList &propertyObjects);

    TString name() const;

    TString editorHints() const;
    void setEditorHints(const TString &hints);

    QWidget *getEditor(QWidget *parent) const;
    PropertyEdit *editor() const;

    bool isRoot() const;
    bool isReadOnly() const;

    Variant value() const;
    void setValue(const Variant &value);

    void updateEditor();

    QSize sizeHint(const QSize &size) const;

    bool isCheckable() const;
    bool isChecked() const;
    void setChecked(bool value);

    static TString editorName(const TString &hints, const TString &typeName);

    static TString propertyTag(const TString &hints, const TString &tag);
    static bool hasTag(const TString &hints, const TString &tag);

    static void trimmType(TString &type, bool &isArray);

signals:
    void propertyChanged(const Object::ObjectList &objects, const TString &property, Variant value);

protected slots:
    void onDataChanged();
    void onEditorDestoyed();

protected:
    PropertyEdit *createEditor(QWidget *parent) const;

protected:
    Object *m_nextObject;
    Object::ObjectList m_nextObjects;

    TString m_hints;
    TString m_name;

    mutable TString m_typeNameTrimmed;

    mutable PropertyEdit *m_editor;

    bool m_root;
    bool m_checkable;

    bool m_readOnly;
};

#endif // PROPERTY_H
