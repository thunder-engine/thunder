#include <QPainter>
#include <QMouseEvent>

#include "curveeditor.h"

CurveEditor::CurveEditor(QWidget *parent) :
        GraphWidget(parent) {

    control   = CONTROL_NONE;
    add       = false;
    remove    = false;

    cell      = 32;
    m_Step    = QPointF(0.1f, 1.0f);

    focus_p   = -1;
    focus_v   = -1;

    select_p  = -1;
    select_v  = -1;

    //m_pCurve  = 0;

    m_Color   = QColor(rand()%255, rand()%255, rand()%255);

    setFocusPolicy(Qt::StrongFocus);
}

QPoint &CurveEditor::translate() {
    return m_Translate;
}

void CurveEditor::setTranslate(QPoint &value) {
    m_Translate = value;
}

QPointF &CurveEditor::step() {
    return m_Step;
}

void CurveEditor::setStep(QPointF &value) {
    m_Step      = value;
}

void CurveEditor::draw(QPainter &mPainter, const QRect &r) {
    mPainter.setRenderHint(QPainter::Antialiasing);
    mPainter.setBrush(QColor(96, 96, 96));
    mPainter.drawRect(r);

    mPainter.setPen(QColor(140, 140, 140));
    for(int i = 0; i < r.height(); i+=cell ) {
        int y = m_Translate.y() % cell;
        mPainter.drawLine(0, i + y, r.width(), i + y);
    }

    for(int i = 0; i < r.width(); i+=cell ) {
        int x = m_Translate.x() % cell;
        mPainter.drawLine(i + x, 0, i + x, r.height());
    }
    // Axises
    mPainter.setPen(QColor(255, 255, 255));

    mPainter.translate(0.0f, (float)m_Translate.y());
    mPainter.drawLine(0, 0, r.width(), 0);
    mPainter.resetTransform();

    mPainter.translate((float)m_Translate.x(), 0.0f);
    mPainter.drawLine(0, 0, 0, r.height());
    mPainter.resetTransform();

    // Curves
    mPainter.translate((float)m_Translate.x(), (float)m_Translate.y());

    mPainter.setBrush(QColor(255, 255, 255, 255));
    mPainter.setPen( m_Color );
    mPainter.drawPath( m_Path );

    int x   = 0;
    int y   = 0;
    QString str;
    // Points
    //int v = 0;
    //int p = 0;
    for(int i = 0; i < m_Path.elementCount(); i++) {
        QPainterPath::Element mElement;
        mElement    = m_Path.elementAt(i);
        if(mElement.type == QPainterPath::CurveToElement) {
            i += 2;
        }
        mElement    = m_Path.elementAt(i);
/*
        if(pRange) {
            if(p == 2) {
                p   = 0;
                v++;
                if(v == 2)
                    v   = 0;
            }

            if(p != 1) {
                mPainter.setBrush( Qt::GlobalColor(Qt::red + v) );
                if(focus_v == v && focus_p == p) {
                    x   = mElement.x + 10;
                    y   = mElement.y - 10;
                    if(v == 0)
                        str = QString("min = %1").arg(pRange->mMin);
                    else
                        str = QString("max = %1").arg(pRange->mMax);
                    mPainter.setBrush(Qt::yellow);
                }
                mPainter.setPen( Qt::GlobalColor(Qt::red + v) );
                mPainter.drawRect( mElement.x - 3, mElement.y - 3, 6, 6 );
            }
        }
*/
/*
        if(m_pCurve) {
            if(p == m_pCurve->mList.size() + 1) {
                p   = 0;
                v++;
                if(v == 3)
                    v   = 0;
            }

            if(p != m_pCurve->mList.size()) {
                mPainter.setBrush( Qt::GlobalColor(Qt::red + v) );
                if(focus_v == v && focus_p == p) {
                    x   = mElement.x + 10;
                    y   = mElement.y - 10;
                    str = QString("(%1; %2)").arg(m_pCurve->mList[p].mX).arg(m_pCurve->mList[p].mY[v]);
                    mPainter.setBrush(Qt::yellow);
                }
                mPainter.setPen( Qt::GlobalColor(Qt::red + v) );
                mPainter.drawRect( mElement.x - 3, mElement.y - 3, 6, 6 );
            }
        }
        p++;
*/
    }
    // Info
    mPainter.setPen(Qt::white);
    mPainter.drawText(x, y, str);

    mPainter.resetTransform();

    mPainter.setPen(QColor(140, 140, 140));
    for(int i = 0; i < r.height(); i+=cell ) {
        int y = m_Translate.y() % cell;
        QString str;
        str.sprintf( "%.2f", ((m_Translate.y() - y) - i) / (cell / m_Step.y()) );
        mPainter.drawText(2, i + y - 2, str);
    }

    for(int i = 0; i < r.width(); i+=cell ) {
        int x = m_Translate.x() % cell;
        QString str;
        str.sprintf( "%.2f", (i - (m_Translate.x() - x)) / (cell / m_Step.x()) );
        mPainter.drawText(i + x + 2, r.height() - 2, str);
    }
}

