#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>

#include <amath.h>
#include <object.h>

#include <engine.h>

class Actor;
class Camera;
class Viewport;

class ENGINE_EXPORT CameraController : public QObject {
    Q_OBJECT

public:
    enum class Axis {
        Y,
        X,
        Z
    };

    struct CameraData {
        Vector3 rotation;

        Vector3 position;

        float focalDistance = 1.0f;

        float orthoSize = 1.0f;

        bool ortho = false;
    };

public:
    CameraController();

    virtual void init(Viewport *view) {};

    virtual void drawHandles();

    virtual void resize(int32_t width, int32_t height);

    virtual Object::ObjectList selected();
    virtual void select(Object &object);

    virtual void update();

    void move();

    void setFocusOn(Actor *actor, float &bottom);

    void setFree(bool flag) { m_cameraFree = flag; }

    bool isMovementBlocked() const { return m_blockMove; }
    void blockMovement(bool flag) { m_blockMove = flag; }

    bool isRotationBlocked() const { return m_blockRotation; }
    void blockRotations(bool flag) { m_blockRotation = flag; }

    bool isPickingBlocked() const { return m_blockPicking; }
    void blockPicking(bool flag) { m_blockPicking = flag; }

    bool isPickingOverlaped() const { return m_overlapPicking; }
    void overlapPicking(bool flag) { m_overlapPicking = flag; }

    Camera *camera() const { return m_activeCamera; }

    Axis gridAxis() const { return m_gridAxis; }
    void setGridAxis(Axis axis) { m_gridAxis = axis; }

    void restoreState(const VariantMap &state);
    VariantMap saveState();

    void setActiveRootObject(Object *object);

    void setZoomLimits(const Vector2 &limit);

    void doRotation(const Vector3 &vector);
    void activateCamera(int index, bool force = false);

    void resetCamera();

signals:
    void setCursor(const QCursor &cursor);

    void unsetCursor();

public slots:
    virtual void onOrthographic(bool flag);

protected:
    virtual void cameraZoom(float delta);

    virtual void cameraRotate(const Vector3 &delta);

    virtual void cameraMove(const Vector3 &delta);

    void drawHelpers(Object *object, bool selected);

protected:
    std::vector<CameraData> m_cameras;

    Vector2 m_screenSize;
    Vector2 m_mouseSaved;
    Vector2 m_mouseDelta;
    Vector2 m_zoomLimit;

    Vector3 m_cameraSpeed;

    Camera *m_activeCamera;

    Object *m_activeRootObject;

    Axis m_gridAxis;

    float m_transferProgress;
    float m_transferSpeed;
    int m_currentCamera;

    bool m_blockMove;
    bool m_blockMoveOnTransfer;
    bool m_blockRotation;
    bool m_blockPicking;
    bool m_overlapPicking;

    bool m_cameraFree;
    bool m_cameraInMove;

};

#endif // CAMERACONTROLLER_H
