#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>
#include <vector>

class World;
class Actor;
class Camera;
class Scene;

class IconRender : public QObject {
public:
    IconRender(QObject *parent = nullptr);

    ~IconRender();

    const QImage render(const QString &resource, const QString &);

protected:
    World *m_world;

    Scene *m_scene;

    Actor *m_actor;

    Actor *m_light;

    Camera *m_camera;

    bool m_init;
};

#endif // ICONRENDER_H
