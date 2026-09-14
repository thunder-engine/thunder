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
#ifndef ARRAYEDIT_H
#define ARRAYEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class ArrayEdit;
}

class ArrayElement;

class ArrayEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit ArrayEdit(QWidget *parent = nullptr);
    ~ArrayEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

    void setObject(Object *object, const TString &name) override;

protected:
    void addOne();

private slots:
    void onAddItem();
    void onRemoveItem();
    void onCountChanged();

    void onDataChanged();
    void onEditFinished();
    void onDeleteElement();

private:
    Ui::ArrayEdit *ui;

    VariantList m_list;

    std::list<ArrayElement *> m_editors;

    TString m_typeName;
    TString m_propertyName;
    TString m_editorName;

    int m_height;

    int m_metaType;

    bool m_dynamic;

};

#endif // ARRAYEDIT_H
