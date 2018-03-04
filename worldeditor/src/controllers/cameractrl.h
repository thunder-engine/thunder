#ifndef CAMERACTRL_H
#define CAMERACTRL_H

#include <QObject>
#include <QMouseEvent>

#include <controller.h>

class Actor;

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
    CameraCtrl                          (Engine *engine);

    void                                init                        (Scene *);

    void                                update                      ();

    virtual void                        drawHandles                 ();

    void                                setFocusOn                  (Actor *actor, float &bottom);

    void                                blockMovement               (bool flag) { mBlockMove    = flag; }

    void                                blockRotations              (bool flag) { mBlockRot     = flag; }

    void                                blockFree                   (bool flag) { mBlockFree    = flag; }

    virtual void                        resize                      (uint32_t width, uint32_t height) {}

public slots:
    virtual void                        onInputEvent                (QInputEvent *);

protected:
    void                                cameraRotate                (const Quaternion  &q);

    void                                cameraZoom                  (float delta);

protected:
    uint8_t                             mCameraMove;
    bool                                mCameraFree;

    bool                                mBlockMove;
    bool                                mBlockRot;
    bool                                mBlockFree;

    Vector3                           mCameraSpeed;

    QPoint                              mSaved;

    Vector3                           mRotation;

    Actor                              *m_pCamera;
};

#endif // CAMERACTRL_H
