#include "materialedit.h"
#include "ui_materialedit.h"

#include <QQmlContext>
#include <QQuickItem>

#include <QMenu>

#include <engine.h>
#include <components/scenegraph.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>

#include <resources/mesh.h>

#include <editor/viewport/cameractrl.h>

#include <global.h>
#include <json.h>

#include "../converter/shaderbuilder.h"

namespace {
    const char *gMeshRender("MeshRender");
};

MaterialEdit::MaterialEdit() :
        ui(new Ui::MaterialEdit),
        m_mesh(nullptr),
        m_light(nullptr),
        m_material(nullptr),
        m_model(new ShaderSchemeModel),
        m_builder(new ShaderBuilder()),
        m_controller(new CameraCtrl),
        m_lastCommand(nullptr) {

    ui->setupUi(this);
    m_controller->blockMovement(true);
    m_controller->setFree(false);

    SceneGraph *scene = Engine::objectCreate<SceneGraph>("SceneGraph");

    ui->preview->init();
    ui->preview->setController(m_controller);
    ui->preview->setSceneGraph(scene);

    m_light = Engine::composeActor("DirectLight", "LightSource", scene);
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    m_mesh = Engine::composeActor("MeshRender", "MeshRender", scene);

    on_actionSphere_triggered();

    connect(m_model, &ShaderSchemeModel::schemeUpdated, this, &MaterialEdit::onSchemeUpdated);
    connect(ui->schemeWidget, &SchemeView::itemSelected, this, &MaterialEdit::itemSelected);

    ui->schemeWidget->setModel(m_model);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

MaterialEdit::~MaterialEdit() {
    delete ui;

    delete m_mesh;
    delete m_light;
}

bool MaterialEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(m_builder) != m_lastCommand);
}

QStringList MaterialEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_builder)->suffixes();
}

void MaterialEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void MaterialEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        m_settings = { settings };

        m_material = Engine::objectCreate<Material>();
        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            mesh->setMaterial(m_material);
        }
        m_model->load(m_settings.first()->source());

        m_lastCommand = UndoManager::instance()->lastCommand(m_builder);

        ui->schemeWidget->onNodesSelected(QVariantList({0}));
    }
}

void MaterialEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_model->save(path.isEmpty() ? m_settings.first()->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(m_builder);
    }
}

void MaterialEdit::onSchemeUpdated() {
    if(m_builder && m_model->buildGraph()) {
        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            VariantMap map = m_model->data(true);
            m_material->loadUserData(map);
        }
    }
}

void MaterialEdit::changeMesh(const string &path) {
    MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
    if(mesh) {
        mesh->setMesh(Engine::loadResource<Mesh>(path));
        if(m_material) {
            mesh->setMaterial(m_material);
        }
        float bottom;
        m_controller->setFocusOn(m_mesh, bottom);
    }
}

void MaterialEdit::on_actionPlane_triggered() {
    changeMesh(".embedded/plane.fbx/Plane001");
}

void MaterialEdit::on_actionCube_triggered() {
    changeMesh(".embedded/cube.fbx/Box001");
}

void MaterialEdit::on_actionSphere_triggered() {
    changeMesh(".embedded/sphere.fbx/Sphere001");
}

void MaterialEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
