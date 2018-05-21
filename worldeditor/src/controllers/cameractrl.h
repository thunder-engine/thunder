#ifndef CAMERACTRL_H
#define CAMERACTRL_H

#include <QObject>
#include <QMouseEvent>

#include <controller.h>

class Actor;
class Viewport;

class CameraCtrl : public QObject, public IController {
    Q_OBJECT

public:
    enum MoveTypes {
        MOVE_IDLE                       = 0,
        MOVE_FORWARD                    = (1<<0),
        MOVE_BACKWARD                   = (1<<1),
        MOVE_LEFT                       = (1<<2),
        MOVE_RIGHT                      = (1<<3)
    };

public:
    CameraCtrl                          (Viewport *view);

    void                                init                        (Scene *);

    void                                update                      ();

    virtual void                        drawHandles                 ();

    void                                setFocusOn                  (Actor *actor, float &bottom);

    void                                blockMovement               (bool flag) { mBlockMove    = flag; }

    void                                blockRotations              (bool flag) { mBlockRot     = flag; }

    void                                blockFree                   (bool flag) { mBlockFree    = flag; }

    virtual void                        resize                      (uint32_t, uint32_t) {}

public slots:
    virtual void                        onInputEvent                (QInputEvent *);

    virtual void                        onOrthographic              (bool flag);

protected:
    void                                cameraZoom                  (float delta);

    void                                cameraRotate                (const Vector3 &delta);

    void                                cameraMove                  (const Vector3 &delta);

protected:
    uint8_t                             mCameraMove;
    bool                                mCameraFree;

    bool                                mBlockMove;
    bool                                mBlockRot;
    bool                                mBlockFree;

    Vector3                             mCameraSpeed;
    Quaternion                          mRotation;

    QPoint                              mSaved;

    Actor                              *m_pCamera;

    Viewport                           *m_pView;
};

#endif // CAMERACTRL_H
