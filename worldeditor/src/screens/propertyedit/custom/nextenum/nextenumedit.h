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
#ifndef NEXTENUMEDIT_H
#define NEXTENUMEDIT_H

#include <editor/propertyedit.h>

#include <metaenum.h>

namespace Ui {
    class NextEnumEdit;
}

class NextEnumEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit NextEnumEdit(QWidget *parent = nullptr);
    ~NextEnumEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

    void setEnumData(const TString &name, Object *object);

private slots:
    void onValueChanged(int item);

private:
    Ui::NextEnumEdit *ui;

    TString m_enumName;

    Object *m_object;

    int32_t m_value;

    MetaEnum m_metaEnum;

};

#endif // NEXTENUMEDIT_H
