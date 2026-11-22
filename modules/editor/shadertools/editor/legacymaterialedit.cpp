#include "legacymaterialedit.h"
#include "ui_legacymaterialedit.h"

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

#include <viewport/legacycameracontroller.h>
#include <editor/undostack.h>
#include <editor/legacyeditorgadget.h>

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

class MaterialProxy : public Object {
    A_OBJECT(MaterialProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(MaterialProxy::onGraphUpdated)
    )
public:
    void setEditor(LegacyMaterialEdit *editor) {
        m_editor = editor;
    }

    void onGraphUpdated() {
        m_editor->onGraphUpdated();
    }

private:
    LegacyMaterialEdit *m_editor = nullptr;

};

LegacyMaterialEdit::LegacyMaterialEdit() :
        ui(new Ui::LegacyMaterialEdit),
        m_mesh(nullptr),
        m_light(nullptr),
        m_material(nullptr),
        m_graph(new ShaderGraph),
        m_builder(new ShaderBuilder),
        m_controller(new LegacyCameraController),
        m_proxy(new MaterialProxy) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

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

    m_light = Engine::composeActor<DirectLight>(gDirectLight, scene);
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    m_mesh = Engine::composeActor<MeshRender>(gMeshRender, scene);

    on_actionSphere_triggered();

    m_material = Engine::objectCreate<Material>();

    Object::connect(m_graph, _SIGNAL(graphUpdated()), m_proxy, _SLOT(onGraphUpdated()));

    connect(ui->schemeWidget, &LegacyGraphView::selectionChanged, this, &LegacyMaterialEdit::selectionChanged);
    connect(ui->schemeWidget, &LegacyGraphView::selectionChanged, this, &LegacyMaterialEdit::copyPasteChanged);
    connect(ui->schemeWidget, &LegacyGraphView::copied, this, &LegacyMaterialEdit::copyPasteChanged);

    ui->schemeWidget->setEditor(this);
    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
    ui->schemeWidget->init(); // must be called after all options set
    ui->schemeWidget->addRenderTask(new PreviewRender(m_graph));

    readSettings();
}

LegacyMaterialEdit::~LegacyMaterialEdit() {
    writeSettings();

    delete m_graph;
    m_graph = nullptr;

    delete m_builder;
    m_builder = nullptr;

    delete m_controller;
    m_controller = nullptr;

    delete m_proxy;
    m_proxy = nullptr;

    delete m_light->world();
    m_light = nullptr;

    delete m_material;
    m_material = nullptr;

    delete ui;
}

void LegacyMaterialEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("material.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void LegacyMaterialEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("material.geometry", ui->splitter->saveState());
}

StringList LegacyMaterialEdit::suffixes() const {
    return {"mtl"}; // Only mtl format represented as node graph to edit
}

TString LegacyMaterialEdit::assetType() const {
    return "Material";
}

void LegacyMaterialEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void LegacyMaterialEdit::onCutAction() {
    ui->schemeWidget->onCutAction();
}

void LegacyMaterialEdit::onCopyAction() {
    ui->schemeWidget->onCopyAction();
}

void LegacyMaterialEdit::onPasteAction() {
    ui->schemeWidget->onPasteAction();
}

bool LegacyMaterialEdit::isCopyActionAvailable() const {
    return ui->schemeWidget->isCopyActionAvailable();
}

bool LegacyMaterialEdit::isPasteActionAvailable() const {
    return ui->schemeWidget->isPasteActionAvailable();
}

void LegacyMaterialEdit::onObjectsChanged(const TString &property, const Variant &value) {
    LegacyEditorGadget *gadget = dynamic_cast<LegacyEditorGadget *>(sender());
    if(gadget) {
        ui->schemeWidget->onObjectsChanged(gadget->selected(), property, value);
    }
}

void LegacyMaterialEdit::loadAsset(AssetConverterSettings *settings) {
    if(std::find(m_settings.begin(), m_settings.end(), settings) == m_settings.end()) {
        LegacyAssetEditor::loadAsset(settings);

        m_graph->load(m_settings.front()->source());

        ui->schemeWidget->setGraph(m_graph);

        MeshRender *mesh = m_mesh->getComponent<MeshRender>();
        if(mesh) {
            mesh->setMaterial(m_material);
        }
    }
}

void LegacyMaterialEdit::saveAsset(const TString &path) {
    if(!path.isEmpty() || !m_settings.front()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.front()->source() : path);

        m_undoRedo->setClean();
    }
}

void LegacyMaterialEdit::onGraphUpdated() {
    if(m_builder && m_graph->buildGraph()) {
        VariantMap data = m_graph->data(true);
        m_codeDlg.setData(data);

        if(ShaderBuilder::compileData(ShaderBuilder::currentRhi(), data)) {
            if(ShaderBuilder::packShaderData(data, ShaderBuilder::currentRhi())) {
                ResourceSystem::loadResourceData(m_material, data);

                MeshRender *mesh = m_mesh->getComponent<MeshRender>();
                if(mesh) {
                    MaterialInstance *instance = mesh->materialInstance(0);
                    m_material->initInstance(instance);
                }
            }
        }
    }

    emit updated();
}

void LegacyMaterialEdit::changeMesh(Mesh *mesh) {
    MeshRender *render = m_mesh->getComponent<MeshRender>();
    if(render) {
        render->setMesh(mesh);
        if(m_material) {
            render->setMaterial(m_material);
        }
        float bottom;
        m_controller->setFocusOn(m_mesh, bottom);
    }
}

void LegacyMaterialEdit::on_actionPlane_triggered() {
    changeMesh(PipelineContext::defaultPlane());
}

void LegacyMaterialEdit::on_actionCube_triggered() {
    changeMesh(PipelineContext::defaultCube());
}

void LegacyMaterialEdit::on_actionSphere_triggered() {
    changeMesh(Engine::loadResource<Mesh>(".editor/sphere.fbx/Sphere001"));
}

void LegacyMaterialEdit::on_actionCode_triggered() {
    m_codeDlg.setData(m_graph->data(true));
    m_codeDlg.setVisible(true);
}

void LegacyMaterialEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

Object::ObjectList LegacyMaterialEdit::selected() const {
    return ui->schemeWidget->selected();
}
