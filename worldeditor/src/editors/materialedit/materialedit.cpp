#include "materialedit.h"
#include "ui_materialedit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include <QQmlContext>
#include <QQuickItem>

#include <QWidgetAction>

#include <engine.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>
#include <components/camera.h>

#include <resources/mesh.h>

#include <global.h>
#include <json.h>

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "shaderbuilder.h"

#include "functionmodel.h"

#include "projectmanager.h"

#include "editors/componentbrowser/componentbrowser.h"

MaterialEdit::MaterialEdit(DocumentModel *document) :
        QMainWindow(nullptr),
        m_Modified(false),
        ui(new Ui::MaterialEdit),
        m_pMesh(nullptr),
        m_pLight(nullptr),
        m_pMaterial(nullptr),
        m_pBuilder(new ShaderBuilder()),
        m_pEditor(nullptr),
        m_pDocument(document) {

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

    Scene *scene = glWidget->scene();
    m_pLight = Engine::composeActor("DirectLight", "LightSource", scene);
    Matrix3 rot;
    rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
    m_pLight->transform()->setQuaternion(rot);

    Camera *camera = glWidget->controller()->camera();
    if(camera) {
        camera->setColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f));
    }

    m_pMesh = Engine::composeActor("MeshRender", "MeshRender", scene);

    on_actionSphere_triggered();

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
    ui->components->setModel(static_cast<AbstractSchemeModel *>(m_pBuilder)->components());

    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));
    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateTemplate()));

    ui->schemeWidget->rootContext()->setContextProperty("schemeModel", m_pBuilder);
    ui->schemeWidget->rootContext()->setContextProperty("stateMachine", QVariant(false));
    ui->schemeWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    //ui->schemeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QQuickItem *item = ui->schemeWidget->rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));

    m_pBrowser  = new ComponentBrowser(this);
    m_pBrowser->setModel(static_cast<AbstractSchemeModel *>(m_pBuilder)->components());
    connect(m_pBrowser, SIGNAL(componentSelected(QString)), this, SLOT(onComponentSelected(QString)));

    m_pCreateMenu = new QMenu(this);
    m_pAction = new QWidgetAction(m_pCreateMenu);
    m_pAction->setDefaultWidget(m_pBrowser);
    m_pCreateMenu->addAction(m_pAction);

    m_pUndo = UndoManager::instance()->createUndoAction(ui->menuEdit);
    m_pUndo->setShortcut(QKeySequence("Ctrl+Z"));
    ui->menuEdit->insertAction(ui->actionPlane, m_pUndo);

    m_pRedo = UndoManager::instance()->createRedoAction(ui->menuEdit);
    m_pRedo->setShortcut(QKeySequence("Ctrl+Y"));
    ui->menuEdit->insertAction(ui->actionPlane, m_pRedo);

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

bool MaterialEdit::isModified() const {
    return m_Modified;
}

QStringList MaterialEdit::assetTypes() const {
    return {"Material"};
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
    QDir dir(ProjectManager::instance()->contentPath());
    m_pDocument->closeFile(dir.relativeFilePath(m_Path));
}

void MaterialEdit::loadAsset(IConverterSettings *settings) {
    show();

    m_Path = settings->source();
    m_pMaterial = Engine::objectCreate<Material>();
    MeshRender *mesh = static_cast<MeshRender *>(m_pMesh->component("MeshRender"));
    if(mesh) {
        mesh->setMaterial(m_pMaterial);
    }
    static_cast<AbstractSchemeModel *>(m_pBuilder)->load(m_Path);

    m_Modified = false;
    onNodesSelected(QVariantList({0}));
}

void MaterialEdit::onUpdateTemplate(bool update) {
    if(m_pBuilder && m_pBuilder->build()) {
        ui->textEdit->setText(m_pBuilder->shader());
        MeshRender *mesh = static_cast<MeshRender *>(m_pMesh->component("MeshRender"));
        if(mesh) {
            VariantMap map = m_pBuilder->data(true).toMap();
            m_pMaterial->loadUserData(map);
        }
        m_Modified = update;
    }
}

void MaterialEdit::changeMesh(const string &path) {
    MeshRender *mesh = static_cast<MeshRender *>(m_pMesh->component("MeshRender"));
    if(mesh) {
        mesh->setMesh(Engine::loadResource<Mesh>(path));
        if(m_pMaterial) {
            mesh->setMaterial(m_pMaterial);
        }
        float bottom;
        static_cast<CameraCtrl *>(glWidget->controller())->setFocusOn(m_pMesh, bottom);
    }
}

void MaterialEdit::onComponentSelected(const QString &path) {
    m_pCreateMenu->hide();

    QQuickItem *ptr = ui->schemeWidget->rootObject()->findChild<QQuickItem *>("Canvas");
    if(ptr) {
        m_pBuilder->createNode(path, ptr->property("mouseX").toInt(), ptr->property("mouseY").toInt());
    }
}

void MaterialEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_pBuilder->node(list.front().toInt());
        if(node) {
            ui->treeView->setObject(static_cast<QObject *>(node->ptr));
        }
    }
}

void MaterialEdit::on_actionPlane_triggered() {
    changeMesh(".embedded/plane.fbx/Plane001");
}

void MaterialEdit::on_actionCube_triggered() {
    changeMesh(".embedded/cube.fbx/Box001");
}

void MaterialEdit::on_actionSphere_triggered() {
    changeMesh(".embedded/sphere.fbx/Sphere001");
}

void MaterialEdit::on_actionSave_triggered() {
    if(!m_Path.isEmpty()) {
        static_cast<AbstractSchemeModel *>(m_pBuilder)->save(m_Path);
        m_Modified = false;
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
