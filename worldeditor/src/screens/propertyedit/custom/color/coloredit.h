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
#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <editor/propertyedit.h>

class ColorEdit : public PropertyEdit {
    Q_OBJECT
public:
    explicit ColorEdit(QWidget *parent = nullptr);

    Variant data() const override;
    void setData(const Variant &data) override;
    void setMixedValue(bool mixed) override;

private:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    Vector4 m_color;
    bool m_mixed = false;

    QBrush m_brush;

};

#endif // COLOREDIT_H
