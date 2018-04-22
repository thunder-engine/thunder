#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QOpenGLWidget>

#include <QInputEvent>
#include <QMenu>
#include <QPainter>

#include <components/scene.h>
#include "config.h"

class Engine;
class ISystem;
class ICommandBuffer;

class CameraCtrl;

class QOffscreenSurface;
class QOpenGLFramebufferObject;



class SceneView : public QOpenGLWidget {
    Q_OBJECT
public:
    SceneView               (QWidget *parent = 0);

    ~SceneView              ();

    void                    setRender           (const QString &render);

    void                    setScene            (Scene *scene);
    Scene                  *scene               ()              { return m_pScene; }

    void                    setController       (IController *ctrl);
    IController            *controller          () const        { return m_pController; }

    void                    startGame           ();

    void                    stopGame            ();

    bool                    isGame              () const;

signals:
    void                    inited              ();

protected:
    void                    initializeGL        ();
    void                    paintGL             ();
    void                    resizeGL            (int width, int height);

protected:
    virtual void            findCamera          ();

    IController            *m_pController;

    ISystem                *m_pRender;
    Scene                  *m_pScene;

    QMenu                   m_RenderModeMenu;

    QString                 m_RenderDesc;

    bool                    m_GameMode;
};

#endif // SCENEVIEW_H