void CurveEditor::select(const QPoint &pos) {
    if( (control & CONTROL_POINT) == false ) {
        focus_p = -1;
        focus_v = -1;

        //uint32_t p  = 0;
        //uint32_t v  = 0;
        for(uint32_t i = 0; i < m_Path.elementCount(); i++) {
            QPainterPath::Element mElement;
            mElement    = m_Path.elementAt(i);
            if(mElement.type == QPainterPath::CurveToElement)
                i += 2;
            mElement    = m_Path.elementAt(i);
/*
            QRect r( m_Translate.x() + mElement.x - 3, m_Translate.y() + mElement.y - 3, 6, 6 );

            if(pRange) {
                if(p == 2) {
                    p   = 0;
                    v++;
                    if(v == 2)
                        v   = 0;
                }

                if(p != 1) {
                    if(r.contains(pos)) {
                        focus_p = p;
                        focus_v = v;
                    }
                }
            }

            if(pRange3D) {
                if(p == 2) {
                    p   = 0;
                    v++;
                    if(v == 6)
                        v   = 0;
                }

                if(p != 1) {
                    if(r.contains(pos)) {
                        focus_p = p;
                        focus_v = v;
                    }
                }
            }
*/
/*
            if(m_pCurve) {
                if(p == m_pCurve->mList.size() + 1) {
                    p   = 0;
                    v++;
                    if(v == 3)
                        v   = 0;
                }

                if(p != m_pCurve->mList.size()) {
                    if(r.contains(pos)) {
                        focus_p = p;
                        focus_v = v;
                    }
                }
            }

            p++;
*/
        }
    }
}
/*
void CurveEditor::set_curve(ACurve *value) {
    m_pCurve    = value;

    update();
}
*/
void CurveEditor::update() {
    m_Path  = QPainterPath();
    /*
    if(pRange) {
        QPointF p;
        p       = QPointF(0.0f, -pRange->mMin / m_Step.y() * cell);
        path.moveTo(p);
        path.lineTo(m_Size.width() - m_Translate.x(), p.y());

        p       = QPointF(0.0f, -pRange->mMax / m_Step.y() * cell);
        path.moveTo(p);
        path.lineTo(m_Size.width() - m_Translate.x(), p.y());
    }
    */
/*
    if(m_pCurve && m_pCurve->mList.size() > 0) {
        for(int l = 0; l < 3; l++) {
            QPointF p(m_pCurve->mList[0].mX, -m_pCurve->mList[0].mY[l] / m_Step.y() * cell);
            m_Path.moveTo(p);

            QPointF p2;
            for(uint32_t i = 0; i < m_pCurve->mList.size(); i++) {
                int o       = i - 1;
                QPointF s1  = p;
                float d     = 0;

                ACurvePoint *pi = &m_pCurve->mList[i];
                if( o >= 0 ) {
					ACurvePoint *po = &m_pCurve->mList[o];

                    QPointF p1( po->mX / m_Step.x() * cell,
                               -po->mY[l] / m_Step.y() * cell);

                    d   = (m_pCurve->mList[i].mX - po->mX) * 0.5 / m_Step.x() * cell;

                    s1  = QPointF(  p1.x() + d * cos(po->mO[l] * DEG2RAD),
                                    p1.y() - tan(-po->mO[l] * DEG2RAD) / m_Step.y() * cell);
                }

                p2      = QPointF(  pi->mX / m_Step.x() * cell,
                                   -pi->mY[l] / m_Step.y() * cell);

                QPointF s2( p2.x() - d * cos(pi->mI[l] * DEG2RAD),
                            p2.y() - tan(pi->mI[l] * DEG2RAD) / m_Step.y() * cell);

                m_Path.cubicTo( s1, s2, p2 );
            }
            m_Path.lineTo(m_Size.width() - m_Translate.x(), p2.y());
        }
    }
*/
    repaint();
}

