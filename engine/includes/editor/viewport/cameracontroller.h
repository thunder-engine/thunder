#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QMouseEvent>

#include <QStack>

#include <amath.h>
#include <object.h>

#include <engine.h>

class Actor;
class Camera;
class Viewport;

class QMenu;

class ENGINE_EXPORT CameraController : public QObject {
    Q_OBJECT

public:
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
    CameraController();

    virtual void init(Viewport *view) {};

    virtual void drawHandles();

    virtual void resize(int32_t width, int32_t height);

    virtual QList<Object *> selected();
    virtual void select(Object &object);

    virtual void update();

    void move();

    void setFocusOn(Actor *actor, float &bottom);

    void setFree(bool flag) { m_cameraFree = flag; m_cameraFreeSaved = m_cameraFree; }

    void blockMovement(bool flag) { m_blockMove = flag; }

    void blockRotations(bool flag) { m_blockRotation = flag; }

    Camera *camera() const { return m_activeCamera; }

    bool cameraInMove() const { return m_cameraInMove; }

    virtual void createMenu(QMenu *menu);

    ViewSide viewSide() const { return m_viewSide; }

    void restoreState(const VariantMap &state);
    VariantMap saveState() const;

    void setActiveRootObject(Object *object);

    void setZoomLimits(const Vector2 &limit);

signals:
    void setCursor(const QCursor &cursor);

    void unsetCursor();

public slots:
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

    void drawHelpers(Object &object);

private:
    void doRotation(const Vector3 &vector);

protected:
    uint8_t m_cameraMove;
    ViewSide m_viewSide;

    bool m_blockMove;
    bool m_blockRotation;

    bool m_cameraFree;
    bool m_cameraFreeSaved;
    bool m_rotationTransfer;

    bool m_cameraInMove;

    Vector2 m_screenSize;

    Vector3 m_cameraSpeed;

    Vector3 m_rotation;
    Vector3 m_position;

    Vector3 m_rotationTarget;
    Vector3 m_positionTarget;

    float m_orthoWidthTarget;
    float m_focalLengthTarget;
    float m_transferProgress;

    Vector2 m_saved;

    Actor *m_camera;

    Camera *m_activeCamera;

    Object *m_activeRootObject;

    Vector2 m_zoomLimit;

};

#endif // CAMERACONTROLLER_H
