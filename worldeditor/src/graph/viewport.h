#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "sceneview.h"

class OverlayButton : public QObject {
    Q_OBJECT
public:
    OverlayButton(const QString &name = QString(), const QImage &icon = QImage()) :
            QObject() {
        m_Hovered   = false;
        m_Name  = name;
        m_Icon  = icon;
        m_Rect  = QRect(0, 0, gMetrics.width(m_Name) + m_Icon.width() + gRoundness * 2, m_Icon.height());
    }

    virtual void draw(QPainter &painter) const {
        painter.setFont(gFont);

        if(m_Hovered) {
            painter.setBrush(QColor("#0277bd"));
        } else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawRoundedRect(m_Rect, gRoundness, gRoundness);
        painter.drawImage(m_Rect.left() + gRoundness, m_Rect.top(), m_Icon);
        QRect r(m_Rect);
        r.setLeft(m_Icon.width() + gRoundness);
        painter.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, m_Name);
    }

    void setPos(const QPoint &p) {
        m_Rect.moveTo(p);
    }

    QRect rect() const {
        return m_Rect;
    }

    QImage icon() const {
        return m_Icon;
    }

    QString name() const {
        return m_Name;
    }

    virtual bool onMouseEvent(QMouseEvent *pe) {
        m_Hovered   = m_Rect.contains(pe->pos());
        if(m_Hovered && pe->button() == Qt::LeftButton) {
            emit clicked();
            return true;
        }
        return false;
    }

signals:
    void clicked();

protected:
    QString                 m_Name;

    QImage                  m_Icon;

    QRect                   m_Rect;

    bool                    m_Hovered;
};

class Viewport : public SceneView {
    Q_OBJECT
public:
    Viewport                (QWidget *parent = 0);

    void                    addButton           (OverlayButton *button);

protected:
    void                    initializeGL        ();
    void                    paintGL             ();
    void                    resizeGL            (int width, int height);

signals:
    void                    keyPress            (QKeyEvent *);
    void                    keyRelease          (QKeyEvent *);

    void                    drop                (QDropEvent *);
    void                    dragEnter           (QDragEnterEvent *);
    void                    dragLeave           (QDragLeaveEvent *);

protected:
    void                    drawOverlay         (QPainter &);

    void                    dragEnterEvent      (QDragEnterEvent *);
    void                    dragLeaveEvent      (QDragLeaveEvent *);
    void                    dropEvent           (QDropEvent *);

    void                    mouseMoveEvent      (QMouseEvent *);
    void                    mousePressEvent     (QMouseEvent *);
    void                    mouseReleaseEvent   (QMouseEvent *);

    void                    wheelEvent          (QWheelEvent *);

    void                    keyPressEvent       (QKeyEvent *);
    void                    keyReleaseEvent     (QKeyEvent *);

private:
    void                    findCamera          () {}

    QList<OverlayButton*>   m_OverlayButtons;

    ICommandBuffer         *m_pCommandBuffer;
};

#endif // VIEWPORT_H
