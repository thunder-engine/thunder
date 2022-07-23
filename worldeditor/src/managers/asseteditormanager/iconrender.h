#ifndef ICONRENDER_H
#define ICONRENDER_H

#include <QObject>

#include <stdint.h>
#include <vector>

class Engine;
class SceneGraph;
class Actor;
class Camera;

class IconRender : public QObject {
public:
    IconRender(Engine *engine, QObject *parent = nullptr);

    ~IconRender();

    const QImage render(const QString &resource, const QString &);

protected:
    SceneGraph *m_pScene;

    Actor *m_pActor;

    Actor *m_pLight;

    Camera *m_pCamera;

    bool m_Init;
};

#endif // ICONRENDER_H
