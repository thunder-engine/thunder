#include "resultview.h"

#include <QPainter>
#include <QImage>
#include <QWheelEvent>

ResultView::ResultView(QWidget *parent):
        GraphWidget(parent) {
    mCaption    = "Result";

    mBrush      = QBrush(QPixmap(":/Images/Cell.png").scaled(32, 32));

    mZoom       = 1.0f;
}

void ResultView::draw(QPainter &painter, const QRect &r) {
    painter.setBrush(QColor(96, 96, 96));
    painter.drawRect(r);

    painter.setPen(QColor(66, 66, 66));
    painter.setFont(QFont("Segoe", 128));
    painter.drawText(48, r.height() - 48, mCaption);

    painter.setBrush(mBrush);

    QRect rect(r.center() - mImage.rect().center() * mZoom,
               mImage.size() * mZoom);

    painter.drawRect(rect);
    painter.drawImage(rect, mImage);
}

void ResultView::setResult(const QImage &image) {
    mImage  = image;

    setMinimumSize(mImage.size() * mZoom);
    repaint();
}

void ResultView::setCaption(const QString &string) {
    mCaption    = string;
}

void ResultView::wheelEvent( QWheelEvent *pe ) {
    float s     = mZoom;
    float delta = (float)pe->delta() / 1000.0f;
    mZoom      += delta;
    if(mZoom < 0) {
        mZoom   = s;
    }

    setMinimumWidth( mImage.width() * mZoom );
    setMinimumHeight( mImage.height() * mZoom );

    GraphWidget::wheelEvent(pe);
}
