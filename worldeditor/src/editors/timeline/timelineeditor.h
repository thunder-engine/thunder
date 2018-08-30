#ifndef TIMELINEEDITOR_H
#define TIMELINEEDITOR_H

#include "../../graph/editors/graphwidget.h"
#include "anim/keyframe.h"

#include <QMenu>

#include <resources/animationclip.h>

class AnimationController;

class TimelineEditor : public GraphWidget {
    Q_OBJECT
public:
    TimelineEditor          (QWidget *parent = 0);

    void                    draw                (QPainter &painter, const QRect &r);

    void                    setClip             (AnimationClip *clip);

    void                    setPosition         (uint32_t ms);

    int32_t                 clipWidth           ();

    int32_t                 clipHeight          ();

    void                    setHovered          (int32_t index);

public slots:
    void                    onHScrolled         (int);
    void                    onVScrolled         (int);

signals:
    void                    moved               (uint32_t ms);

    void                    changed             ();

    void                    scaled              ();

    void                    hovered             (uint32_t index);

protected:
    void                    wheelEvent          ( QWheelEvent *pe );

    void                    mouseMoveEvent      ( QMouseEvent *pe );
    void                    mousePressEvent     ( QMouseEvent *pe );
    void                    mouseReleaseEvent   ( QMouseEvent *pe );

    void                    mouseDoubleClickEvent( QMouseEvent *pe );

    void                    keyPressEvent       ( QKeyEvent *pe );

    void                    resizeEvent         ( QResizeEvent *pe );

    bool                    checkKeys           (const QPoint &pos);

    void                    deleteSelected      ();

protected slots:
    void                    onAddKey            ();

    void                    onDeleteKey         ();

    void                    on_customContextMenuRequested   (const QPoint &pos);

protected:
    struct Select {
        KeyFrame           *key;

        float               pos;
    };

    float                   m_Position;
    float                   m_Scale;
    uint8_t                 m_Step;

    int32_t                 m_OldPos;

    bool                    m_Drag;

    AnimationClip          *m_pClip;

    QPainterPath            m_Key;

    QList<Select>           m_Selected;

    QRect                   m_Head;

    QMenu                   m_CreateMenu;
    QAction                *m_pAddKey;
    QAction                *m_pDeleteKeys;

    QPoint                  m_Point;

    QPoint                  m_Translate;

    int32_t                 m_HoverTrack;
};

#endif // TIMELINEEDITOR_H
