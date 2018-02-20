#ifndef OBJECTCTRL_H
#define OBJECTCTRL_H

#include <QObject>
#include <QInputEvent>

#include <cstdint>
#include <map>

#include <amath.h>
#include <aobject.h>

#include "cameractrl.h"
#include "graph/sceneview.h"
#include "managers/undomanager/undomanager.h"

class Engine;
class Actor;
class Scene;

class RadioOverlayButton : public OverlayButton {
    Q_OBJECT
public:
    RadioOverlayButton() {

    }

    void addButton(OverlayButton *button) {
        m_Buttons.push_back(button);
        QSize size;
        foreach(OverlayButton *it, m_Buttons) {
            size.setWidth( size.width() + it->rect().width() );
            size.setHeight( MAX(it->rect().height(), size.height()) );
        }
        m_Rect.setSize(size);
    }

    void draw(QPainter &painter) const {
        painter.setFont(gFont);
        painter.setPen(Qt::NoPen);

        int pos = 0;
        for(int i = 0; i < m_Buttons.size(); i++) {
            const OverlayButton *it = m_Buttons.at(i);

            if(it == m_pCurrent) {
                painter.setBrush(QColor((it == m_pHovered) ? "#388e3c" : "#2e7d32"));
            } else {
                painter.setBrush(QColor((it == m_pHovered) ? "#039be5" : "#0277bd"));
            }

            QRect r(it->rect());
            r.moveTo(m_Rect.left() + pos, m_Rect.top());
            QPainterPath path;
            path.setFillRule( Qt::WindingFill );
            if(i == 0) {
                path.addRoundedRect( r, gRoundness, gRoundness );
                path.addRect( QRect( r.right() - gRoundness + 1, m_Rect.top(), gRoundness, m_Rect.height() ) ); // Righ side not rounded
            } else if(i == m_Buttons.size() - 1) {
                path.addRoundedRect( r, gRoundness, gRoundness );
                path.addRect( QRect( r.left(), m_Rect.top(), gRoundness, m_Rect.height() ) ); // Left side not rounded
            } else {
                path.addRect( r );
            }
            painter.drawPath( path.simplified() );
            painter.drawImage(m_Rect.left() + gRoundness + pos, m_Rect.top(), it->icon());

            r.moveTo(m_Rect.left() + it->icon().width() + gRoundness + pos, m_Rect.top());
            painter.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, it->name());

            pos += it->rect().width();
        }
    }

    bool onMouseEvent(QMouseEvent *pe) {
        m_pHovered  = nullptr;
        int pos     = 0;
        foreach(OverlayButton *it, m_Buttons) {
            QRect r = it->rect();
            r.moveTo(m_Rect.left() + pos, m_Rect.top());
            pos += r.width();
            if(r.contains(pe->pos())) {
                m_pHovered   = it;
            }
        }

        if(pe->button() == Qt::LeftButton) {
            if(m_pHovered) {
                setActive(m_pHovered);
                return true;
            }
        }
        return false;
    }

    void setActive(OverlayButton *active) {
        m_pCurrent  = active;
        if(m_pCurrent) {
            m_pCurrent->clicked();
        }
    }

protected:
    QList<OverlayButton *>  m_Buttons;

    OverlayButton          *m_pHovered;
    OverlayButton          *m_pCurrent;
};

class ObjectCtrl : public CameraCtrl {
    Q_OBJECT

public:
    enum ModeTypes {
        MODE_TRANSLATE  = 1,
        MODE_ROTATE     = 2,
        MODE_SCALE      = 3
    };

    struct Select {
        Actor          *object;
        Vector3       position;
        Quaternion      rotation;
        Vector3       scale;
    };

public:
    ObjectCtrl          (Engine *engine, SceneView *view);

    void                drawHandles                 ();

    void                clear                       (bool signal = true);

    void                deleteSelected              (bool force = false);

    void                selectActor                 (const list<uint32_t> &list, bool undo = true);

    AObject::ObjectList selected                    ();

    void                setMap                      (AObject *map)      { m_pMap = map; }

    void                setMoveGrid                 (float value)       { mMoveGrid = value; }
    void                setAngleGrid                (float value)       { mAngleGrid = value; }

    AObject            *findObject                  (uint32_t id, AObject *parent = nullptr);

public slots:
    void                onInputEvent                (QInputEvent *);

    void                onComponentSelected         (const QString &path);

    void                onDrop                      ();
    void                onDragEnter                 (QDragEnterEvent *);
    void                onDragLeave                 (QDragLeaveEvent *);

    void                onSelectActor               (AObject::ObjectList list, bool undo = true);
    void                onRemoveActor               (AObject::ObjectList, bool undo = true);
    void                onParentActor               (AObject::ObjectList objects, AObject::ObjectList parents, bool undo = true);

    void                onFocusActor                (AObject *object);

    void                onMoveActor                 ();
    void                onRotateActor               ();
    void                onScaleActor                ();

signals:
    void                mapUpdated                  ();

    void                objectsUpdated              ();

    void                objectsSelected             (AObject::ObjectList objects);

protected:
    void                drawHelpers                 (AObject &object);

    void                selectGeometry              (Vector2 &, Vector2 &);

    Vector3             objectPosition              ();

    bool                isDrag                      ()  { return mDrag; }

    void                setDrag                     (bool drag);

private slots:


protected:
    typedef map<uint32_t, Select>   SelectMap;
    SelectMap           m_Selected;

    bool                mAdditive;
    bool                mCopy;
    bool                mDrag;

    bool                mMoveLeft;
    bool                mMoveRight;

    /// Current mode (see AController::ModeTypes)
    uint8_t             mMode;

    uint8_t             mAxes;

    float               mMoveGrid;
    float               mAngleGrid;
    float               mScaleGrid;

    AObject            *m_pMap;

    AObject::ObjectList m_DragObjects;

    SceneView          *m_pView;

    Vector2           mMousePosition;

    Vector3           mWorld;
    Vector3           mSaved;
    Vector3           mPosition;

    UndoManager::PropertyObjects   *m_pPropertyState;
};

#endif // OBJECTCTRL_H
