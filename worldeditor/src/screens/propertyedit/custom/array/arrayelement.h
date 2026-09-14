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
#ifndef ARRAYELEMENT_H
#define ARRAYELEMENT_H

#include <QWidget>
#include <astring.h>
#include <variant.h>

class PropertyEdit;

namespace Ui {
    class ArrayElement;
}

class ArrayElement : public QWidget {
    Q_OBJECT

public:
    explicit ArrayElement(QWidget *parent = nullptr);
    ~ArrayElement();

    Variant data() const;
    void setData(int index, const Variant &data, const TString &editor, Object *object, const TString &typeName);

    int32_t index() const;

signals:
    void dataChanged();
    void editFinished();
    void deleteElement();

private:
    Ui::ArrayElement *ui;

    TString m_tag;

    PropertyEdit *m_editor;

    int32_t m_index;

};

#endif // ARRAYELEMENT_H
