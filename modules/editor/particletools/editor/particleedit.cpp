#include "particleedit.h"
#include "ui_particleedit.h"

#include <QSettings>
#include <QToolButton>

#include <global.h>

#include <editor/viewport/cameracontroller.h>

#include <components/world.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/effectrender.h>
#include <components/camera.h>

#include "effectbuilder.h"
#include "effectrootnode.h"
#include "effectmodule.h"

namespace {
    const char *gFunction("function");
}

ParticleEdit::ParticleEdit() :
        ui(new Ui::ParticleEdit),
        m_builder(new EffectBuilder),
        m_controller(new CameraController()),
        m_effect(nullptr),
        m_render(nullptr),
        m_lastCommand(nullptr) {

    ui->setupUi(this);

    m_controller->blockMovement(true);
    m_controller->setFree(false);

    World *world = Engine::objectCreate<World>("World");
    Scene *scene = Engine::objectCreate<Scene>("Scene", world);

    ui->preview->setController(m_controller);
    ui->preview->setWorld(world);
    ui->preview->init(); // must be called after all options set

    m_effect = Engine::composeActor("EffectRender", "ParticleEffect", scene);
    m_render = m_effect->getComponent<EffectRender>();

    connect(ui->graph, &GraphView::itemsSelected, this, &ParticleEdit::itemsSelected);
    connect(m_builder, &EffectBuilder::effectUpdated, this, &ParticleEdit::onUpdateTemplate);

    EffectGraph *graph = &m_builder->graph();

    ui->graph->setWorld(Engine::objectCreate<World>("World"));
    ui->graph->setGraph(graph);
    ui->graph->init();

    connect(graph, &EffectGraph::moduleChanged, ui->graph, &GraphView::reselect);

    startTimer(16);

    readSettings();
}

ParticleEdit::~ParticleEdit() {
    writeSettings();

    delete ui;

    delete m_effect;
}

void ParticleEdit::timerEvent(QTimerEvent *) {
    if(m_render && ui->preview->isVisible()) {
        Camera::setCurrent(m_controller->camera());
        m_render->deltaUpdate(1.0f / 60.0f);
        Camera::setCurrent(nullptr);
    }
}

void ParticleEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("effects.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void ParticleEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("effects.geometry", ui->splitter->saveState());
}

bool ParticleEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(&m_builder->graph()) != m_lastCommand);
}

QStringList ParticleEdit::suffixes() const {
    return m_builder->suffixes();
}

void ParticleEdit::onActivated() {
    ui->graph->reselect();
}

QList<QWidget *> ParticleEdit::createActionWidgets(QObject *object, QWidget *parent) const {
    QList<QWidget *> result;

    QToolButton *button = new QToolButton(parent);
    button->setProperty(gFunction, QVariant::fromValue(object));
    button->setIconSize(QSize(12, 12));

    if(dynamic_cast<EffectModule *>(object)) {
        button->setIcon(QIcon(":/icons/remove.png"));
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onDeleteModule()));
    } else {
        button->setIcon(QIcon(":/Style/styles/dark/icons/plus.png"));
        connect(button, SIGNAL(clicked(bool)), &m_builder->graph(), SLOT(showFunctionsMenu()));
    }

    result.push_back(button);

    return result;
}

void ParticleEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        m_render->setEffect(Engine::loadResource<ParticleEffect>(qPrintable(settings->destination())));
        m_builder->graph().load(settings->source());

        ui->graph->selectNode(m_builder->graph().rootNode());

        m_lastCommand = UndoManager::instance()->lastCommand(&m_builder->graph());

        onUpdateTemplate();
    }
}

void ParticleEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_builder->graph().save(path.isEmpty() ? m_settings.first()->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(&m_builder->graph());
    }
}

void ParticleEdit::onUpdateTemplate() {
    ParticleEffect *effect = m_render->effect();
    if(effect) {
        effect->loadUserData(m_builder->graph().data());
        m_render->setEffect(effect);

        emit updated();
    }
}

void ParticleEdit::onDeleteModule() {
    EffectModule *module = static_cast<EffectModule *>(sender()->property(gFunction).value<QObject *>());

    EffectRootNode *root = static_cast<EffectRootNode *>(module->parent());
    if(root) {
        root->removeModule(module);
    }

    ui->graph->reselect();
}

void ParticleEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
