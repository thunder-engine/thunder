#include "materialedit.h"
#include "ui_materialedit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include <QQmlContext>
#include <QQuickItem>

#include <QWidgetAction>

#include <engine.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>
#include <components/camera.h>

#include <global.h>
#include "json.h"

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "shaderbuilder.h"

#include "functionmodel.h"

#include "editors/componentbrowser/componentbrowser.h"

MaterialEdit::MaterialEdit(Engine *engine) :
        QMainWindow(nullptr),
        IAssetEditor(engine),
        ui(new Ui::MaterialEdit),
        m_pMesh(nullptr),
        m_pLight(nullptr),
        m_pMaterial(nullptr),
        m_pBuilder(new ShaderBuilder()),
        m_pEditor(nullptr) {

    ui->setupUi(this);

    glWidget = new Viewport(this);
    CameraCtrl *ctrl = new CameraCtrl(glWidget);
    ctrl->blockMovement(true);
    ctrl->setFree(false);
    ctrl->init(nullptr);

    glWidget->setController(ctrl);
    glWidget->setScene(Engine::objectCreate<Scene>("Scene"));
    glWidget->setObjectName("Preview");
    glWidget->setWindowTitle("Preview");

    ui->treeView->setWindowTitle("Properties");
    ui->textEdit->setWindowTitle("Source Code");
    ui->schemeWidget->setWindowTitle("Scheme");

    connect(glWidget, SIGNAL(inited()), this, SLOT(onGLInit()), Qt::DirectConnection);
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
    ui->components->setModel(m_pBuilder->components());

    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));
    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateTemplate()));

    ui->schemeWidget->rootContext()->setContextProperty("schemeModel", m_pBuilder);
    ui->schemeWidget->rootContext()->setContextProperty("stateMachine", QVariant(false));
    ui->schemeWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    //ui->schemeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QQuickItem *item = ui->schemeWidget->rootObject();
    connect(item, SIGNAL(nodeSelected(int)), this, SLOT(onNodeSelected(int)));

    m_pBrowser  = new ComponentBrowser(this);
    m_pBrowser->setModel(m_pBuilder->components());
    connect(m_pBrowser, SIGNAL(componentSelected(QString)), this, SLOT(onComponentSelected(QString)));

    m_pCreateMenu = new QMenu(this);
    m_pAction = new QWidgetAction(m_pCreateMenu);
    m_pAction->setDefaultWidget(m_pBrowser);
    m_pCreateMenu->addAction(m_pAction);

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
    glWidget->repaint();
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
        MeshRender *mesh = m_pMesh->component<MeshRender>();
        if(mesh) {
            mesh->setMaterial(m_pMaterial);
        }
        m_pBuilder->load(m_Path);

        setModified(false);
        onNodeSelected(0);
    }
}

void MaterialEdit::onUpdateTemplate(bool update) {
    if(m_pBuilder && m_pBuilder->build()) {
        ui->textEdit->setText(m_pBuilder->shader());
        glWidget->makeCurrent();
        MeshRender *mesh    = m_pMesh->component<MeshRender>();
        if(mesh) {
            VariantMap map  = m_pBuilder->data().toMap();
            for(auto it : mesh->materials()) {
                it->material()->loadUserData(map);
            }
        }
        setModified(update);
    }
}

void MaterialEdit::changeMesh(const string &path) {
    MeshRender *mesh    = m_pMesh->component<MeshRender>();
    if(mesh) {
        mesh->setMesh(Engine::loadResource<Mesh>(path));
        if(m_pMaterial) {
            mesh->setMaterial(m_pMaterial);
        }
        float bottom;
        static_cast<CameraCtrl *>(glWidget->controller())->setFocusOn(m_pMesh, bottom);
    }
}

void MaterialEdit::onGLInit() {
    Scene *scene = glWidget->scene();
    m_pLight = Engine::objectCreate<Actor>("LightSource", scene);
    Matrix3 rot;
    rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
    m_pLight->transform()->setRotation(rot);
    m_pLight->addComponent<DirectLight>();

    Camera *camera = glWidget->controller()->camera();
    if(camera) {
        camera->setColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f));
    }

    m_pMesh = Engine::objectCreate<Actor>("MeshRender", scene);
    m_pMesh->addComponent<MeshRender>();

    on_actionSphere_triggered();
}


void MaterialEdit::onComponentSelected(const QString &path) {
    m_pCreateMenu->hide();

    QQuickItem *ptr = ui->schemeWidget->rootObject()->findChild<QQuickItem *>("Canvas");
    if(ptr) {
        AbstractSchemeModel::Node *node = m_pBuilder->createNode(path);
        if(node) {
            node->pos = QPoint(ptr->property("mouseX").toInt(), ptr->property("mouseY").toInt());
            m_pBuilder->schemeUpdated();
        }
    }
}

void MaterialEdit::onNodeSelected(int index) {
    const AbstractSchemeModel::Node *node = m_pBuilder->node(index);
    if(node) {
        ui->treeView->setObject(static_cast<QObject *>(node->ptr));
    }
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

void MaterialEdit::on_schemeWidget_customContextMenuRequested(const QPoint &) {
    m_pCreateMenu->exec(QCursor::pos());
}
