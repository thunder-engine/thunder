#include "materialedit.h"
#include "ui_materialedit.h"

#include <QQmlContext>
#include <QQuickItem>

#include <QWidgetAction>
#include <QMenu>

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

#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "shaderbuilder.h"

#include "functionmodel.h"

#include "projectmanager.h"

#include "editors/componentbrowser/componentbrowser.h"

namespace {
    const char *gMeshRender("MeshRender");
};

MaterialEdit::MaterialEdit() :
        m_Modified(false),
        ui(new Ui::MaterialEdit),
        m_pMesh(nullptr),
        m_pLight(nullptr),
        m_pMaterial(nullptr),
        m_pBuilder(new ShaderBuilder()),
        m_pBrowser(new ComponentBrowser(this)),
        m_node(-1),
        m_port(-1),
        m_out(false) {

    ui->setupUi(this);
    CameraCtrl *ctrl = new CameraCtrl(ui->preview);
    ctrl->blockMovement(true);
    ctrl->setFree(false);

    Scene *scene = Engine::objectCreate<Scene>("Scene");

    ui->preview->setController(ctrl);
    ui->preview->setScene(scene);

    m_pLight = Engine::composeActor("DirectLight", "LightSource", scene);
    Matrix3 rot;
    rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
    m_pLight->transform()->setQuaternion(rot);

    Camera *camera = ctrl->camera();
    if(camera) {
        camera->setColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f));
    }

    m_pMesh = Engine::composeActor("MeshRender", "MeshRender", scene);

    on_actionSphere_triggered();

    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateTemplate()));

    ui->schemeWidget->rootContext()->setContextProperty("schemeModel", m_pBuilder);
    ui->schemeWidget->rootContext()->setContextProperty("stateMachine", QVariant(false));
    ui->schemeWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    ui->schemeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QQuickItem *item = ui->schemeWidget->rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));
    connect(item, SIGNAL(showContextMenu(int,int,bool)), this, SLOT(onShowContextMenu(int,int,bool)));

    m_pBrowser->setModel(static_cast<AbstractSchemeModel *>(m_pBuilder)->components());
    connect(m_pBrowser, SIGNAL(componentSelected(QString)), this, SLOT(onComponentSelected(QString)));

    m_pCreateMenu = new QMenu(this);
    m_pAction = new QWidgetAction(m_pCreateMenu);
    m_pAction->setDefaultWidget(m_pBrowser);
    m_pCreateMenu->addAction(m_pAction);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);
}

MaterialEdit::~MaterialEdit() {
    delete ui;

    delete m_pMesh;
    delete m_pLight;
}

bool MaterialEdit::isModified() const {
    return m_Modified;
}

QStringList MaterialEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_pBuilder)->suffixes();
}

void MaterialEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings != settings) {
        m_pSettings = settings;

        m_pMaterial = Engine::objectCreate<Material>();
        MeshRender *mesh = static_cast<MeshRender *>(m_pMesh->component(gMeshRender));
        if(mesh) {
            mesh->setMaterial(m_pMaterial);
        }
        static_cast<AbstractSchemeModel *>(m_pBuilder)->load(m_pSettings->source());

        m_Modified = false;
        onNodesSelected(QVariantList({0}));
    }
}

void MaterialEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_pSettings->source().isEmpty()) {
        static_cast<AbstractSchemeModel *>(m_pBuilder)->save(path.isEmpty() ? m_pSettings->source() : path);
        m_Modified = false;
    }
}

void MaterialEdit::onUpdateTemplate(bool update) {
    if(m_pBuilder && m_pBuilder->build()) {
        MeshRender *mesh = static_cast<MeshRender *>(m_pMesh->component(gMeshRender));
        if(mesh) {
            VariantMap map = m_pBuilder->data(true).toMap();
            m_pMaterial->loadUserData(map);
        }
        m_Modified = update;
    }
}

void MaterialEdit::changeMesh(const string &path) {
    MeshRender *mesh = static_cast<MeshRender *>(m_pMesh->component(gMeshRender));
    if(mesh) {
        mesh->setMesh(Engine::loadResource<Mesh>(path));
        if(m_pMaterial) {
            mesh->setMaterial(m_pMaterial);
        }
        float bottom;
        static_cast<CameraCtrl *>(ui->preview->controller())->setFocusOn(m_pMesh, bottom);
    }
}

void MaterialEdit::onComponentSelected(const QString &path) {
    m_pCreateMenu->hide();

    QQuickItem *scheme = ui->schemeWidget->rootObject()->findChild<QQuickItem *>("Scheme");
    if(scheme) {
        int x = scheme->property("x").toInt();
        int y = scheme->property("y").toInt();
        float scale = scheme->property("scale").toFloat();

        QQuickItem *canvas = ui->schemeWidget->rootObject()->findChild<QQuickItem *>("Canvas");
        if(canvas) {
            int mouseX = canvas->property("mouseX").toInt();
            int mouseY = canvas->property("mouseY").toInt();
            x = (float)(mouseX - x) * scale;
            y = (float)(mouseY - y) * scale;

            if(m_node > -1 && m_port > -1) {
                m_pBuilder->createAndLink(path, x, y, m_node, m_port, m_out);
            } else {
                m_pBuilder->createNode(path, x, y);
            }
        }
    }
}

void MaterialEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_pBuilder->node(list.front().toInt());
        if(node) {
            emit itemSelected(static_cast<QObject *>(node->ptr));
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

void MaterialEdit::on_schemeWidget_customContextMenuRequested(const QPoint &) {
    //m_pCreateMenu->exec(QCursor::pos());
}

void MaterialEdit::onShowContextMenu(int node, int port, bool out) {
    m_node = node;
    m_port = port;
    m_out = out;
    m_pCreateMenu->exec(QCursor::pos());
}

void MaterialEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
