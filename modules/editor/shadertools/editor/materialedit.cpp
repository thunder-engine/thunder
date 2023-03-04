#include "materialedit.h"
#include "ui_materialedit.h"

#include <QSettings>

#include <QMenu>
#include <QTextEdit>

#include <engine.h>
#include <pipelinepass.h>
#include <pipelinecontext.h>
#include <commandbuffer.h>

#include <components/world.h>
#include <components/scene.h>
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
    const char *gDirectLight("DirectLight");
};

class PreviewRender : public PipelinePass {
public:
    enum Inputs {
        Source
    };

public:
    PreviewRender(ShaderNodeGraph *graph) :
        m_graph(graph) {
    }

private:
    uint32_t layer() const override {
        return CommandBuffer::RAYCAST;
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
        CommandBuffer *buffer = context->buffer();
        if(m_graph) {
            buffer->setViewport(0, 0, 128, 128);
            m_graph->updatePreviews(*buffer);
        }

        return source;
    }

private:
    ShaderNodeGraph *m_graph;

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

    World *world = Engine::objectCreate<World>("World");
    Scene *scene = Engine::objectCreate<Scene>("Scene", world);

    ui->preview->setController(m_controller);
    ui->preview->init();
    ui->preview->setWorld(world);
    ui->preview->setGizmoEnabled(false);
    ui->preview->setGridEnabled(false);

    m_light = Engine::composeActor(gDirectLight, gDirectLight, scene);
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    m_mesh = Engine::composeActor(gMeshRender, gMeshRender, scene);

    on_actionSphere_triggered();

    connect(m_graph, &ShaderNodeGraph::graphUpdated, this, &MaterialEdit::onGraphUpdated);
    connect(ui->schemeWidget, &GraphView::itemsSelected, this, &MaterialEdit::itemsSelected);

    ui->schemeWidget->init();
    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
    ui->schemeWidget->addPass(new PreviewRender(m_graph));

    readSettings();

    ui->plainTextEdit->setHidden(true);
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
        VariantMap data = m_graph->data(true);
        m_material->loadUserData(data);
        ui->plainTextEdit->setPlainText(data[SHADER].toString().c_str());
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

void MaterialEdit::on_actionCode_triggered(bool checked) {
    if(checked) {
        VariantMap data = m_graph->data(true);
        ui->plainTextEdit->setPlainText(data[SHADER].toString().c_str());

        ui->codeSplitter->setSizes({200, 100});
    }
    ui->plainTextEdit->setVisible(checked);
}

void MaterialEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
