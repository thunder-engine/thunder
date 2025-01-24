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
    enum class Axis {
        Y,
        X,
        Z
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

    bool isMovementBlocked() const { return m_blockMove; }
    void blockMovement(bool flag) { m_blockMove = flag; }

    bool isRotationBlocked() const { return m_blockRotation; }
    void blockRotations(bool flag) { m_blockRotation = flag; }

    bool isPickingBlocked() const { return m_blockPicking; }
    void blockPicking(bool flag) { m_blockPicking = flag; }

    bool isPickingOverlaped() const { return m_overlapPicking; }
    void overlapPicking(bool flag) { m_overlapPicking = flag; }

    Camera *camera() const { return m_activeCamera; }

    bool cameraInMove() const { return m_cameraInMove; }

    Axis gridAxis() const { return m_gridAxis; }
    void setGridAxis(Axis axis) { m_gridAxis = axis; }

    void restoreState(const VariantMap &state);
    VariantMap saveState() const;

    void setActiveRootObject(Object *object);

    void setZoomLimits(const Vector2 &limit);

    void doRotation(const Vector3 &vector);

signals:
    void setCursor(const QCursor &cursor);

    void unsetCursor();

public slots:
    virtual void onOrthographic(bool flag);

protected:
    virtual void cameraZoom(float delta);

    virtual void cameraRotate(const Vector3 &delta);

    virtual void cameraMove(const Vector3 &delta);

    void drawHelpers(Object *object);

protected:
    uint8_t m_cameraMove;
    Axis m_gridAxis;

    bool m_blockMove;
    bool m_blockRotation;
    bool m_blockPicking;
    bool m_overlapPicking;

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

    Vector2 m_delta;
    Vector2 m_saved;

    Camera *m_activeCamera;

    Object *m_activeRootObject;

    Vector2 m_zoomLimit;

};

#endif // CAMERACONTROLLER_H
