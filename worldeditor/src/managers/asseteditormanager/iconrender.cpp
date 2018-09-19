#include "iconrender.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

#include <QImage>
#include <QFileInfo>

#include <engine.h>

#include <system.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/directlight.h>
#include <components/staticmesh.h>
#include <components/spritemesh.h>

#include <resources/pipeline.h>

#include "pluginmodel.h"
#include "assetmanager.h"
#include "converters/converter.h"
#include "controllers/cameractrl.h"

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

    m_pController   = new IController();

    m_pRender   = PluginModel::instance()->createSystem("RenderGL");
    if(m_pRender) {
        m_pRender->init();
        m_pRender->overrideController(m_pController);
    }

    m_pScene    = Engine::objectCreate<Scene>();
    m_pScene->setAmbient(0.3f);
    m_pCamera   = Engine::createActor("ActiveCamera", m_pScene);
    m_pCamera->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    Camera *camera  = m_pCamera->addComponent<Camera>();
    m_pController->setActiveCamera(camera);

    m_pLight    = Engine::createActor("LightSource", m_pScene);
    Matrix3 rot;
    rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
    m_pLight->transform()->setRotation(rot);
    m_pLight->addComponent<DirectLight>();
}

IconRender::~IconRender() {
    delete m_pController;
}

const QImage IconRender::render(const QString &resource, uint8_t type) {
    m_Context->makeCurrent(m_Surface);

    Camera *camera  = m_pController->activeCamera();
    camera->pipeline()->resize(m_pFBO->size().width(), m_pFBO->size().height());
    camera->setOrthographic(false);
    Actor *object   = Engine::createActor("", m_pScene);
    float fov       = camera->fov();
    switch(type) {
        case IConverter::ContentTexture: {
            camera->setOrthographic(true);
            m_pCamera->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));

            SpriteMesh *sprite  = object->addComponent<SpriteMesh>();
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
                    mesh->setMaterial(mat);
                }
                AABBox bb   = m->bound();
                m_pCamera->transform()->setPosition(Vector3(bb.center.x, bb.center.y, bb.size.length() * 0.6 / sinf(fov * DEG2RAD)) );
            }
        } break;
        case IConverter::ContentMesh: {
            StaticMesh *mesh    = object->addComponent<StaticMesh>();
            Mesh *m = Engine::loadResource<Mesh>(resource.toStdString());
            if(m) {
                mesh->setMesh(m);
                AABBox bb   = m->bound();
                m_pCamera->transform()->setPosition(Vector3(bb.center.x, bb.center.y, bb.size.length() / sinf(fov * DEG2RAD)) );
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
