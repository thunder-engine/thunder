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

PipelineEdit::PipelineEdit() :
        ui(new Ui::PipelineEdit),
        m_graph(new PipelineTaskGraph),
        m_builder(new PipelineConverter()),
        m_controller(new CameraController),
        m_lastCommand(nullptr) {

    ui->setupUi(this);

    ui->preview->setController(m_controller);
    ui->preview->setWorld(Engine::objectCreate<World>("World"));
    ui->preview->init(); // must be called after all options set
    ui->preview->setGizmoEnabled(false);
    ui->preview->setGridEnabled(false);

    ui->preview->hide();

    connect(m_graph, &PipelineTaskGraph::graphUpdated, this, &PipelineEdit::onGraphUpdated);
    connect(ui->schemeWidget, &GraphView::itemsSelected, this, &PipelineEdit::itemsSelected);

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
    return (UndoManager::instance()->lastCommand(m_graph) != m_lastCommand);
}

QStringList PipelineEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_builder)->suffixes();
}

void PipelineEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void PipelineEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        m_graph->load(m_settings.first()->source());

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void PipelineEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.first()->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void PipelineEdit::onGraphUpdated() {
    if(m_builder && m_graph->buildGraph()) {
        // Need to attach it
        m_graph->data();
    }
}

void PipelineEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
