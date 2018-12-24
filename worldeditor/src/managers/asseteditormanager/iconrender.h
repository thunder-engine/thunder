#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>

class Engine;
class Scene;
class Actor;
class Camera;

class IController;
class ISystem;

class QOffscreenSurface;
class QOpenGLContext;
class QOpenGLFramebufferObject;

class IconRender : public QObject {
public:
    IconRender                  (Engine *engine, QOpenGLContext *share, QObject *parent = nullptr);

    ~IconRender                 ();

    const QImage                render              (const QString &resource, uint32_t type);

protected:

    QOffscreenSurface          *m_Surface;

    QOpenGLContext             *m_Context;

    QOpenGLFramebufferObject   *m_pFBO;

    Engine                     *m_pEngine;

    ISystem                    *m_pRender;

    Scene                      *m_pScene;

    Actor                      *m_pActor;

    Actor                      *m_pLight;

    IController                *m_pController;

    Camera                     *m_pCamera;
};

#endif // ICONRENDER_H
