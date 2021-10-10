#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>

class Engine;
class Scene;
class Actor;
class Camera;

class IconRender : public QObject {
public:
    IconRender(Engine *engine, QObject *parent = nullptr);

    ~IconRender();

    const QImage render(const QString &resource, const QString &);

protected:
    Scene                    *m_pScene;

    Actor                    *m_pActor;

    Actor                    *m_pLight;

    Camera                   *m_pCamera;

    bool                      m_Init;
};

#endif // ICONRENDER_H
