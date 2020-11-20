#ifndef CAMERACTRL_H
#define CAMERACTRL_H

#include <QObject>
#include <QMouseEvent>

#include <QStack>

#include <amath.h>
#include <object.h>

class Actor;
class Scene;
class Camera;

class QOpenGLWidget;
class QMenu;
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

    enum class ViewSide {
        VIEW_SCENE    = 0,
        VIEW_FRONT,
        VIEW_BACK,
        VIEW_LEFT,
        VIEW_RIGHT,
        VIEW_TOP,
        VIEW_BOTTOM
    };

public:
    CameraCtrl(QOpenGLWidget *view);

    virtual void init(Scene *scene);

    virtual void drawHandles();

    virtual void resize(int32_t width, int32_t height);

    virtual Object::ObjectList selected();

    void loadSettings();

    void update();

    void setFocusOn(Actor *actor, float &bottom);

    void setFree(bool flag) { m_CameraFree = flag; }

    void blockMovement(bool flag) { m_BlockMove = flag; }

    void blockRotations(bool flag) { m_BlockRotation = flag; }

    Camera *camera() const { return m_pActiveCamera; }

    virtual void createMenu(QMenu *menu);

    ViewSide viewSide() const { return m_ViewSide; }

public slots:
    virtual void onInputEvent(QInputEvent *);

    virtual void onOrthographic(bool flag);

    void frontSide();
    void backSide();
    void leftSide();
    void rightSide();
    void topSide();
    void bottomSide();

protected:
    void cameraZoom(float delta);

    void cameraRotate(const Vector3 &delta);

    void cameraMove(const Vector3 &delta);

private:
    void doRotation(const Vector3 &vector);

protected:
    uint8_t m_CameraMove;
    ViewSide m_ViewSide;

    bool m_BlockMove;
    bool m_BlockRotation;

    bool m_CameraFree;
    bool m_CameraFreeSaved;
    bool m_RotationTransfer;

    Vector3 m_CameraSpeed;

    Vector3 m_Rotation;
    Vector3 m_Position;

    Vector3 m_RotationTarget;
    Vector3 m_PositionTarget;

    float m_OrthoWidthTarget;
    float m_FocalLengthTarget;
    float m_TransferProgress;

    QPoint m_Saved;

    Actor *m_pCamera;

    QOpenGLWidget *m_pView;

    Camera *m_pActiveCamera;
};

#endif // CAMERACTRL_H
