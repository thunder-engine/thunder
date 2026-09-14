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
#ifndef ALIGNMENTEDIT_H
#define ALIGNMENTEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class AlignmentEdit;
}

class AlignmentEdit : public PropertyEdit {
    Q_OBJECT

public:
    enum Alignment {
        Left    = (1<<0),
        Center  = (1<<1),
        Right   = (1<<2),

        Top     = (1<<4),
        Middle  = (1<<5),
        Bottom  = (1<<6)
    };

public:
    explicit AlignmentEdit(QWidget *parent = nullptr);
    ~AlignmentEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

private:
    void onToggle();

    Ui::AlignmentEdit *ui;

};

#endif // ALIGNMENTEDIT_H
