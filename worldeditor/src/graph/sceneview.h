#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QOpenGLWidget>

#include <QInputEvent>
#include <QMenu>
#include <QPainter>

#include <QStandardPaths>

#include <components/scene.h>
#include <adapters/iplatformadaptor.h>
#include "config.h"

class Engine;
class ISystem;
class ICommandBuffer;

class CameraCtrl;

class QOffscreenSurface;
class QOpenGLFramebufferObject;

class SceneView : public QOpenGLWidget, public IPlatformAdaptor {
    Q_OBJECT
public:
    SceneView               (QWidget *parent = nullptr);

    ~SceneView              ();

    void                    setRender           (const QString &render);

    void                    setScene            (Scene *scene);
    Scene                  *scene               ()              { return m_pScene; }

    void                    setController       (CameraCtrl *ctrl);
    CameraCtrl             *controller          () const        { return m_pController; }

public:
    bool                    init                () { return true; }

    void                    update              () {}

    bool                    start               () { return true; }

    void                    stop                () {}

    void                    destroy             () {}

    bool                    isValid             () { return true; }

    bool                    key                 (Input::KeyCode);

    Vector4                 mousePosition       () {
        QPoint p    = mapFromGlobal(QCursor::pos());
        return Vector4(p.x(), p.y(),
                       (float)p.x() / width(), (float)p.y() / height());
    }

    Vector4                 mouseDelta          () { return Vector4(); }

    uint32_t                mouseButtons        () { return m_MouseButtons; }

    uint32_t                screenWidth         () { return width(); }

    uint32_t                screenHeight        () { return height(); }

    void                    setMousePosition    (int32_t x, int32_t y) {
        QCursor::setPos(mapToGlobal(QPoint(x, y)));
    }

    uint32_t                joystickCount       () { return 0; }

    uint32_t                joystickButtons     (uint32_t) { return 0; }

    Vector4                 joystickThumbs      (uint32_t) { return Vector4(); }

    Vector2                 joystickTriggers    (uint32_t) { return Vector2(); }

    void                   *pluginLoad          (const char *) { return nullptr; }

    bool                    pluginUnload        (void *) { return false; }

    void                   *pluginAddress       (void *, const string &) { return nullptr; }

    string                  locationLocalDir    () { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(); }

signals:
    void                    inited              ();

protected:
    void                    initializeGL        ();
    void                    paintGL             ();
    void                    resizeGL            (int width, int height);

    void                    mousePressEvent     (QMouseEvent *);
    void                    mouseReleaseEvent   (QMouseEvent *);

    void                    keyPressEvent       (QKeyEvent *);
    void                    keyReleaseEvent     (QKeyEvent *);

protected:
    virtual void            findCamera          ();

    CameraCtrl             *m_pController;

    Scene                  *m_pScene;

    QMenu                   m_RenderModeMenu;

    int32_t                 m_MouseButtons;

    QList<int32_t>          m_Keys;
};

#endif // SCENEVIEW_H
