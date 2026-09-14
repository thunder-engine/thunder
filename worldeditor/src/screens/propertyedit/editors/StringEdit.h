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
#ifndef STRINGEDIT_H
#define STRINGEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class StringEdit;
}

class StringEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit StringEdit(QWidget *parent = nullptr);
    ~StringEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::StringEdit *ui;

};

#endif // STRINGEDIT_H
