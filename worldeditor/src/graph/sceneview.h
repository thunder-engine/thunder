#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QOpenGLWidget>

#include <QInputEvent>
#include <QMenu>
#include <QPainter>

#include <components/scene.h>
#include "common.h"

class Engine;
class IRenderSystem;

class CameraCtrl;

class QOffscreenSurface;
class QOpenGLFramebufferObject;

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

class SceneView : public QOpenGLWidget {
    Q_OBJECT
public:
    SceneView               (Engine *engine, QWidget *parent = 0);

    ~SceneView              ();

    void                    setRender           (const QString &render);

    Scene                  *scene               ()              { return m_pScene; }

    void                    setController       (CameraCtrl *ctrl);
    CameraCtrl             *controller          () const        { return m_pController; }

    void                    addButton           (OverlayButton *button);

signals:
    void                    inited              ();

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

    void                    initializeGL        ();
    void                    paintGL             ();
    void                    resizeGL            (int width, int height);

private:
    CameraCtrl             *m_pController;

    Engine                 *m_pEngine;
    IRenderSystem          *m_pRender;
    Scene                  *m_pScene;

    QList<OverlayButton*>   m_OverlayButtons;

    QMenu                   m_RenderModeMenu;

    QString                 m_RenderDesc;

};

#endif // SCENEVIEW_H
