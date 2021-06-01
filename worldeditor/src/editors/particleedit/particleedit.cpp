#include "particleedit.h"
#include "ui_particleedit.h"

#include <QSettings>
#include <QQmlContext>
#include <QQuickItem>
#include <QQmlEngine>
#include <QMessageBox>

#include "effectconverter.h"

#include <global.h>

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/viewport.h"

#include <components/scene.h>
#include <components/actor.h>
#include <components/particlerender.h>

ParticleEdit::ParticleEdit(DocumentModel *document) :
        QMainWindow(nullptr),
        m_Modified(false),
        ui(new Ui::ParticleEdit),
        m_pEditor(nullptr),
        m_pEffect(nullptr),
        m_pBuilder(new EffectConverter),
        m_pRender(nullptr),
        m_pDocument(document) {

    ui->setupUi(this);

    CameraCtrl *ctrl = new CameraCtrl(ui->glWidget);
    ctrl->blockMovement(true);
    ctrl->setFree(false);
    ctrl->init(nullptr);
    ui->glWidget->setController(ctrl);
    ui->glWidget->setScene( Engine::objectCreate<Scene>("Scene") );
    ui->glWidget->setObjectName("Preview");

    ui->glWidget->setWindowTitle("Preview");
    ui->treeView->setWindowTitle("Properties");
    ui->quickWidget->setWindowTitle("Scheme");

    m_pEffect = Engine::composeActor("ParticleRender", "ParticleEffect", ui->glWidget->scene());
    m_pRender = static_cast<ParticleRender *>(m_pEffect->component("ParticleRender"));

    startTimer(16);

    ui->centralwidget->addToolWindow(ui->quickWidget, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(ui->glWidget, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(ui->quickWidget));
    ui->centralwidget->addToolWindow(ui->treeView, QToolWindowManager::ReferenceBottomOf, ui->centralwidget->areaFor(ui->glWidget));

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = ui->menuWindow->addAction(it->windowTitle());
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }
    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));
    connect(m_pBuilder, SIGNAL(effectUpdated()), this, SLOT(onUpdateTemplate()));

    ui->quickWidget->rootContext()->setContextProperty("effectModel", QVariant::fromValue(m_pBuilder->children()));
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/Emitters.qml"));

    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(emitterSelected(QString)), this, SLOT(onEmitterSelected(QString)));
    connect(item, SIGNAL(emitterCreate()), this, SLOT(onEmitterCreated()));
    connect(item, SIGNAL(emitterDelete(QString)), this, SLOT(onEmitterDeleted(QString)));

    connect(item, SIGNAL(functionSelected(QString, QString)), this, SLOT(onFunctionSelected(QString, QString)));
    connect(item, SIGNAL(functionCreate(QString, QString)), this, SLOT(onFunctionCreated(QString, QString)));
    connect(item, SIGNAL(functionDelete(QString, QString)), this, SLOT(onFunctionDeleted(QString, QString)));

    readSettings();
}

ParticleEdit::~ParticleEdit() {
    writeSettings();

    delete ui;

    delete m_pEditor;

    delete m_pEffect;
}

void ParticleEdit::timerEvent(QTimerEvent *) {
    if(m_pRender) {
        static_cast<NativeBehaviour *>(m_pRender)->update();
    }
}

bool ParticleEdit::isModified() const {
    return m_Modified;
}

QStringList ParticleEdit::assetTypes() const {
    return {"ParticleEffect"};
}

void ParticleEdit::readSettings() {
    QSettings settings(COMPANY_NAME, PRODUCT_NAME);
    restoreGeometry(settings.value("particle.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("particle.windows"));
}

void ParticleEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, PRODUCT_NAME);
    settings.setValue("particle.geometry", saveGeometry());
    settings.setValue("particle.windows", ui->centralwidget->saveState());
}

void ParticleEdit::closeEvent(QCloseEvent *event) {
    writeSettings();

    if(isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The effect has been modified.");
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
    QDir dir(ProjectManager::instance()->contentPath());
    m_pDocument->closeFile(dir.relativeFilePath(m_Path));
}

void ParticleEdit::loadAsset(IConverterSettings *settings) {
    show();

    m_Path = settings->source();

    m_pRender->setEffect(Engine::loadResource<ParticleEffect>(qPrintable(settings->destination())));
    m_pBuilder->load(m_Path);

    ui->treeView->setObject(nullptr);
    onUpdateTemplate(false);
}

void ParticleEdit::onNodeSelected(void *node) {
    if(node) {
        ui->treeView->setObject(static_cast<QObject *>(node));
    }
}

void ParticleEdit::onNodeDeleted() {
    ui->treeView->setObject(nullptr);
}

void ParticleEdit::onEmitterSelected(QString emitter) {
    EffectEmitter *obj = m_pBuilder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        onNodeSelected(obj);
    }
}

void ParticleEdit::onEmitterCreated() {
    onNodeSelected(m_pBuilder->createEmitter());
}

void ParticleEdit::onEmitterDeleted(QString name) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Do you want to delete emitter?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int result = msgBox.exec();
    if(result == QMessageBox::Yes) {
        m_pBuilder->deleteEmitter(name);
        ui->treeView->setObject(nullptr);
    }
}


void ParticleEdit::onFunctionSelected(QString emitter, QString function) {
    EffectEmitter *obj = m_pBuilder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        EffectFunction *func = obj->findChild<EffectFunction *>(function);
        if(func) {
            onNodeSelected(func);
        }
    }
}

void ParticleEdit::onFunctionCreated(QString emitter, QString function) {
    EffectEmitter *obj = m_pBuilder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        EffectFunction *func = obj->findChild<EffectFunction *>(function);
        if(func == nullptr) {
            onNodeSelected(m_pBuilder->createFunction(emitter, function));
        } else {
            QMessageBox msgBox(this);
            msgBox.setText("This type of modifier already assigned.");
            msgBox.exec();
        }
    }
}

void ParticleEdit::onFunctionDeleted(QString emitter, QString function) {
    m_pBuilder->deleteFunction(emitter, function);
}

void ParticleEdit::onUpdateTemplate(bool update) {
    ParticleRender *render = static_cast<ParticleRender *>(m_pEffect->component("ParticleRender"));
    if(render) {
        render->effect()->loadUserData(m_pBuilder->data().toMap());
        render->setEffect(render->effect());
    }
    QObjectList list = m_pBuilder->children();
    ui->quickWidget->rootContext()->setContextProperty("effectModel", QVariant::fromValue(list));

    m_Modified = update;
}

void ParticleEdit::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void ParticleEdit::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}

void ParticleEdit::on_actionSave_triggered() {
    if(!m_Path.isEmpty()) {
        m_pBuilder->save(m_Path);
        m_Modified = false;
    }
}
