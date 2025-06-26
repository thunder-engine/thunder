#include "materialedit.h"
#include "ui_materialedit.h"

#include <QSettings>

#include <engine.h>
#include <pipelinetask.h>
#include <pipelinecontext.h>
#include <commandbuffer.h>

#include <components/world.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>

#include <resources/mesh.h>

#include <editor/viewport/cameracontroller.h>

#include <global.h>

#include "../converter/shaderbuilder.h"

namespace {
    const char *gMeshRender("MeshRender");
    const char *gDirectLight("DirectLight");
};

class PreviewRender : public PipelineTask {
 public:
    PreviewRender(ShaderGraph *graph) :
        m_graph(graph) {
    }

private:
    void exec() override {
        CommandBuffer *buffer = m_context->buffer();
        if(m_graph) {
            buffer->setViewport(0, 0, 150, 150);
            m_graph->updatePreviews(*buffer);
        }
    }

private:
    ShaderGraph *m_graph;

};

MaterialEdit::MaterialEdit() :
        ui(new Ui::MaterialEdit),
        m_mesh(nullptr),
        m_light(nullptr),
        m_material(nullptr),
        m_graph(new ShaderGraph),
        m_builder(new ShaderBuilder),
        m_controller(new CameraController),
        m_lastCommand(nullptr) {

    ui->setupUi(this);
    m_controller->blockMovement(true);
    m_controller->setFree(false);

    World *world = Engine::objectCreate<World>("World");
    Scene *scene = Engine::objectCreate<Scene>("Scene", world);

    ui->preview->setController(m_controller);
    ui->preview->setWorld(world);
    ui->preview->init(); // must be called after all options set
    ui->preview->setGridEnabled(false);
    ui->preview->showCube(true);
    ui->preview->showGizmos(false);

    m_light = Engine::composeActor(gDirectLight, gDirectLight, scene);
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    m_mesh = Engine::composeActor(gMeshRender, gMeshRender, scene);

    on_actionSphere_triggered();

    m_material = Engine::objectCreate<Material>();

    connect(m_graph, &ShaderGraph::graphUpdated, this, &MaterialEdit::onGraphUpdated);
    connect(m_graph, &ShaderGraph::graphUpdated, this, &MaterialEdit::updated);
    connect(ui->schemeWidget, &GraphView::objectsSelected, this, &MaterialEdit::objectsSelected);
    connect(ui->schemeWidget, &GraphView::objectsSelected, this, &MaterialEdit::copyPasteChanged);
    connect(ui->schemeWidget, &GraphView::copied, this, &MaterialEdit::copyPasteChanged);

    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
    ui->schemeWidget->init(); // must be called after all options set
    ui->schemeWidget->addRenderTask(new PreviewRender(m_graph));

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
    return {"mtl"}; // Only mtl format represented as node graph to edit
}

void MaterialEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void MaterialEdit::onCutAction() {
    ui->schemeWidget->onCutAction();
}

void MaterialEdit::onCopyAction() {
    ui->schemeWidget->onCopyAction();
}

void MaterialEdit::onPasteAction() {
    ui->schemeWidget->onPasteAction();
}

bool MaterialEdit::isCopyActionAvailable() const {
    return ui->schemeWidget->isCopyActionAvailable();
}

bool MaterialEdit::isPasteActionAvailable() const {
    return ui->schemeWidget->isPasteActionAvailable();
}

void MaterialEdit::onObjectsChanged(const std::list<Object *> &objects, QString property, const Variant &value) {
    ui->schemeWidget->onObjectsChanged(objects, property, value);
}

void MaterialEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        m_graph->load(m_settings.first()->source().toStdString());

        ui->schemeWidget->setGraph(m_graph);

        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            mesh->setMaterial(m_material);
        }

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void MaterialEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.first()->source().toStdString() : path.toStdString());

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void MaterialEdit::onGraphUpdated() {
    if(m_builder && m_graph->buildGraph()) {
        VariantMap data = m_graph->data(true);
        m_codeDlg.setData(data);

        ShaderBuilder::compileData(data);
        m_material->loadUserData(data);

        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            MaterialInstance *instance = mesh->materialInstance();
            m_material->initInstance(instance);
        }
    }
}

void MaterialEdit::changeMesh(Mesh *mesh) {
    MeshRender *render = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
    if(render) {
        render->setMesh(mesh);
        if(m_material) {
            render->setMaterial(m_material);
        }
        float bottom;
        m_controller->setFocusOn(m_mesh, bottom);
    }
}

void MaterialEdit::on_actionPlane_triggered() {
    changeMesh(PipelineContext::defaultPlane());
}

void MaterialEdit::on_actionCube_triggered() {
    changeMesh(PipelineContext::defaultCube());
}

void MaterialEdit::on_actionSphere_triggered() {
    changeMesh(Engine::loadResource<Mesh>(".embedded/sphere.fbx/Sphere001"));
}

void MaterialEdit::on_actionCode_triggered() {
    m_codeDlg.setData(m_graph->data(true));
    m_codeDlg.setVisible(true);
}

void MaterialEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
