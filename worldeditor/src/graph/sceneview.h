#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QOpenGLWidget>

#include <adapters/iplatformadaptor.h>

class Engine;
class System;
class Scene;
class ICommandBuffer;

class QOffscreenSurface;
class QOpenGLFramebufferObject;

class SceneView : public QOpenGLWidget, public IPlatformAdaptor {
    Q_OBJECT
public:
    SceneView               (QWidget *parent = nullptr);
    ~SceneView              () override;

    void                    setEngine           (Engine *engine);

    void                    setScene            (Scene *scene);
    Scene                  *scene               () { return m_pScene; }

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

    uint32_t                screenWidth         () override;
    uint32_t                screenHeight        () override;

    uint32_t                joystickCount       () override;
    uint32_t                joystickButtons     (uint32_t) override;
    Vector4                 joystickThumbs      (uint32_t) override;
    Vector2                 joystickTriggers    (uint32_t) override;

    void                   *pluginLoad          (const char *) override;
    bool                    pluginUnload        (void *) override;
    void                   *pluginAddress       (void *, const string &) override;

    string                  locationLocalDir    () override;

protected:
    void                    paintGL             () override;
    void                    resizeGL            (int width, int height) override;

    void                    mousePressEvent     (QMouseEvent *) override;
    void                    mouseReleaseEvent   (QMouseEvent *) override;

    void                    keyPressEvent       (QKeyEvent *) override;
    void                    keyReleaseEvent     (QKeyEvent *) override;

protected:
    virtual void            findCamera          ();

    Scene                  *m_pScene;

    Engine                 *m_pEngine;

    unordered_map<int32_t, int32_t> m_Keys;
    unordered_map<int32_t, int32_t> m_MouseButtons;

};

#endif // SCENEVIEW_H
