#include "graph/viewers/resultview.h"

#include <QPainter>
#include <QMouseEvent>

#include <QDebug>

ResultView::ResultView(QWidget *parent):
        GraphWidget(parent) {

    pResult     = 0;
    pData       = 0;
    mWidth      = 0;
    mHeight     = 0;

    drawUV      = false;
}

ResultView::~ResultView() {
    delete []pData;
}

void ResultView::draw(QPainter &mPainter, const QRect &r) {
    mPainter.setBrush(QColor(96, 96, 96));
    mPainter.drawRect(r);

    mPainter.setPen(QColor(66, 66, 66));
    mPainter.setFont(QFont("Segoe", 128));
    mPainter.drawText(48, r.height() - 48, "Result");

    mPainter.scale(mTranslate.z, mTranslate.z);
    mPainter.translate(mTranslate.x, mTranslate.y);

    if(pData) {
        mPainter.drawImage(r.width() * 0.5 - mWidth * 0.5, r.height() * 0.5 - mHeight * 0.5, mImg);
    }

    if(drawUV) {
        mPainter.setPen(QColor(0, 255, 0));
        mPainter.setBrush(Qt::NoBrush);
        //for(uint32_t i = 0; i < mList.size(); i += 3) {
        //    QPolygonF polygon;
        //    polygon << QPointF(mList[i + 0].uv.x * mWidth, mHeight - mList[i + 0].uv.y * mHeight)
        //            << QPointF(mList[i + 1].uv.x * mWidth, mHeight - mList[i + 1].uv.y * mHeight)
        //            << QPointF(mList[i + 2].uv.x * mWidth, mHeight - mList[i + 2].uv.y * mHeight);
        //    mPainter.drawConvexPolygon(polygon);
        //}
    }
}

void ResultView::setResult(Vector4 *result, int w, int h) {
    pResult     = result;
    delete []pData;

    pData   = new uchar[w * h * 3];

    mImg    = QImage(pData, w, h, QImage::Format_RGB888);

    mWidth  = w;
    mHeight = h;
}

void ResultView::showUVGrid(bool visible) {
    drawUV  = visible;
}

void ResultView::onUpdateResult(const QRect &rect) {
    for(uint32_t y = 0; y < rect.height(); y++) {
        for(uint32_t x = 0; x < rect.width(); x++) {
            int pos = (rect.x() + x) + (rect.y() + y) * mWidth;
            Vector4 v = pResult[pos] * 255.0f;
            pos = (rect.x() + x) * 3 + (rect.y() + y) * mWidth * 3;
            memset(&pData[pos + 0], (uchar)CLAMP(v.x, 0.0f, 255.0f), sizeof(uchar));
            memset(&pData[pos + 1], (uchar)CLAMP(v.y, 0.0f, 255.0f), sizeof(uchar));
            memset(&pData[pos + 2], (uchar)CLAMP(v.z, 0.0f, 255.0f), sizeof(uchar));
          //memset(&pData[pos + 3], (uchar)CLAMP(255, 0.0f, 255.0f), sizeof(uchar));
        }
    }

    repaint();
}
