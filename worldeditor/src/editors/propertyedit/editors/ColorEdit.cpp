#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>

#include "ColorEdit.h"

ColorEdit::ColorEdit(QWidget *parent) :
    QToolButton(parent) {

    connect(this, SIGNAL(clicked()), this, SLOT(colorPickDlg()));

    m_Brush = QBrush(QPixmap(":/Images/Cell.png").scaled(16, 16));
    setMaximumHeight(20);
}

QColor ColorEdit::color() const {
    return m_Color;
}

void ColorEdit::setColor(const QString &color) {
    m_Color.setNamedColor(color);
}

void ColorEdit::colorPickDlg() {
    QColor color = QColorDialog::getColor(m_Color, this, QString(), QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        m_Color = color;
        emit colorChanged(m_Color.name(QColor::HexArgb));
    }
}

void ColorEdit::paintEvent(QPaintEvent *ev) {
    QRect r = ev->rect();
    r.setTop(1);
    r.setSize(QSize(r.width() - 2, r.height() - 2));

    QPainter painter;
    painter.begin(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_Brush);
    painter.drawRect(r);
    r.setWidth(r.width() / 2);
    painter.setBrush(QColor(m_Color.rgb()));
    painter.drawRect(r);
    r.translate(r.width(), 0);
    painter.setBrush(m_Color);
    painter.drawRect(r);
    painter.end();
}
