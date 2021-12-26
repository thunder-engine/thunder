#include "materialedit.h"
#include "ui_materialedit.h"

#include <QQmlContext>
#include <QQuickItem>

#include <QMenu>

#include <engine.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/directlight.h>

#include <resources/mesh.h>

#include <editor/viewport/cameractrl.h>

#include <global.h>
#include <json.h>

#include "shaderbuilder.h"

namespace {
    const char *gMeshRender("MeshRender");
};

MaterialEdit::MaterialEdit() :
        ui(new Ui::MaterialEdit),
        m_mesh(nullptr),
        m_light(nullptr),
        m_material(nullptr),
        m_builder(new ShaderBuilder()),
        m_createMenu(new QMenu(this)),
        m_selectedItem(nullptr),
        m_lastCommand(nullptr),
        m_node(-1),
        m_port(-1),
        m_out(false) {

    ui->setupUi(this);
    CameraCtrl *ctrl = new CameraCtrl();
    ctrl->blockMovement(true);
    ctrl->setFree(false);

    Scene *scene = Engine::objectCreate<Scene>("Scene");

    ui->preview->setController(ctrl);
    ui->preview->setScene(scene);

    m_light = Engine::composeActor("DirectLight", "LightSource", scene);
    m_light->transform()->setRotation(Vector3(-45.0f, 45.0f, 0.0f));

    m_mesh = Engine::composeActor("MeshRender", "MeshRender", scene);

    on_actionSphere_triggered();

    connect(m_builder, &ShaderBuilder::schemeUpdated, this, &MaterialEdit::onSchemeUpdated);

    ui->schemeWidget->rootContext()->setContextProperty("schemeModel", m_builder);
    ui->schemeWidget->rootContext()->setContextProperty("stateMachine", QVariant(false));
    ui->schemeWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    ui->schemeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QQuickItem *item = ui->schemeWidget->rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));
    connect(item, SIGNAL(showContextMenu(int,int,bool)), this, SLOT(onShowContextMenu(int,int,bool)));

    for(auto &it : m_builder->nodes()) {
        QMenu *menu = m_createMenu;
        QStringList list = it.split("/", QString::SkipEmptyParts);

        for(int i = 0; i < list.size(); i++) {
            QString part = list.at(i);
            QAction *action = nullptr;
            for(QAction *act : menu->actions()) {
                if(part == act->objectName()) {
                    action = act;
                    menu = act->menu();
                    break;
                }
            }
            if(action == nullptr) {
                action = menu->addAction(part);
                action->setObjectName(qPrintable(part));
                if(i < (list.size() - 1)) {
                    menu = new QMenu;
                    action->setMenu(menu);
                } else {
                    connect(action, &QAction::triggered, this, &MaterialEdit::onComponentSelected);
                }
            }
        }
    }

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);
}

MaterialEdit::~MaterialEdit() {
    delete ui;

    delete m_mesh;
    delete m_light;
}

bool MaterialEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(m_builder) != m_lastCommand);
}

QStringList MaterialEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_builder)->suffixes();
}

void MaterialEdit::onActivated() {
    emit itemSelected(m_selectedItem);
}

void MaterialEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings != settings) {
        m_pSettings = settings;

        m_material = Engine::objectCreate<Material>();
        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            mesh->setMaterial(m_material);
        }
        static_cast<AbstractSchemeModel *>(m_builder)->load(m_pSettings->source());

        m_lastCommand = UndoManager::instance()->lastCommand(m_builder);

        onNodesSelected(QVariantList({0}));
    }
}

void MaterialEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_pSettings->source().isEmpty()) {
        static_cast<AbstractSchemeModel *>(m_builder)->save(path.isEmpty() ? m_pSettings->source() : path);

        m_lastCommand = UndoManager::instance()->lastCommand(m_builder);
    }
}

void MaterialEdit::onSchemeUpdated() {
    if(m_builder && m_builder->build()) {
        MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
        if(mesh) {
            VariantMap map = m_builder->data(true).toMap();
            m_material->loadUserData(map);
        }
    }
}

void MaterialEdit::changeMesh(const string &path) {
    MeshRender *mesh = static_cast<MeshRender *>(m_mesh->component(gMeshRender));
    if(mesh) {
        mesh->setMesh(Engine::loadResource<Mesh>(path));
        if(m_material) {
            mesh->setMaterial(m_material);
        }
        float bottom;
        static_cast<CameraCtrl *>(ui->preview->controller())->setFocusOn(m_mesh, bottom);
    }
}

void MaterialEdit::onComponentSelected() {
    m_createMenu->hide();

    QAction *action = static_cast<QAction *>(sender());

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
                m_builder->createAndLink(action->objectName(), x, y, m_node, m_port, m_out);
            } else {
                m_builder->createNode(action->objectName(), x, y);
            }
        }
    }
}

void MaterialEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_builder->node(list.front().toInt());
        if(node) {
            m_selectedItem = static_cast<QObject *>(node->ptr);
            emit itemSelected(m_selectedItem);
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

void MaterialEdit::onShowContextMenu(int node, int port, bool out) {
    m_node = node;
    m_port = port;
    m_out = out;
    m_createMenu->exec(QCursor::pos());
}

void MaterialEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
