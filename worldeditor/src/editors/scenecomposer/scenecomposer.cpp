#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QFile>
#include <QMenu>
#include <QDateTime>

#include <json.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/scene.h>

#include <editor/assetconverter.h>

#include "controllers/objectctrl.h"

#include "undomanager.h"
#include "projectmanager.h"
#include "pluginmanager.h"
#include "assetmanager.h"

#include "editors/propertyedit/nextobject.h"

SceneComposer::SceneComposer(QWidget *parent) :
        ui(new Ui::SceneComposer),
        m_properties(new NextObject(this)),
        m_controller(nullptr) {

    ui->setupUi(this);

    ui->renderMode->setMenu(new QMenu);

    m_controller = new ObjectCtrl(ui->viewport);
    m_controller->resetModified();
    m_controller->createMenu(ui->renderMode->menu());

    ui->viewport->setController(m_controller);

    int index = 0;
    for(auto &it : m_controller->tools()) {
        QPushButton *tool = new QPushButton();
        tool->setProperty("blue", true);
        tool->setProperty("checkred", true);
        tool->setCheckable(true);
        tool->setAutoExclusive(true);
        tool->setIcon(QIcon(it->icon()));
        tool->setObjectName(it->name());
        QString shortcut = it->shortcut();
        tool->setShortcut(QKeySequence(shortcut));
        tool->setToolTip(it->toolTip() + (!shortcut.isEmpty() ? (" (" + shortcut + ")") : ""));

        ui->viewportLayout->insertWidget(index, tool);

        connect(tool, SIGNAL(clicked()), m_controller, SLOT(onChangeTool()));
        if(index == 0) {
            tool->click();
        }
        index++;
    }

    connect(m_controller, SIGNAL(mapUpdated()), this, SIGNAL(hierarchyUpdated()));
    connect(m_controller, SIGNAL(dropMap(QString)), this, SIGNAL(dropAsset(QString)));
    connect(m_controller, SIGNAL(objectsSelected(Object::ObjectList)), this, SLOT(onItemsSelected(Object::ObjectList)));

    connect(this, SIGNAL(createComponent(QString)), m_controller, SLOT(onCreateComponent(QString)));

    connect(ui->orthoButton, &QPushButton::toggled, m_controller, &ObjectCtrl::onOrthographic);
    connect(ui->localButton, &QPushButton::toggled, m_controller, &ObjectCtrl::onLocal);
    connect(ui->localButton, &QPushButton::toggled, this, &SceneComposer::onLocal);

    connect(PluginManager::instance(), SIGNAL(pluginReloaded()), m_controller, SLOT(onUpdateSelected()));

    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &SceneComposer::onRepickSelected);

    connect(m_controller, SIGNAL(objectsUpdated()), m_properties, SLOT(onUpdated()));

    connect(m_properties, SIGNAL(deleteComponent(QString)), m_controller, SLOT(onDeleteComponent(QString)));
    connect(m_properties, SIGNAL(changed(Object*,QString)), m_controller, SLOT(onUpdated()));
    connect(m_properties, SIGNAL(aboutToBeChanged(Object*,QString,Variant)), m_controller, SLOT(onPropertyChanged(Object*,QString,Variant)), Qt::DirectConnection);
    connect(m_properties, SIGNAL(updated()), this, SIGNAL(itemUpdated()));

    //connect(ctl, SIGNAL(objectsChanged(Object::ObjectList,QString)), ui->timeline, SLOT(onChanged(Object::ObjectList,QString)));
    //connect(m_properties, SIGNAL(changed(Object*,QString)), ui->timeline, SLOT(onUpdated(Object*,QString)));
    //connect(ui->timeline, SIGNAL(moved()), m_properties, SLOT(onUpdated()));

    ui->orthoButton->setProperty("checkgreen", true);

    m_contentMenu.addAction(createAction(tr("Rename"), SLOT(onItemRename()), QKeySequence(Qt::Key_F2)));
    m_contentMenu.addAction(createAction(tr("Duplicate"), SLOT(onItemDuplicate())));
    m_contentMenu.addAction(createAction(tr("Delete"), SLOT(onItemDelete()), QKeySequence(Qt::Key_Delete)));
    m_contentMenu.addSeparator();

    m_prefab.push_back((createAction(tr("Unpack Prefab"), SLOT(onItemUnpack()))));
    m_prefab.push_back(createAction(tr("Unpack Prefab Completely"), SLOT(onItemUnpackAll())));
    for(auto &it : m_prefab) {
        m_contentMenu.addAction(it);
    }
    m_contentMenu.addSeparator();
    m_contentMenu.addAction(createAction(tr("Create Actor"), SLOT(onCreateActor())));

    connect(&m_contentMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShow()));
}

SceneComposer::~SceneComposer() {
    delete ui;
}

void SceneComposer::setScene(Scene *scene) {
    ui->viewport->setScene(scene);
    ui->viewport->controller()->init(scene);
}

VariantList SceneComposer::saveState() const {
    return ui->viewport->controller()->saveState();
}

void SceneComposer::restoreState(const VariantList &state) {
    ui->orthoButton->setChecked(ui->viewport->controller()->restoreState(state));
}

