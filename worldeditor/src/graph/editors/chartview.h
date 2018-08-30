#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include "graphwidget.h"

#include <QMap>

class ChartView : public GraphWidget {
    Q_OBJECT

    typedef QList<float>            List;
    typedef QMap<QString, List>     Data;
    typedef QMap<QString, QString>  Units;
    typedef QMap<QString, QPointF>  Range;

public:
    explicit ChartView      (QWidget *parent = 0);

    void                    draw                (QPainter &painter, const QRect &r);
    void                    select              (const QPoint &pos);

    void                    addData             (const QString &key, float data);
    void                    setUnits            (const QString &key, const QString &unit);

protected:
    void                    drawChart           (QPainter &painter, const QRect &r, Data::const_iterator &it);

private:
    Data                    mData;
    Units                   mUnits;
    Range                   mRange;

    int                     mFontSize;
    int                     mHeight;
    int                     mCell;
    int                     mRow;
    int                     mRound;
    int                     mStep;

    int                     mCompound;

    QFont                   mFont;
    QFont                   mFontSmall;

    QLinearGradient         mBackground;
    QLinearGradient         mChart;

};

#endif // CHARTVIEW_H
