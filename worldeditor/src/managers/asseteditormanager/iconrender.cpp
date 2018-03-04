#include "iconrender.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

#include <QImage>
#include <QFileInfo>

#include <engine.h>
#include <controller.h>

#include <system.h>

#include <components/actor.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/directlight.h>
#include <components/staticmesh.h>
#include <components/sprite.h>

#include "managers/pluginmanager/pluginmodel.h"
#include "assetmanager.h"
#include "baseconvertersettings.h"

IconRender::IconRender(Engine *engine, QOpenGLContext *share, QObject *parent) :
        QObject(parent) {

    m_pEngine   = engine;

    m_Surface   = new QOffscreenSurface();
    m_Surface->create();

    m_Context   = new QOpenGLContext();
    m_Context->setShareContext(share);
    m_Context->setFormat(m_Surface->requestedFormat());
    m_Context->create();
    m_Context->makeCurrent(m_Surface);

    m_pFBO      = new QOpenGLFramebufferObject(128, 128);

    m_pController   = new IController(m_pEngine);

    m_pRender   = PluginModel::instance()->createSystem("RenderGL");
    if(m_pRender) {
        m_pRender->init();
        m_pRender->overrideController(m_pController);
        m_pRender->resize(m_pFBO->size().width(), m_pFBO->size().height());
    }

    m_pScene    = Engine::objectCreate<Scene>();
    m_pScene->setAmbient(0.3f);
    m_pCamera   = Engine::objectCreate<Actor>("ActiveCamera", m_pScene);
    m_pCamera->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    Camera *camera  = m_pCamera->addComponent<Camera>();
    m_pController->setActiveCamera(camera);

    m_pLight    = Engine::objectCreate<Actor>("LightSource", m_pScene);
    Matrix3 rot;
    rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
    m_pLight->setRotation(rot);
    m_pLight->addComponent<DirectLight>();
}

const QImage IconRender::render(const QString &resource, uint8_t type) {
    m_Context->makeCurrent(m_Surface);

    Camera *camera  = m_pController->activeCamera();
    camera->setType(Camera::PERSPECTIVE);
    Actor *object   = Engine::objectCreate<Actor>("", m_pScene);
    float fov       = camera->fov();
    switch(type) {
        case IConverter::ContentTexture: {
            camera->setType(Camera::ORTHOGRAPHIC);
            m_pCamera->setPosition(Vector3(0.0f, 0.0f, 1.0f));

            Sprite *sprite  = object->addComponent<Sprite>();
            if(sprite) {
                sprite->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
                Texture *t  = Engine::loadResource<Texture>(resource.toStdString());
                sprite->setTexture(t);
            }
        } break;
        case IConverter::ContentMaterial: {
            StaticMesh *mesh    = object->addComponent<StaticMesh>();
            Mesh *m = Engine::loadResource<Mesh>(".embedded/sphere.fbx");
            if(m) {
                mesh->setMesh(m);
                Material *mat   = Engine::loadResource<Material>(resource.toStdString());
                if(mat) {
                    mesh->setMaterial(0, mat);
                }
                for(uint32_t s = 0; s < m->surfacesCount(); s++) {
                    m_pCamera->setPosition(m->bound(s).center + Vector3( 0.0f, 0.0f, m->bound(s).size.length() / sinf(fov * DEG2RAD) ));
                }
            }
        } break;
        case IConverter::ContentMesh: {
            StaticMesh *mesh    = object->addComponent<StaticMesh>();
            Mesh *m = Engine::loadResource<Mesh>(resource.toStdString());
            if(m) {
                mesh->setMesh(m);
                Vector3 pos;
                float radius    = 0.0f;
                float bottom    = 0.0f;
                uint32_t i      = 0;

                for(uint32_t s = 0; s < m->surfacesCount(); s++) {
                    pos     += m->bound(s).center;
                    int length  = m->bound(s).center.length();
                    radius  += length * 1.5;
                    Vector3 min, max;
                    m->bound(s).box(min, max);
                    if(i == 0) {
                        bottom  = min.y;
                    }
                    bottom      = MIN(bottom, min.y);
                    i++;
                }
                uint32_t size   = m->surfacesCount();
                pos     /= size;
                radius  /= size;
                m_pCamera->setPosition(pos + Vector3(0.0f, 0.0f, radius / sinf(fov * DEG2RAD)) );
            }
        } break;
        default: break;
    }

    if(m_pRender) {
        m_pRender->update(*m_pScene, m_pFBO->handle());
    }
    m_Context->functions()->glFlush();

    delete object;
    return m_pFBO->toImage();
}
