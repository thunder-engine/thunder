#include "meshedit.h"
#include "ui_meshedit.h"

#include <QSettings>
#include <QKeyEvent>

#include <engine.h>

#include <components/actor.h>
#include <components/staticmesh.h>
#include <components/directlight.h>
#include <components/camera.h>

#include "common.h"

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "fbxconverter.h"

MeshEdit::MeshEdit(Engine *engine) :
        QMainWindow(nullptr),
        IAssetEditor(engine),
        ui(new Ui::MeshEdit),
        m_pMesh(nullptr),
        m_pGround(nullptr),
        m_pLight(nullptr),
        m_pDome(nullptr),
        m_pEditor(nullptr) {

    ui->setupUi(this);

    glWidget    = new SceneView(engine, this);
    glWidget->setController(new CameraCtrl(m_pEngine));
    glWidget->setObjectName("Preview");
    glWidget->setWindowTitle("Preview");

    ui->treeView->setWindowTitle("Properties");

    connect(glWidget, SIGNAL(inited()), this, SLOT(onGLInit()));
    startTimer(16);

    ui->centralwidget->addToolWindow(glWidget, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(ui->treeView, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(glWidget));

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = ui->menuWindow->addAction(it->windowTitle());
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }

    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));

    readSettings();

    m_Yaw   = 0.0f;
}

MeshEdit::~MeshEdit() {
    writeSettings();

    delete ui;
    delete m_pEditor;

    delete glWidget;
    delete m_pMesh;
    delete m_pGround;
    delete m_pLight;
}

void MeshEdit::timerEvent(QTimerEvent *event) {
    glWidget->update();
}

void MeshEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value("mesh.mesh.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("mesh.mesh.windows"));

}

void MeshEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("mesh.mesh.geometry", saveGeometry());
    settings.setValue("mesh.mesh.windows", ui->centralwidget->saveState());
}

void MeshEdit::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);

    writeSettings();
}

void MeshEdit::loadAsset(IConverterSettings *settings) {
    show();
    prepareScene(settings->destination());

    delete m_pEditor;
    m_pEditor   = new NextObject(m_pMesh, nullptr, this);
    ui->treeView->setObject(dynamic_cast<QObject *>(settings));
}

void MeshEdit::prepareScene(const QString &resource) {
    StaticMesh *staticMesh  = m_pMesh->component<StaticMesh>();
    if(staticMesh) {
        Mesh *m = Engine::loadResource<Mesh>(qPrintable(resource));
        staticMesh->setMesh(m);
    }
    float bottom;
    glWidget->controller()->setFocusOn(m_pMesh, bottom);
    m_pGround->setPosition(Vector3(0.0f, bottom - m_pGround->scale().y * 2.0, 0.0f));
}

void MeshEdit::onGLInit() {
    Scene *scene    = glWidget->scene();
    scene->setAmbient(0.5f);

    m_pMesh     = Engine::objectCreate<Actor>("Model", scene);
    m_pMesh->addComponent<StaticMesh>();

    m_pLight    = Engine::objectCreate<Actor>("LightSource", scene);
    Matrix3 mat;
    mat.rotate(Vector3(-45.0f, 45.0f, 0.0f));//162.0
    m_pLight->setRotation(mat);
    DirectLight *light  = m_pLight->addComponent<DirectLight>();
    light->setCastShadows(false);
    light->setColor(Vector4(0.99f, 0.83985f, 0.7326f, 1.0f));

    m_pGround   = Engine::objectCreate<Actor>("Ground", scene);
    m_pGround->setScale(Vector3(100.0f, 1.0f, 100.0f));
    m_pGround->addComponent<StaticMesh>()->setMesh(Engine::loadResource<Mesh>(".embedded/cube.fbx"));

    m_pDome   = Engine::objectCreate<Actor>("Dome", scene);
    m_pDome->setScale(Vector3(250.0f, 250.0f, 250.0f));
    StaticMesh *mesh    = m_pDome->addComponent<StaticMesh>();
    if(mesh) {
        //mesh->setMesh(Cache::load<Mesh>(".embedded/sphere.fbx"));
    }
    m_pDome->setEnable(false);
}

void MeshEdit::onKeyPress(QKeyEvent *pe) {
    switch (pe->key()) {
        case Qt::Key_L: {
            /// \todo: Light control
        } break;
        default: break;
    }
}

void MeshEdit::onKeyRelease(QKeyEvent *pe) {
    switch (pe->key()) {
        case Qt::Key_L: {
            /// \todo: Light control
        } break;
        default: break;
    }
}

void MeshEdit::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void MeshEdit::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}
