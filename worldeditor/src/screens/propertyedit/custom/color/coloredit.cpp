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

QVariant ColorEdit::data() const {
    return m_color;
}

void ColorEdit::setData(const QVariant &data) {
    m_color.setNamedColor(data.value<QColor>().name(QColor::HexArgb));
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
    painter.setBrush(m_color);
    painter.drawRect(r);
    r.setWidth(r.width() / 2);
    painter.setBrush(QColor(m_color.rgb()));
    painter.drawRect(r);
    painter.end();
}

void ColorEdit::mousePressEvent(QMouseEvent *event) {
    QColor color = QColorDialog::getColor(m_color, this, QString(), QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        m_color = color;
        emit editFinished();
    }
}
