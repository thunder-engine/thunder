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
    ~SceneView              () override;

    void                    setEngine           (Engine *engine);

    void                    setScene            (Scene *scene);
    Scene                  *scene               ()              { return m_pScene; }

    void                    setController       (CameraCtrl *ctrl);
    CameraCtrl             *controller          () const        { return m_pController; }

public:
    bool                    init                () override { return true; }

    void                    update              () override {}

    bool                    start               () override { return true; }

    void                    stop                () override {}

    void                    destroy             () override {}

    bool                    isValid             () override { return true; }

    bool                    key                 (Input::KeyCode) override;
    bool                    keyPressed          (Input::KeyCode) override;
    bool                    keyReleased         (Input::KeyCode) override;

    bool                    mouseButton         (Input::MouseButton) override;
    bool                    mousePressed        (Input::MouseButton) override;
    bool                    mouseReleased       (Input::MouseButton) override;

    Vector4                 mousePosition       () override {
        QPoint p    = mapFromGlobal(QCursor::pos());
        return Vector4(p.x(),
                       p.y(),
                       static_cast<float>(p.x()) / width(),
                       static_cast<float>(p.y()) / height());
    }

    Vector4                 mouseDelta          () override { return Vector4(); }

    void                    setMousePosition    (int32_t x, int32_t y) override {
        QCursor::setPos(mapToGlobal(QPoint(x, y)));
    }

    uint32_t                screenWidth         () override { return width(); }

    uint32_t                screenHeight        () override { return height(); }

    uint32_t                joystickCount       () override { return 0; }

    uint32_t                joystickButtons     (uint32_t) override { return 0; }

    Vector4                 joystickThumbs      (uint32_t) override { return Vector4(); }

    Vector2                 joystickTriggers    (uint32_t) override { return Vector2(); }

    void                   *pluginLoad          (const char *) override { return nullptr; }

    bool                    pluginUnload        (void *) override { return false; }

    void                   *pluginAddress       (void *, const string &) override { return nullptr; }

    string                  locationLocalDir    () override { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(); }

signals:
    void                    inited              ();

protected:
    void                    initializeGL        () override;
    void                    paintGL             () override;
    void                    resizeGL            (int width, int height) override;

    void                    mousePressEvent     (QMouseEvent *) override;
    void                    mouseReleaseEvent   (QMouseEvent *) override;

    void                    keyPressEvent       (QKeyEvent *) override;
    void                    keyReleaseEvent     (QKeyEvent *) override;

protected:
    virtual void            findCamera          ();

    CameraCtrl             *m_pController;

    Scene                  *m_pScene;

    Engine                 *m_pEngine;

    QMenu                   m_RenderModeMenu;

    unordered_map<int32_t, int32_t> m_Keys;
    unordered_map<int32_t, int32_t> m_MouseButtons;

};

#endif // SCENEVIEW_H