void SceneComposer::takeScreenshot() {
    QImage result = ui->viewport->grabFramebuffer();
    result.save("MainWindow-" + QDateTime::currentDateTime().toString("ddMMyy-HHmmss") + ".png");
}

QString SceneComposer::path() const {
    if(m_pSettings) {
        return m_pSettings->source();
    }
    return QString();
}

QMenu *SceneComposer::contextMenu() {
    return &m_contentMenu;
}

void SceneComposer::onItemsSelected(const Object::ObjectList &objects) {
    if(!objects.empty()) {
        m_properties->setObject(*objects.begin());
    }

    emit itemSelected(!objects.empty() ? m_properties : nullptr);

    emit itemsSelected(objects);
}

void SceneComposer::onSelectActors(Object::ObjectList objects) {
    m_controller->onSelectActor(objects);
}

void SceneComposer::onRemoveActors(Object::ObjectList objects) {
    m_controller->onRemoveActor(objects);
}

void SceneComposer::onUpdated() {
    m_controller->onUpdated();
    emit itemUpdated();
}

void SceneComposer::onParentActors(Object::ObjectList objects, Object *parent) {
    m_controller->onParentActor(objects, parent);
}

void SceneComposer::onFocusActor(Object *actor) {
    m_controller->onFocusActor(actor);
}

void SceneComposer::onRepickSelected() {
    onItemsSelected(m_controller->selected());
}

void SceneComposer::backupScene() {
    m_backupScene = Bson::save(Engine::toVariant(m_controller->map()));
}

void SceneComposer::restoreBackupScene() {
    Object *map = Engine::toObject(Bson::load(m_backupScene), nullptr);
    if(map) {
        m_controller->clear();
        m_controller->setMap(map);

        map->setParent(ui->viewport->scene()); // Set parent after detach previous one

        emit hierarchyCreated(m_controller->map());
    }
}

bool SceneComposer::isModified() const {
    return m_controller->isModified();
}

void SceneComposer::setModified(bool flag) {
    if(!flag) {
        m_controller->resetModified();
    }
}

QStringList SceneComposer::suffixes() const {
    return { "map" };
}

void SceneComposer::newAsset() {
    AssetEditor::newAsset();
    m_controller->clear();
    m_controller->setMap(Engine::objectCreate<Actor>("Chunk", ui->viewport->scene()));

    emit hierarchyCreated(m_controller->map());
}

void SceneComposer::loadAsset(AssetConverterSettings *settings) {
    m_pSettings = settings;
    QFile loadFile(m_pSettings->source());
    if(!loadFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray array = loadFile.readAll();
    Variant var = Json::load(array.constData());
    Object *map = Engine::toObject(var, ui->viewport->scene());
    if(map) {
        m_controller->clear();
        m_controller->setMap(map);

        emit hierarchyCreated(m_controller->map());

        UndoManager::instance()->clear();
    }
}

void SceneComposer::saveAsset(const QString &path) {
    Object *map = m_controller->map();
    if(map) {
        string data = Json::save(Engine::toVariant(map), 0);
        if(!data.empty()) {
            QFile file(path);
            if(file.open(QIODevice::WriteOnly)) {
                file.write(static_cast<const char *>(&data[0]), data.size());
                file.close();
            }
        }
        m_controller->resetModified();

        QString path = ProjectManager::instance()->iconPath() + "/auto.png";
        QImage result = ui->viewport->grabFramebuffer();
        QRect rect((result.width() - result.height()) / 2, 0, result.height(), result.height());
        result.copy(rect).scaled(128, 128).save(path);
    }
}

void SceneComposer::onLocal(bool flag) {
    ui->localButton->setIcon(flag ? QIcon(":/Style/styles/dark/icons/local.png") :
                                    QIcon(":/Style/styles/dark/icons/global.png"));
}

void SceneComposer::onCreateActor() {
    UndoManager::instance()->push(new CreateObject("Actor", m_controller));
}

void SceneComposer::onItemRename() {
    emit renameItem();
}

void SceneComposer::onItemDuplicate() {
    UndoManager::instance()->push(new DuplicateObjects(m_controller));
}

void SceneComposer::onItemDelete() {
     UndoManager::instance()->push(new DeleteActors(m_controller->selected(), m_controller));
}

void SceneComposer::onItemUnpack() {
    for(auto it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            actor->setPrefab(nullptr);
            it->clearCloneRef();
        }
    }
}

void unpackHelper(Object *object) {
    if(object) {
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            actor->setPrefab(nullptr);
            object->clearCloneRef();
        }

        for(auto it : object->getChildren()) {
            unpackHelper(it);
        }
    }
}

void SceneComposer::onItemUnpackAll() {
    for(auto it : m_controller->selected()) {
        unpackHelper(it);
    }
}

void SceneComposer::onAboutToShow() {
    bool enabled = false;
    for(auto &it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor && actor->isInstance()) {
            enabled = true;
        }
    }

    for(auto &it : m_prefab) {
        it->setEnabled(enabled);
    }
}

QAction *SceneComposer::createAction(const QString &name, const char *member, const QKeySequence &shortcut) {
    QAction *a = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    connect(a, SIGNAL(triggered(bool)), this, member);
    return a;
}
