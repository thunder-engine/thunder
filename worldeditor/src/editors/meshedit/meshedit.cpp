#include "meshedit.h"
#include "ui_meshedit.h"

#include <engine.h>

#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>
#include <components/camera.h>

#include <resources/prefab.h>

#include <global.h>

#include "controllers/cameractrl.h"

#include "assimpconverter.h"

MeshEdit::MeshEdit() :
        ui(new Ui::MeshEdit),
        m_pMesh(nullptr),
        m_pGround(nullptr),
        m_pLight(nullptr) {

    ui->setupUi(this);

    ui->preview->setScene(Engine::objectCreate<Scene>("Scene"));

    CameraCtrl *ctrl = new CameraCtrl(ui->preview);
    ctrl->blockMovement(true);
    ctrl->setFree(false);
    ctrl->init(nullptr);

    ui->preview->setController(ctrl);

    Scene *scene = ui->preview->scene();

    m_pLight = Engine::composeActor("DirectLight", "LightSource", scene);
    m_pLight->transform()->setQuaternion(Quaternion(Vector3(-30.0f, 45.0f, 0.0f)));
    DirectLight *light = static_cast<DirectLight *>(m_pLight->component("DirectLight"));
    if(light) {
        light->setCastShadows(true);
    }

    Prefab *prefab = Engine::loadResource<Prefab>(".embedded/cube.fbx");
    if(prefab) {
        m_pGround = static_cast<Actor *>(prefab->actor()->clone(scene));
        m_pGround->transform()->setScale(Vector3(100.0f, 1.0f, 100.0f));
    }
}

MeshEdit::~MeshEdit() {
    delete ui;

    delete m_pMesh;
    delete m_pGround;
    delete m_pLight;
}

bool MeshEdit::isModified() const {
    if(m_pSettings) {
        return m_pSettings->isModified();
    }
    return false;
}

QStringList MeshEdit::suffixes() const {
    return {"fab", "fbx"};
}

void MeshEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings != settings) {
        if(m_pMesh) {
            delete m_pMesh;
        }

        if(m_pSettings) {
            disconnect(m_pSettings, &AssetConverterSettings::updated, this, &MeshEdit::onUpdateTemplate);
        }
        m_pSettings = settings;
        connect(m_pSettings, SIGNAL(updated()), this, SLOT(onUpdateTemplate()));

        Prefab *prefab = Engine::loadResource<Prefab>(qPrintable(settings->destination()));
        if(prefab) {
            m_pMesh = static_cast<Actor *>(prefab->actor()->clone(ui->preview->scene()));
        }

        float bottom;
        ui->preview->controller()->setFocusOn(m_pMesh, bottom);
        if(m_pGround) {
            Transform *t = m_pGround->transform();
            t->setPosition(Vector3(0.0f, bottom - (t->scale().y * 0.5f), 0.0f));
        }
    }
}

void MeshEdit::saveAsset(const QString &path) {
    m_pSettings->saveSettings();
}

void MeshEdit::onUpdateTemplate() {
    m_pSettings->setModified();
}

void MeshEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
