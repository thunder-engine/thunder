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
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>

#include "coloredit.h"

ColorEdit::ColorEdit(QWidget *parent) :
        PropertyEdit(parent),
        m_brush(QBrush(QPixmap(":/Images/Cell.png").scaled(16, 16))) {

    setMaximumHeight(20);
    setMinimumHeight(20);
}

Variant ColorEdit::data() const {
    return m_color;
}

void ColorEdit::setData(const Variant &data) {
    m_mixed = false;
    m_color = data.toVector4();
}

void ColorEdit::setMixedValue(bool mixed) {
    PropertyEdit::setMixedValue(mixed);
    m_mixed = mixed;
    if(mixed) {
        update();
    }
}

void ColorEdit::paintEvent(QPaintEvent *ev) {
    QRect r = ev->rect();
    r.setTop(1);
    r.setSize(QSize(r.width() - 2, r.height() - 2));

    QPainter painter;
    painter.begin(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_brush);
    painter.drawRect(r);
    if(!m_mixed) {
        painter.setBrush(QColor::fromRgbF(m_color.x, m_color.y, m_color.z, m_color.w));
        painter.drawRect(r);
        r.setWidth(r.width() / 2);
        painter.setBrush(QColor::fromRgbF(m_color.x, m_color.y, m_color.z));
        painter.drawRect(r);
    }
    painter.end();
}

void ColorEdit::mousePressEvent(QMouseEvent *event) {
    QColor color = QColorDialog::getColor(QColor::fromRgbF(m_color.x, m_color.y, m_color.z, m_color.w), this, QString(), QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        m_color = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        emit editFinished();
    }
}
