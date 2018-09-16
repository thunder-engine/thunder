#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>

class Engine;
class Scene;
class Actor;

class IController;
class ISystem;

class QOffscreenSurface;
class QOpenGLContext;
class QOpenGLFramebufferObject;

class IconRender : public QObject {
public:
    IconRender                  (Engine *engine, QOpenGLContext *share, QObject *parent = 0);

    ~IconRender                 ();

    const QImage                render              (const QString &resource, uint8_t type);

protected:

    QOffscreenSurface          *m_Surface;

    QOpenGLContext             *m_Context;

    QOpenGLFramebufferObject   *m_pFBO;

    Engine                     *m_pEngine;

    ISystem                    *m_pRender;

    Scene                      *m_pScene;

    Actor                      *m_pCamera;

    Actor                      *m_pLight;

    IController                *m_pController;
};

#endif // ICONRENDER_H
