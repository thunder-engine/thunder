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
#ifndef ACTIONS_H
#define ACTIONS_H

#include <editor/propertyedit.h>

#include <QMetaProperty>

class Object;

class PropertyEditor;

namespace Ui {
    class Actions;
}

class Actions : public PropertyEdit {
    Q_OBJECT

public:
    explicit Actions(QWidget *parent = nullptr);
    ~Actions();

    void setObject(Object *object, const TString &name) override;

    bool isChecked() const;

public slots:
    void onDataChanged(bool);

private:
    PropertyEditor *findEditor(QWidget *parent) const;

    Ui::Actions *ui;

    MetaProperty m_property;

};

#endif // ACTIONS_H
