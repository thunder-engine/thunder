#include "materialedit.h"
#include "ui_materialedit.h"

#include <QSettings>

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
        m_graph(new ShaderNodeGraph),
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
    ui->preview->setGizmoEnabled(false);
    ui->preview->setGridEnabled(false);

    m_light = Engine::composeActor("DirectLight", "LightSource", scene);
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    m_mesh = Engine::composeActor("MeshRender", "MeshRender", scene);

    on_actionSphere_triggered();

    connect(m_graph, &ShaderNodeGraph::graphUpdated, this, &MaterialEdit::onGraphUpdated);
    connect(ui->schemeWidget, &GraphView::itemSelected, this, &MaterialEdit::itemSelected);

    ui->schemeWidget->init();
    ui->schemeWidget->setSceneGraph(Engine::objectCreate<SceneGraph>("SceneGraph"));
    ui->schemeWidget->setGraph(m_graph);

    readSettings();
}

MaterialEdit::~MaterialEdit() {
    writeSettings();

    delete ui;

    delete m_mesh;
    delete m_light;
}

void MaterialEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("material.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void MaterialEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("material.geometry", ui->splitter->saveState());
}

bool MaterialEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(m_graph) != m_lastCommand);
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
        m_graph->load(m_settings.first()->source());

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void MaterialEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.first()->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void MaterialEdit::onGraphUpdated() {
    if(m_builder && m_graph->buildGraph()) {
        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            VariantMap map = m_graph->data(true);
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
