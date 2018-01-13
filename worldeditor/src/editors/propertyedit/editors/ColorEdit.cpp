#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>

#include "ColorEdit.h"

ColorEdit::ColorEdit(QWidget *parent) :
    QToolButton(parent) {

    connect(this, SIGNAL(clicked()), this, SLOT(colorPickDlg()));

    mBrush  = QBrush(QPixmap(":/Images/Cell.png").scaled(16, 16));
}

QColor ColorEdit::color() const {
    return mColor;
}

void ColorEdit::setColor(const QString &color) {
    mColor.setNamedColor(color);
}

void ColorEdit::colorPickDlg() {
    QColor color    = QColorDialog::getColor(mColor, this, QString(), QColorDialog::ShowAlphaChannel);
    if(color.isValid()) {
        mColor      = color;
        emit colorChanged(mColor.name(QColor::HexArgb));
    }
}

void ColorEdit::paintEvent(QPaintEvent *ev) {
    QRect r = ev->rect();
    r.setTop(1);
    r.setSize(QSize(r.width() - 2, r.height() - 2));

    QPainter painter;
    painter.begin(this);
    painter.setBrush(mBrush);
    painter.drawRect(r);
    r.setWidth(r.width() / 2);
    painter.setBrush(QColor(mColor.rgb()));
    painter.drawRect(r);
    r.translate(r.width(), 0);
    painter.setBrush(mColor);
    painter.drawRect(r);
    painter.end();
}
