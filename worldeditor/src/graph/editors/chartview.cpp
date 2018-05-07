#include "chartview.h"

#include <QPainter>

ChartView::ChartView(QWidget *parent) :
    GraphWidget(parent) {

    mFontSize   = 14;
    mFont       = QFont("Helvetica Neue", mFontSize);
    mFontSmall  = QFont("Helvetica Neue", mFontSize / 2);

    mHeight     = 100;
    mCell       = mHeight / 5;
    mRow        = mCell + mCell / 1.62;
    mRound      = 8;
    mStep       = mRound * 2;

    mCompound   = 2;

    QColor color(3, 62, 154);

    mBackground = QLinearGradient(QPointF(0, 0), QPointF(0, mHeight));
    mBackground.setColorAt(0, color.lighter(220));
    mBackground.setColorAt(1, color);

    mChart      = QLinearGradient(QPointF(0, -mRow * 2), QPointF(0, 0));
    mChart.setColorAt(0, QColor(255, 255, 255, 100));
    mChart.setColorAt(1, QColor(255, 255, 255, 0));
}

void ChartView::draw(QPainter &painter, const QRect &r) {
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(96, 96, 96));
    painter.drawRect(r);

    painter.setClipping(true);
    painter.setClipRect(r);

    int x   = 0;
    int y   = 0;
    Data::const_iterator it = mData.constBegin();
    while (it != mData.constEnd()) {
        painter.resetTransform();
        int width   = (r.width() - mRound * (mCompound + 1)) / mCompound;
        int left    = (width + mRound) * x + mRound;

        QRect rect(left, (mHeight + mRound) * y + mRound, width, mHeight);
        drawChart(painter, rect, it);
		++it;
        x++;
        if(x == mCompound) {
            x   = 0;
            y++;
        }
    }
}

void ChartView::select(const QPoint &pos) {

}

void ChartView::addData(const QString &key, float data) {
    mData[key].push_front(data);
    QPointF range   = mRange[key];
    range.setX(qMin((float)range.x(), data));
    range.setY(qMax((float)range.y(), data));
    mRange[key]     = range;
}

void ChartView::setUnits(const QString &key, const QString &unit) {
    mUnits[key] = unit;
}

void ChartView::drawChart(QPainter &painter, const QRect &r, Data::const_iterator &it) {
    QString key = it.key();

    painter.translate(r.x(), r.y());

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(mBackground));
    painter.drawRoundedRect(QRect(0, 0, r.width(), r.height()), mRound, mRound);

    int width   = r.width() - mStep;
    int height  = mRow * 2;

    QRect text(mRound * 2, 0, width - mStep, mCell );

    painter.setFont(mFont);
    painter.setPen(QColor(255, 255, 255, 200));
    painter.drawText(text, Qt::AlignBottom, key);

    QString unit    = mUnits[key];
    painter.setFont(mFontSmall);
    painter.drawText(text, Qt::AlignRight | Qt::AlignBottom, unit);

    QFontMetrics info(mFontSmall);
    text.setWidth(text.width() - info.width(unit));
    float v = it.value().front();
    painter.setFont(mFont);
    painter.drawText(text, Qt::AlignRight | Qt::AlignBottom, QString::number(v));

    painter.setPen(QColor(255, 255, 255, 100));
    painter.drawLine(mStep, mCell, width, mCell);
    painter.setPen(QColor(255, 255, 255, 50));
    painter.drawLine(mStep, mCell + mRow, width, mCell + mRow);
    painter.setPen(QColor(255, 255, 255, 25));
    painter.drawLine(mStep, mCell + mRow * 2, width, mCell + mRow * 2);

    QPointF range   = mRange[key];

    text    = QRect(width - mCell, mCell, mCell, height);
    painter.setPen(QColor(255, 255, 255, 200));
    painter.setFont(mFontSmall);
    painter.drawText(text, Qt::AlignRight | Qt::AlignBottom,    QString::number(range.x()));
    painter.drawText(text, Qt::AlignRight | Qt::AlignTop,       QString::number(range.y()));

    int x   = width - mCell;

    painter.translate(x, height + mCell);

    int count       = r.width() / mStep;
    float delta     = range.y() - range.x();
    if(delta == 0.0f) {
        return;
    }

    QPainterPath path;

    float start = 0.0f;
    int i   = 0;
    foreach(const float &value, it.value()) {
        float p     = (value - range.x()) / delta;
        if(i == 0) {
            start   = -height * p;
            path.moveTo(0, start);
        } else {
            path.lineTo(-i * mStep, -height * p);
        }

        i++;
        if(i >= count) {
            break;
        }
    }

    QPainterPath fill;
    fill.lineTo(0, start);
    fill.connectPath(path);
    fill.lineTo(-(i - 1) * mStep,   0);

    painter.setClipRect(QRectF(-(x - mStep), -mRow * 2, x - mStep, mRow * 2));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(mChart));
    painter.drawPath(fill);

    painter.setPen(QColor(255, 255, 255, 255));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
    painter.setClipping(false);
}
