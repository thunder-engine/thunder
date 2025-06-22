#include "animationedit.h"
#include "ui_animationedit.h"

#include <QMenu>
#include <QWidgetAction>

#include <components/world.h>
#include <components/animator.h>
#include <resources/animationstatemachine.h>

#include "../converter/animationbuilder.h"

AnimationEdit::AnimationEdit() :
        ui(new Ui::AnimationEdit),
        m_graph(new AnimationControllerGraph),
        m_assetConverter(new AnimationControllerBuilder),
        m_stateMachine(nullptr),
        m_lastCommand(nullptr) {

    ui->setupUi(this);

    connect(m_graph, &AnimationControllerGraph::graphUpdated, this, &AnimationEdit::updated);
    connect(ui->schemeWidget, &GraphView::objectsSelected, this, &AnimationEdit::objectsSelected);

    ui->schemeWidget->init();
    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
}

AnimationEdit::~AnimationEdit() {
    delete ui;
}

bool AnimationEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(m_graph) != m_lastCommand);
}

QStringList AnimationEdit::suffixes() const {
    return m_assetConverter->suffixes();
}

void AnimationEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void AnimationEdit::onObjectsChanged(const std::list<Object *> &objects, QString property, const Variant &value) {
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

bool AnimationEdit::isCopyActionAvailable() const {
    return ui->schemeWidget->isCopyActionAvailable();
}

bool AnimationEdit::isPasteActionAvailable() const {
    return ui->schemeWidget->isPasteActionAvailable();
}

void AnimationEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        m_stateMachine = Engine::loadResource<AnimationStateMachine>(qPrintable(settings->destination()));

        m_graph->load(settings->source().toStdString());

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void AnimationEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.first()->source().toStdString() : path.toStdString());

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void AnimationEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
