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
#ifndef AXISESEDIT_H
#define AXISESEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class AxisesEdit;
}

class AxisesEdit : public PropertyEdit {
    Q_OBJECT

public:
    enum Axises {
        AXIS_X = (1<<0),
        AXIS_Y = (1<<1),
        AXIS_Z = (1<<2)
    };

public:
    explicit AxisesEdit(QWidget *parent = nullptr);
    ~AxisesEdit();

    Variant data() const override;
    void setData(const Variant &value) override;

private:
    Ui::AxisesEdit *ui;

};

#endif // AXISESEDIT_H
