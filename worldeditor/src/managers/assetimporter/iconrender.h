#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

class World;
class Actor;
class Camera;
class Scene;
class Texture;

class PipelineContext;
class CommandBuffer;

class TString;

class IconRender : public QObject {
public:
    IconRender(QObject *parent = nullptr);

    ~IconRender();

    const QImage render(const TString &uuid);

protected:
    static void readPixels(void *object);

protected:
    World *m_world;

    Scene *m_scene;

    Actor *m_actor;

    Actor *m_light;

    Camera *m_camera;

    PipelineContext *m_context;

    Texture *m_color;
};

#endif // ICONRENDER_H
