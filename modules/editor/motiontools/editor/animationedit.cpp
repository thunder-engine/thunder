#include "animationedit.h"
#include "ui_animationedit.h"

#include <components/world.h>
#include <components/actor.h>
#include <components/animator.h>
#include <components/armature.h>
#include <components/transform.h>
#include <components/directlight.h>
#include <resources/animationstatemachine.h>

#include <editor/viewport/cameracontroller.h>

#include <QToolButton>
#include <QSettings>

#include "renamedialog.h"
#include "actions/addvariable.h"
#include "actions/removevariable.h"
#include "actions/renamevariable.h"
#include "../converter/animationbuilder.h"

#include "statelink.h"
#include "basestate.h"

namespace {
    const char *gProperty("property");
    const char *gDirectLight("DirectLight");
};

class AnimationProxy : public Object {
    A_OBJECT(AnimationProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(AnimationProxy::onGraphUpdated),
        A_SLOT(AnimationProxy::onVariableChanged)
    )
public:
    void setEditor(AnimationEdit *editor) {
        m_editor = editor;
    }

    void onGraphUpdated() {
        m_editor->onGraphUpdated();
    }

    void onVariableChanged() {
        m_editor->onActivated();
    }

private:
    AnimationEdit *m_editor = nullptr;

};

AnimationEdit::AnimationEdit() :
        ui(new Ui::AnimationEdit),
        m_graph(new AnimationControllerGraph),
        m_builder(new AnimationControllerBuilder),
        m_controller(new CameraController),
        m_light(nullptr),
        m_animator(nullptr),
        m_armature(nullptr),
        m_stateMachine(Engine::objectCreate<AnimationStateMachine>()),
        m_proxy(new AnimationProxy),
        m_variableButton(nullptr) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

    m_controller->blockMovement(true);
    m_controller->setFree(false);

    World *world = Engine::objectCreate<World>("World");
    Scene *scene = Engine::objectCreate<Scene>("Scene", world);

    ui->preview->setController(m_controller);
    ui->preview->setWorld(world);
    ui->preview->init(); // must be called after all options set
    //ui->preview->showGizmos(false);

    m_light = Engine::composeActor<DirectLight>(gDirectLight, scene);
    m_light->transform()->setPosition(Vector3(0.0f, 5.0f, 0.0f));
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    Object::connect(m_graph, _SIGNAL(graphUpdated()), m_proxy, _SLOT(onGraphUpdated()));
    Object::connect(m_graph, _SIGNAL(variableChanged()), m_proxy, _SLOT(onVariableChanged()));

    connect(ui->schemeWidget, &GraphView::selectionChanged, this, &AnimationEdit::selectionChanged);

    ui->schemeWidget->setEditor(this);
    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
    ui->schemeWidget->init();

    QAction *renAction = m_variablesMenu.addAction(tr("Rename Variable"));
    connect(renAction, &QAction::triggered, this, &AnimationEdit::onRenameVariable);

    QAction *delAction = m_variablesMenu.addAction(tr("Delete Variable"));
    connect(delAction, &QAction::triggered, this, &AnimationEdit::onDeleteVariable);

    readSettings();

    startTimer(16);
}

AnimationEdit::~AnimationEdit() {
    writeSettings();

    delete m_graph;
    m_graph = nullptr;

    delete m_builder;
    m_builder = nullptr;

    delete m_controller;
    m_controller = nullptr;

    delete m_light->world();
    m_light = nullptr;
    m_animator = nullptr;
    m_armature = nullptr;

    delete m_stateMachine;
    m_stateMachine = nullptr;

    delete m_proxy;
    m_proxy = nullptr;

    delete ui;
}

void AnimationEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("animation.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void AnimationEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("animation.geometry", ui->splitter->saveState());
}

void AnimationEdit::timerEvent(QTimerEvent *) {
    if(ui->preview->isVisible()) {
        if(m_animator) {
            m_animator->update();
        }

        if(m_armature) {
            m_armature->update();
        }
    }
}

StringList AnimationEdit::suffixes() const {
    StringList result;
    for(auto &it : m_builder->suffixes()) {
        result.push_back(it);
    }
    return result;
}

TString AnimationEdit::assetType() const {
    return "Animation";
}

void AnimationEdit::onAddVariable(QAction *action) {
    TString name;
    Variant value;
    switch(action->data().toInt()) {
        case MetaType::BOOLEAN: name = "New Bool"; value = false; break;
        case MetaType::INTEGER: name = "New Int"; value = 0; break;
        case MetaType::FLOAT: name = "New Float"; value = 0.0f; break;
        default: break;
    }

    m_undoRedo->push(new AddVariable(name, value, m_graph));
}

void AnimationEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void AnimationEdit::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    ui->schemeWidget->onObjectsChanged(objects, property, value);
}

void AnimationEdit::onCutAction() {
    ui->schemeWidget->onCutAction();
}

void AnimationEdit::onCopyAction() {
    ui->schemeWidget->onCopyAction();
}

void AnimationEdit::onPasteAction() {
    ui->schemeWidget->onPasteAction();
}

QWidget *AnimationEdit::propertiesWidget() {
    if(m_variableButton == nullptr) {
        m_variableButton = new QToolButton;

        m_variableButton->setProperty("blue", true);
        m_variableButton->setPopupMode(QToolButton::InstantPopup);
        m_variableButton->setText(tr("Add Variable"));
        m_variableButton->setToolTip(tr("Adds a new Animator state machine variable."));
        m_variableButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_variableButton->setMinimumHeight(25);
        m_variableButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        QMenu *rootMenu = new QMenu;
        rootMenu->addAction("Boolean")->setData(MetaType::BOOLEAN);
        rootMenu->addAction("Integer")->setData(MetaType::INTEGER);
        rootMenu->addAction("Float")->setData(MetaType::FLOAT);

        connect(rootMenu, &QMenu::triggered, this, &AnimationEdit::onAddVariable);

        m_variableButton->setMenu(rootMenu);
    }

    return m_variableButton;
}

QMenu *AnimationEdit::propertyContextMenu(Object *object, const TString &property) {
    if(object == m_graph) {
        m_variablesMenu.setProperty(gProperty, property.data());
        return &m_variablesMenu;
    }
    return nullptr;
}

void AnimationEdit::onSelectionChanged() {
    Object::ObjectList objects = ui->schemeWidget->selected();
    if(!objects.empty()) {
        if(m_variableButton) {
            m_variableButton->setVisible(objects.front() == m_graph);
        }

        StateLink *link = dynamic_cast<StateLink *>(objects.front());
        if(link) {
            Object::connect(link, _SIGNAL(variableChanged()), m_proxy, _SLOT(onVariableChanged()));
        }

        BaseState *state = dynamic_cast<BaseState *>(objects.front());
        if(state && m_animator) {
            m_animator->setState(state->name());
        }
    }

    emit selectionChanged();
}

void AnimationEdit::onRenameVariable() {
    QString oldName = m_variablesMenu.property(gProperty).toString();
    RenameDialog dlg;
    dlg.setText(oldName);

    if(dlg.exec() == QDialog::Accepted) {
        m_undoRedo->push(new RenameVariable(qPrintable(oldName), qPrintable(dlg.text()), m_graph));
    }
}

void AnimationEdit::onDeleteVariable() {
    m_undoRedo->push(new RemoveVariable(qPrintable(m_variablesMenu.property(gProperty).toString()), m_graph));
}

bool AnimationEdit::isCopyActionAvailable() const {
    return ui->schemeWidget->isCopyActionAvailable();
}

bool AnimationEdit::isPasteActionAvailable() const {
    return ui->schemeWidget->isPasteActionAvailable();
}

void AnimationEdit::loadAsset(AssetConverterSettings *settings) {
    if(std::find(m_settings.begin(), m_settings.end(), settings) == m_settings.end()) {
        LegacyAssetEditor::loadAsset(settings);

        if(m_animator) {
            delete m_animator->actor();
            m_animator = nullptr;
        }
        m_armature = nullptr;

        AnimationBuilderSettings *builderSettings = static_cast<AnimationBuilderSettings *>(settings);

        Prefab *prefab = Engine::loadResource<Prefab>(builderSettings->previewAsset());
        if(prefab) {
            Actor *prefabActor = prefab->actor();
            if(prefabActor) {
                Actor *actor = static_cast<Actor *>(prefabActor->clone(m_light->scene()));

                m_armature = actor->getComponent<Armature>();
                if(m_armature == nullptr) {
                    m_armature = actor->getComponentInChild<Armature>();
                }

                m_animator = actor->getComponent<Animator>();
                if(m_animator == nullptr) {
                    m_animator = static_cast<Animator *>(actor->addComponent("Animator"));
                }

                if(m_animator) {
                    m_animator->setStateMachine(m_stateMachine);
                } else {
                    delete actor;
                    m_armature = nullptr;
                }
            }
        }

        m_graph->load(settings->source());
    }
}

void AnimationEdit::saveAsset(const TString &path) {
    if(!path.isEmpty() || !m_settings.front()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.front()->source() : path);
    }
}

void AnimationEdit::onGraphUpdated() {
    if(m_builder) {
        m_stateMachine->loadUserData(m_graph->data());
    }

    emit updated();
}

void AnimationEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

Object::ObjectList AnimationEdit::selected() const {
    return ui->schemeWidget->selected();
}
