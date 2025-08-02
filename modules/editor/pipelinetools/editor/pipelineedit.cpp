#include "pipelineedit.h"
#include "ui_pipelineedit.h"

#include <QSettings>

#include <QMenu>

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
#include <json.h>

#include "../converter/pipelineconverter.h"
#include "../converter/pipelinetaskgraph.h"

namespace {
    const char *gMeshRender("MeshRender");
    const char *gDirectLight("DirectLight");
};

class PipelineProxy : public Object {
    A_OBJECT(PipelineProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(PipelineProxy::onGraphUpdated)
    )
public:
    void setEditor(PipelineEdit *editor) {
        m_editor = editor;
    }

    void onGraphUpdated() {
        m_editor->onGraphUpdated();
    }

private:
    PipelineEdit *m_editor = nullptr;;
};

PipelineEdit::PipelineEdit() :
        ui(new Ui::PipelineEdit),
        m_graph(new PipelineTaskGraph),
        m_builder(new PipelineConverter()),
        m_controller(new CameraController),
        m_lastCommand(nullptr),
        m_proxy(new PipelineProxy()) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

    ui->preview->setController(m_controller);
    ui->preview->setWorld(Engine::objectCreate<World>("World"));
    ui->preview->init(); // must be called after all options set
    ui->preview->setGizmoEnabled(false);
    ui->preview->setGridEnabled(false);

    ui->preview->hide();

    Object::connect(m_graph, _SIGNAL(graphUpdated()), m_proxy, _SLOT(onGraphUpdated()));

    connect(ui->schemeWidget, &GraphView::objectsSelected, this, &PipelineEdit::objectsSelected);

    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
    ui->schemeWidget->init(); // must be called after all options set

    readSettings();
}

PipelineEdit::~PipelineEdit() {
    writeSettings();

    delete ui;
}

void PipelineEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("pipeline.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void PipelineEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("pipeline.geometry", ui->splitter->saveState());
}

bool PipelineEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(this) != m_lastCommand);
}

StringList PipelineEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_builder)->suffixes();
}

void PipelineEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void PipelineEdit::onCutAction() {
    ui->schemeWidget->onCutAction();
}

void PipelineEdit::onCopyAction() {
    ui->schemeWidget->onCopyAction();
}

void PipelineEdit::onPasteAction() {
    ui->schemeWidget->onPasteAction();
}

bool PipelineEdit::isCopyActionAvailable() const {
    return ui->schemeWidget->isCopyActionAvailable();
}

bool PipelineEdit::isPasteActionAvailable() const {
    return ui->schemeWidget->isPasteActionAvailable();
}

void PipelineEdit::loadAsset(AssetConverterSettings *settings) {
    if(std::find(m_settings.begin(), m_settings.end(), settings) == m_settings.end()) {
        AssetEditor::loadAsset(settings);

        m_graph->load(m_settings.front()->source());

        m_lastCommand = UndoManager::instance()->lastCommand(this);
    }
}

void PipelineEdit::saveAsset(const TString &path) {
    if(!path.isEmpty() || !m_settings.front()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.front()->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(this);
    }
}

void PipelineEdit::onGraphUpdated() {
    if(m_builder && m_graph->buildGraph()) {
        // Need to attach it
        m_graph->data();
    }

    emit updated();
}

void PipelineEdit::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    ui->schemeWidget->onObjectsChanged(objects, property, value);
}

void PipelineEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
