#ifndef OBJECTCTRL_H
#define OBJECTCTRL_H

#include <QObject>
#include <QInputEvent>

#include <cstdint>
#include <map>

#include <amath.h>
#include <object.h>

#include "cameractrl.h"
#include "graph/viewport.h"
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
        Vector3         position;
        Vector3         scale;
        Vector3         euler;
    };

public:
    ObjectCtrl          (Viewport *view);

    void                drawHandles                 ();

    void                clear                       (bool signal = true);

    void                deleteSelected              (bool force = false);

    void                selectActor                 (const list<uint32_t> &list, bool undo = true);

    Object::ObjectList  selected                    ();

    void                setMap                      (Object *map)      { m_pMap = map; }

    void                setMoveGrid                 (float value)       { mMoveGrid = value; }
    void                setAngleGrid                (float value)       { mAngleGrid = value; }

    Object             *findObject                  (uint32_t id, Object *parent = nullptr);

    void                resize                      (uint32_t width, uint32_t height);

public slots:
    void                onInputEvent                (QInputEvent *);

    void                onComponentSelected         (const QString &path);

    void                onDrop                      ();
    void                onDragEnter                 (QDragEnterEvent *);
    void                onDragLeave                 (QDragLeaveEvent *);

    void                onSelectActor               (Object::ObjectList list, bool undo = true);
    void                onRemoveActor               (Object::ObjectList, bool undo = true);
    void                onParentActor               (Object::ObjectList objects, Object::ObjectList parents, bool undo = true);

    void                onFocusActor                (Object *object);

    void                onMoveActor                 ();
    void                onRotateActor               ();
    void                onScaleActor                ();

signals:
    void                mapUpdated                  ();

    void                objectsUpdated              ();

    void                objectsSelected             (Object::ObjectList objects);

protected:
    void                drawHelpers                 (Object &object);

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

    Object             *m_pMap;

    Object::ObjectList  m_DragObjects;

    Viewport           *m_pView;

    Vector2             mMousePosition;
    Vector2             m_Screen;

    Vector3             mWorld;
    Vector3             mSaved;
    Vector3             mPosition;

    UndoManager::PropertyObjects   *m_pPropertyState;
};

#endif // OBJECTCTRL_H
