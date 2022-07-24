#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>
#include <vector>

class SceneGraph;
class Actor;
class Camera;

class IconRender : public QObject {
public:
    IconRender(QObject *parent = nullptr);

    ~IconRender();

    const QImage render(const QString &resource, const QString &);

protected:
    SceneGraph *m_scene;

    Actor *m_actor;

    Actor *m_light;

    Camera *m_camera;

    bool m_init;
};

#endif // ICONRENDER_H
