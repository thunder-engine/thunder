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
    m_color = data.toVector4();
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
    painter.setBrush(QColor::fromRgbF(m_color.x, m_color.y, m_color.z, m_color.w));
    painter.drawRect(r);
    r.setWidth(r.width() / 2);
    painter.setBrush(QColor::fromRgbF(m_color.x, m_color.y, m_color.z));
    painter.drawRect(r);
    painter.end();
}

void ColorEdit::mousePressEvent(QMouseEvent *event) {
    QColor color = QColorDialog::getColor(QColor::fromRgbF(m_color.x, m_color.y, m_color.z, m_color.w), this, QString(), QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        m_color = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        emit editFinished();
    }
}
