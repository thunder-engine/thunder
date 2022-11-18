#include "animationedit.h"
#include "ui_animationedit.h"

#include <QMenu>
#include <QWidgetAction>

#include "animationbuilder.h"

#include <components/scenegraph.h>
#include <components/animator.h>
#include <resources/animationstatemachine.h>

AnimationEdit::AnimationEdit() :
        ui(new Ui::AnimationEdit),
        m_graph(new AnimationNodeGraph),
        m_assetConverter(new AnimationBuilder),
        m_stateMachine(nullptr),
        m_lastCommand(nullptr) {

    ui->setupUi(this);

    connect(ui->schemeWidget, &GraphView::itemSelected, this, &AnimationEdit::itemSelected);

    ui->schemeWidget->init();
    ui->schemeWidget->setSceneGraph(Engine::objectCreate<SceneGraph>("SceneGraph"));
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

void AnimationEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        m_settings = { settings };

        m_stateMachine = Engine::loadResource<AnimationStateMachine>(qPrintable(settings->destination()));

        m_graph->load(settings->source());

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void AnimationEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.first()->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(m_graph);
    }
}

void AnimationEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
