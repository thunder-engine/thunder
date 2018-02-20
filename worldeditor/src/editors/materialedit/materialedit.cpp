#include "materialedit.h"
#include "ui_materialedit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include <engine.h>
#include <components/actor.h>
#include <components/staticmesh.h>
#include <components/directlight.h>
#include <components/camera.h>

#include "common.h"
#include "ajson.h"

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "materialconverter.h"

#include "shaderbuilder.h"

#include "functionmodel.h"

MaterialEdit::MaterialEdit(Engine *engine) :
        QMainWindow(nullptr),
        IAssetEditor(engine),
        ui(new Ui::MaterialEdit),
        m_pMaterial(nullptr),
        m_pMesh(nullptr),
        m_pLight(nullptr),
        m_pEditor(nullptr),
        m_pBuilder(nullptr) {

    ui->setupUi(this);

    glWidget    = new SceneView(engine, this);
    CameraCtrl *ctrl    = new CameraCtrl(m_pEngine);
    ctrl->blockFree(true);
    ctrl->blockMovement(true);
    glWidget->setController(ctrl);
    glWidget->setObjectName("Preview");
    glWidget->setWindowTitle("Preview");

    ui->treeView->setWindowTitle("Properties");
    ui->textEdit->setWindowTitle("Source Code");
    ui->schemeWidget->setWindowTitle("Scheme");

    connect(glWidget, SIGNAL(inited()), this, SLOT(onGLInit()));
    startTimer(16);

    ui->centralwidget->addToolWindow(ui->schemeWidget, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(glWidget, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(ui->schemeWidget));
    ui->centralwidget->addToolWindow(ui->treeView, QToolWindowManager::ReferenceBottomOf, ui->centralwidget->areaFor(glWidget));
    ui->centralwidget->addToolWindow(ui->components, QToolWindowManager::ReferenceRightOf, ui->centralwidget->areaFor(ui->schemeWidget));
    ui->centralwidget->addToolWindow(ui->textEdit, QToolWindowManager::ReferenceBottomOf, ui->centralwidget->areaFor(ui->schemeWidget));

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = ui->menuWindow->addAction(it->windowTitle());
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }

    m_pBuilder  = new ShaderBuilder();
    ui->components->setModel(m_pBuilder->components());

    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));
    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateTemplate()));
    connect(ui->schemeWidget, SIGNAL(nodeSelected(void*)), this, SLOT(onNodeSelected(void*)));

    readSettings();
}

MaterialEdit::~MaterialEdit() {
    writeSettings();

    delete ui;
    delete m_pEditor;
    delete m_pBuilder;
    delete glWidget;

    delete m_pMesh;
    delete m_pLight;
}

void MaterialEdit::timerEvent(QTimerEvent *) {
    glWidget->update();
}

void MaterialEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value("material.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("material.windows"));
}

void MaterialEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("material.geometry", saveGeometry());
    settings.setValue("material.windows", ui->centralwidget->saveState());
}

void MaterialEdit::closeEvent(QCloseEvent *event) {
    writeSettings();

    if(isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The material has been modified.");
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

void MaterialEdit::loadAsset(IConverterSettings *settings) {
    show();
    if(m_Path != settings->source()) {
        m_Path      = settings->source();
        m_pMaterial = Engine::loadResource<Material>(settings->destination());
        StaticMesh *mesh    = m_pMesh->component<StaticMesh>();
        if(mesh) {
            mesh->setMaterial(0, m_pMaterial);
        }
        m_pBuilder->load(m_Path);

        onUpdateTemplate(false);

        ui->schemeWidget->setModel(m_pBuilder);
        onNodeSelected(m_pBuilder);
    }
}

void MaterialEdit::onUpdateTemplate(bool update) {
    if(m_pBuilder && m_pBuilder->build()) {
        ui->textEdit->setText(m_pBuilder->shader());
        glWidget->makeCurrent();
        StaticMesh *mesh    = m_pMesh->component<StaticMesh>();
        if(mesh) {
            AVariantMap map   = m_pBuilder->data().toMap();
            for(auto it : mesh->materials()) {
                it->material()->loadUserData(map);
            }
        }
        setModified(update);
    }
}

void MaterialEdit::changeMesh(const string &path) {
    StaticMesh *mesh    = m_pMesh->component<StaticMesh>();
    if(mesh) {
        mesh->setMesh(Engine::loadResource<Mesh>(path));
        if(m_pMaterial) {
            mesh->setMaterial(0, m_pMaterial);
        }
        float bottom;
        glWidget->controller()->setFocusOn(m_pMesh, bottom);
    }
}

void MaterialEdit::onGLInit() {
    Scene *scene    = glWidget->scene();
    if(scene) {
        scene->setAmbient(0.25f);
    }

    m_pLight    = Engine::objectCreate<Actor>("LightSource", scene);
    Matrix3 rot;
    rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
    m_pLight->setRotation(rot);
    m_pLight->addComponent<DirectLight>();

    CameraCtrl *controller  = glWidget->controller();
    Camera *camera  = controller->activeCamera();
    if(camera) {
        camera->setColor(Vector4(0.3, 0.3, 0.3, 1.0));
    }

    m_pMesh     = Engine::objectCreate<Actor>("StaticMesh", scene);
    m_pMesh->addComponent<StaticMesh>();

    on_actionSphere_triggered();
}

void MaterialEdit::onNodeSelected(void *node) {
    QObject *o  = static_cast<QObject *>(node);
    ui->treeView->setObject(o);
}

void MaterialEdit::on_actionPlane_triggered() {
    changeMesh(".embedded/plane.fbx");
}

void MaterialEdit::on_actionCube_triggered() {
    changeMesh(".embedded/cube.fbx");
}

void MaterialEdit::on_actionSphere_triggered() {
    changeMesh(".embedded/sphere.fbx");
}

void MaterialEdit::on_actionSave_triggered() {
    if(!m_Path.isEmpty()) {
        m_pBuilder->save(m_Path);
        setModified(false);
    }
}

void MaterialEdit::onKeyPress(QKeyEvent *pe) {
    switch (pe->key()) {
        case Qt::Key_L: {
            /// \todo: Light control
        } break;
        default: break;
    }
}

void MaterialEdit::onKeyRelease(QKeyEvent *pe) {
    switch (pe->key()) {
        case Qt::Key_L: {
            /// \todo: Light control
        } break;
        default: break;
    }
}

void MaterialEdit::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void MaterialEdit::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}
