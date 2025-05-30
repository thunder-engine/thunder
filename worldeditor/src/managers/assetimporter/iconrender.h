#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>
#include <vector>

class World;
class Actor;
class Camera;
class Scene;
class Texture;

class RenderSystem;
class CommandBuffer;

class IconRender : public QObject {
public:
    IconRender(QObject *parent = nullptr);

    ~IconRender();

    const QImage render(const QString &uuid);

protected:
    static void readPixels(void *object);

protected:
    World *m_world;

    Scene *m_scene;

    Actor *m_actor;

    Actor *m_light;

    Camera *m_camera;

    RenderSystem *m_render;

    Texture *m_color;
};

#endif // ICONRENDER_H
