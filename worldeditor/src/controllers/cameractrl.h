#ifndef CAMERACTRL_H
#define CAMERACTRL_H

#include <QObject>
#include <QMouseEvent>

#include <amath.h>

class Actor;
class Scene;
class Camera;

class QOpenGLWidget;
class ICommandBuffer;

class CameraCtrl : public QObject {
    Q_OBJECT

public:
    enum MoveTypes {
        MOVE_IDLE     = 0,
        MOVE_FORWARD  = (1<<0),
        MOVE_BACKWARD = (1<<1),
        MOVE_LEFT     = (1<<2),
        MOVE_RIGHT    = (1<<3)
    };

public:
    CameraCtrl (QOpenGLWidget *view);

    virtual void init (Scene *scene);

    void loadSettings ();

    void update ();

    void setFocusOn (Actor *actor, float &bottom);

    void setFree (bool flag) { mCameraFree = flag; }

    void blockMovement (bool flag) { mBlockMove = flag; }

    void blockRotations (bool flag) { mBlockRot = flag; }

    Camera *camera () const { return m_pActiveCamera; }

public slots:
    virtual void onInputEvent (QInputEvent *);

    virtual void onOrthographic (bool flag);

protected:
    void cameraZoom (float delta);

    void cameraRotate (const Vector3 &delta);

    void cameraMove (const Vector3 &delta);

protected:
    uint8_t mCameraMove;
    bool mCameraFree;

    bool mBlockMove;
    bool mBlockRot;

    Vector3 mCameraSpeed;
    Quaternion mRotation;

    QPoint mSaved;

    Actor *m_pCamera;

    QOpenGLWidget *m_pView;

    Camera *m_pActiveCamera;
};

#endif // CAMERACTRL_H
