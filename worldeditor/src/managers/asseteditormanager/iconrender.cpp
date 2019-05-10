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
#include <components/meshrender.h>
#include <components/spriterender.h>

#include <resources/pipeline.h>

#include "pluginmodel.h"
#include "assetmanager.h"
#include "converters/converter.h"
#include "controllers/cameractrl.h"

IconRender::IconRender(Engine *engine, QOpenGLContext *share, QObject *parent) :
        QObject(parent),
        m_Init(false) {

    m_pEngine   = engine;

    m_Surface   = new QOffscreenSurface();
    m_Surface->create();

    m_Context   = new QOpenGLContext();
    m_Context->setShareContext(share);
    m_Context->setFormat(m_Surface->requestedFormat());
    m_Context->create();
    m_Context->makeCurrent(m_Surface);

    //PluginModel::instance()->initSystems();

    m_pFBO = new QOpenGLFramebufferObject(128, 128);

    m_pScene = Engine::objectCreate<Scene>();
    m_pScene->setAmbient(0.3f);
    m_pActor = Engine::objectCreate<Actor>("ActiveCamera", m_pScene);
    m_pActor->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_pCamera = m_pActor->addComponent<Camera>();

    m_pLight = nullptr;

    m_pController = nullptr;
}

IconRender::~IconRender() {

}

void IconRender::init() {

}

const QImage IconRender::render(const QString &resource, uint32_t type) {
    m_Context->makeCurrent(m_Surface);

    if(!m_Init) {
        //PluginModel::instance()->initSystems();

        Pipeline *pipe = m_pCamera->pipeline();
        pipe->resize(m_pFBO->size().width(), m_pFBO->size().height());
        pipe->setTarget(m_pFBO->handle());

        m_pLight = Engine::objectCreate<Actor>("LightSource", m_pScene);
        Matrix3 rot;
        rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
        m_pLight->transform()->setRotation(rot);
        m_pLight->addComponent<DirectLight>();

        m_Init = true;
    }

    m_pCamera->setOrthographic(false);
    Actor *object   = Engine::objectCreate<Actor>("", m_pScene);
    float fov       = m_pCamera->fov();
    switch(type) {
        case IConverter::ContentTexture: {
            m_pCamera->setOrthographic(true);
            m_pActor->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));

            SpriteRender *sprite = object->addComponent<SpriteRender>();
            if(sprite) {
                sprite->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
                Texture *t  = Engine::loadResource<Texture>(resource.toStdString());
                sprite->setTexture(t);
            }
        } break;
        case IConverter::ContentMaterial: {
            MeshRender *mesh    = object->addComponent<MeshRender>();
            Mesh *m = Engine::loadResource<Mesh>(".embedded/sphere.fbx");
            if(m) {
                mesh->setMesh(m);
                Material *mat   = Engine::loadResource<Material>(resource.toStdString());
                if(mat) {
                    mesh->setMaterial(mat);
                }
                AABBox bb   = m->bound();
                m_pActor->transform()->setPosition(Vector3(bb.center.x, bb.center.y, bb.size.length() * 0.6 / sinf(fov * DEG2RAD)) );
            }
        } break;
        case IConverter::ContentMesh: {
            MeshRender *mesh    = object->addComponent<MeshRender>();
            Mesh *m = Engine::loadResource<Mesh>(resource.toStdString());
            if(m) {
                mesh->setMesh(m);
                AABBox bb   = m->bound();
                m_pActor->transform()->setPosition(Vector3(bb.center.x, bb.center.y, bb.size.length() / sinf(fov * DEG2RAD)) );
            }
        } break;
        default: break;
    }

    Camera::setCurrent(m_pCamera);
    PluginModel::instance()->updateSystems(m_pScene);

    m_Context->functions()->glFinish();

    delete object;
    return m_pFBO->toImage();
}
