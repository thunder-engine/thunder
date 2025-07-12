#include "particleedit.h"
#include "ui_particleedit.h"

#include <QSettings>
#include <QToolButton>
#include <QMenu>

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

#include "actions/createmodule.h"
#include "actions/deletemodule.h"

namespace {
    const char *gFunction("function");
}

Q_DECLARE_METATYPE(Object *)

ParticleEdit::ParticleEdit() :
        ui(new Ui::ParticleEdit),
        m_builder(new EffectBuilder),
        m_controller(new CameraController()),
        m_effect(nullptr),
        m_render(nullptr),
        m_lastCommand(nullptr),
        m_moduleButton(nullptr) {

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

    connect(ui->graph, &GraphView::objectsSelected, this, &ParticleEdit::objectsSelected);
    connect(m_builder, &EffectBuilder::effectUpdated, this, &ParticleEdit::onUpdateTemplate);

    EffectGraph *graph = &m_builder->graph();

    ui->graph->setWorld(Engine::objectCreate<World>("World"));
    ui->graph->setGraph(graph);
    ui->graph->init();

    connect(graph, &EffectGraph::moduleChanged, ui->graph, &GraphView::reselect);
    connect(graph, &EffectGraph::graphUpdated, this, &ParticleEdit::updated);

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

void ParticleEdit::onCutAction() {
    ui->graph->onCutAction();
}

void ParticleEdit::onCopyAction() {
    ui->graph->onCopyAction();
}

void ParticleEdit::onPasteAction() {
    ui->graph->onPasteAction();
}

bool ParticleEdit::isCopyActionAvailable() const {
    return ui->graph->isCopyActionAvailable();
}

bool ParticleEdit::isPasteActionAvailable() const {
    return ui->graph->isPasteActionAvailable();
}

void ParticleEdit::onAddModule(QAction *action) {
    QString name = tr("Create %1").arg(action->text());
    EffectGraph *graph = &m_builder->graph();
    UndoManager::instance()->push(new CreateModule(action->text().toStdString(), graph, name));
}

void ParticleEdit::onObjectsChanged(const Object::ObjectList &objects, QString property, const Variant &value) {
    ui->graph->onObjectsChanged(objects, property, value);
}

std::list<QWidget *> ParticleEdit::createActionWidgets(Object *object, QWidget *parent) const {
    std::list<QWidget *> result;

    if(dynamic_cast<EffectModule *>(object)) {
        QToolButton *button = new QToolButton(parent);
        button->setProperty(gFunction, QVariant::fromValue(object));
        button->setIconSize(QSize(12, 12));

        button->setIcon(QIcon(":/icons/remove.png"));
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onDeleteModule()));

        result.push_back(button);
    }

    return result;
}

QWidget *ParticleEdit::propertiesWidget() {
    if(m_moduleButton == nullptr) {
        m_moduleButton = new QToolButton;

        m_moduleButton->setProperty("blue", true);
        m_moduleButton->setPopupMode(QToolButton::InstantPopup);
        m_moduleButton->setText(tr("Add Modificator"));
        m_moduleButton->setToolTip(tr("Adds a new Modificator to this Emitter."));
        m_moduleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_moduleButton->setMinimumHeight(25);
        m_moduleButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        QMenu *rootMenu = new QMenu;
        for(auto it : m_builder->graph().modules()) {
            QMenu *menu = rootMenu;
            QString str(it.c_str());
            QStringList list = str.split('/');
            for(int i = 0; i < list.size(); i++) {
                if(i == list.size() - 1) {
                    menu->addAction(list.at(i));
                } else {
                    bool found = false;
                    for(auto it : menu->actions()) {
                        if(it->text() == list.at(i)) {
                            if(it->menu()) {
                                found = true;
                                menu = it->menu();
                                break;
                            }
                        }
                    }
                    if(!found) {
                        menu = menu->addMenu(list.at(i));
                    }
                }
            }
        }

        connect(rootMenu, &QMenu::triggered, this, &ParticleEdit::onAddModule);

        m_moduleButton->setMenu(rootMenu);
    }
    return m_moduleButton;
}

void ParticleEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        std::string dest = settings->destination().toStdString();
        VisualEffect *effect = Engine::loadResource<VisualEffect>(dest);
        m_render->setEffect(effect);

        EffectGraph &graph = m_builder->graph();
        graph.load(settings->source().toStdString());

        m_lastCommand = UndoManager::instance()->lastCommand(&m_builder->graph());

        onUpdateTemplate();
    }
}

void ParticleEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_builder->graph().save(path.isEmpty() ? m_settings.first()->source().toStdString() : path.toStdString());

        m_lastCommand = UndoManager::instance()->lastCommand(&m_builder->graph());
    }
}

void ParticleEdit::onUpdateTemplate() {
    VisualEffect *effect = m_render->effect();
    if(effect) {
        effect->loadUserData(m_builder->graph().data());
        m_render->setEffect(effect);

        emit updated();
    }
}

void ParticleEdit::onDeleteModule() {
    EffectModule *module = static_cast<EffectModule *>(sender()->property(gFunction).value<Object *>());

    QString name = tr("Delete %1").arg(module->name().c_str());

    EffectGraph *graph = &m_builder->graph();
    UndoManager::instance()->push(new DeleteModule(module, graph, name));
}

void ParticleEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
