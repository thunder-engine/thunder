#include "animationedit.h"
#include "ui_animationedit.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QMessageBox>

#include "animationbuilder.h"

#include <components/animationcontroller.h>
#include <resources/animationstatemachine.h>

AnimationEdit::AnimationEdit() :
        QMainWindow(nullptr),
        m_Modified(false),
        ui(new Ui::AnimationEdit),
        m_pBuilder(new AnimationBuilder()),
        m_pMachine(nullptr) {

    ui->setupUi(this);

    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateTemplate()));

    ui->centralwidget->addToolWindow(ui->quickWidget, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(ui->treeView, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(ui->quickWidget));
    ui->centralwidget->addToolWindow(ui->components, QToolWindowManager::ReferenceRightOf, ui->centralwidget->areaFor(ui->quickWidget));

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = ui->menuWindow->addAction(it->windowTitle());
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }
    ui->components->setModel(m_pBuilder->components());

    ui->quickWidget->rootContext()->setContextProperty("schemeModel", m_pBuilder);
    ui->quickWidget->rootContext()->setContextProperty("stateMachine", true);
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    ui->treeView->setWindowTitle("Properties");
    ui->quickWidget->setWindowTitle("Scheme");

    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));

    m_pUndo = UndoManager::instance()->createUndoAction(ui->menuEdit);
    m_pUndo->setShortcut(QKeySequence("Ctrl+Z"));
    ui->menuEdit->addAction(m_pUndo);

    m_pRedo = UndoManager::instance()->createRedoAction(ui->menuEdit);
    m_pRedo->setShortcut(QKeySequence("Ctrl+Y"));
    ui->menuEdit->addAction(m_pRedo);

    readSettings();
}

AnimationEdit::~AnimationEdit() {
    delete ui;
}

void AnimationEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value("animation.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("animation.windows"));
}

void AnimationEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("animation.geometry", saveGeometry());
    settings.setValue("animation.windows", ui->centralwidget->saveState());
}

void AnimationEdit::closeEvent(QCloseEvent *event) {
    writeSettings();

    if(isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The scheme has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if(result == QMessageBox::Yes) {
            on_actionSave_triggered();
        }
    }
}

bool AnimationEdit::isModified() const {
    return m_Modified;
}

void AnimationEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_pBuilder->node(list.front().toInt());
        if(node) {
            ui->treeView->setObject(static_cast<QObject *>(node->ptr));
        }
    }
}

void AnimationEdit::loadAsset(IConverterSettings *settings) {
    show();
    if(m_Path != settings->source()) {
        m_Path = settings->source();
        m_pMachine = Engine::loadResource<AnimationStateMachine>(settings->destination());

        m_pBuilder->load(m_Path);

        onUpdateTemplate(false);
        onNodesSelected(QVariantList({0}));
    }
}

void AnimationEdit::onUpdateTemplate(bool update) {
    if(m_pBuilder) {
        m_Modified = update;
    }
}

void AnimationEdit::on_actionSave_triggered() {
    if(!m_Path.isEmpty()) {
        m_pBuilder->save(m_Path);
        m_Modified = false;
    }
}

void AnimationEdit::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void AnimationEdit::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}