void CurveEditor::resizeEvent(QResizeEvent *pe) {
    m_Size  = pe->size();

    update();
}

void CurveEditor::mouseMoveEvent(QMouseEvent *pe) {
    if(control & CONTROL_CAMERA) {
        m_Translate += pe->pos() - m_Pos;
        m_Pos       = pe->pos();

        update();
    }

    if(control & CONTROL_POINT) {
        QPoint d    = pe->pos() - m_Pos;
        m_Pos       = pe->pos();
/*
        if(pRange) {
            if(focus_v == 0)
                pRange->mMin  -= (d.y() * m_Step.y() / cell);
            else
                pRange->mMax  -= (d.y() * m_Step.y() / cell);
        }
*/
/*
        if(m_pCurve) {
            if(focus_p > 0)
                m_pCurve->mList[focus_p].mX         += (d.x() * m_Step.x() / cell);

            if(m_pCurve->mList[focus_p].mX < 0)
                m_pCurve->mList[focus_p].mX   = 0.0f;

            m_pCurve->mList[focus_p].mY[focus_v]  -= (d.y() * m_Step.y() / cell);

            uint32_t result = 0;
            m_pCurve->sort(result);
            focus_p             = result;
        }
*/
        update();
    }

    GraphWidget::mouseMoveEvent(pe);
}

void CurveEditor::wheelEvent(QWheelEvent *pe) {
    if(pe->delta() < 0) {
        m_Step.setX(m_Step.x() + 0.1f);
        m_Step.setY(m_Step.y() + 1.0f);
    } else {
        float x = m_Step.x() - 0.1f;
        float y = m_Step.y() - 1.0f;
        if(x > 0 && y > 0) {
            m_Step.setX(x);
            m_Step.setY(y);
        }
    }

    update();

    GraphWidget::wheelEvent(pe);
}

void CurveEditor::mousePressEvent(QMouseEvent *pe) {
    if(!control && pe->button() == Qt::RightButton) {
        m_Pos        = pe->pos();
        control     = CONTROL_CAMERA;
    }

    if(pe->button() == Qt::LeftButton) {
        m_Pos        = pe->pos();
/*
        if(m_pCurve) {
            if(add) {
                QPointF point   = pe->pos() - m_Translate;
                QRectF rect     = QRectF(point.x() - 3, point.y() - 3, 6, 6);

                if(m_Path.intersects(rect)) {
                    float x     = point.x() * m_Step.x() / cell;
                    if(x < 0.0f)
                        x       = 0.0f;

                    Vector3 t;
                    Vector3 pos;
                    uint32_t index;
                    m_pCurve->point(x, index, pos);
                    m_pCurve->insert(index, x, pos, t, t);
                    uint32_t result  = 0;
                    m_pCurve->sort(result);
                    update();
                }
            }
            if(remove && focus_p > -1) {
                m_pCurve->erase(focus_p);
                update();
            }
        }
*/
        if(focus_p > -1) {
            select_p    = focus_p;
            select_v    = focus_v;
        }
        if(select_p > -1 && focus_p > -1) {
            control     = CONTROL_POINT;
        }
    }

    GraphWidget::mousePressEvent(pe);
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *pe) {
    if(control & CONTROL_POINT) {
        // Update property grid
    }
    control         = CONTROL_NONE;

    GraphWidget::mouseReleaseEvent(pe);
}

void CurveEditor::keyPressEvent(QKeyEvent *pe) {
    switch (pe->key()) {
        case Qt::Key_Control: {
            add     = true;
        } break;
        case Qt::Key_Alt: {
            remove  = true;
        } break;
        default: break;
    }
}

void CurveEditor::keyReleaseEvent(QKeyEvent *pe) {
    switch (pe->key()) {
        case Qt::Key_Control: {
            add     = false;
        } break;
        case Qt::Key_Alt: {
            remove  = false;
        } break;
        default: break;
    }
}
