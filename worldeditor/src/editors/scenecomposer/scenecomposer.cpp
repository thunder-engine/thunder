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
        m_properties(new NextObject(this)) {

    ui->setupUi(this);

    ui->renderMode->setMenu(new QMenu);

    ObjectCtrl *ctl = new ObjectCtrl(ui->viewport);
    ctl->resetModified();
    ctl->createMenu(ui->renderMode->menu());

    ui->viewport->setController(ctl);

    int index = 0;
    for(auto &it : ctl->tools()) {
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

        connect(tool, SIGNAL(clicked()), ctl, SLOT(onChangeTool()));
        if(index == 0) {
            tool->click();
        }
        index++;
    }

    connect(ctl, SIGNAL(mapUpdated()), this, SIGNAL(hierarchyUpdated()));
    connect(ctl, SIGNAL(dropMap(QString)), this, SIGNAL(dropAsset(QString)));
    connect(ctl, SIGNAL(objectsSelected(Object::ObjectList)), this, SLOT(onItemsSelected(Object::ObjectList)));

    connect(this, SIGNAL(createComponent(QString)), ctl, SLOT(onCreateComponent(QString)));

    connect(ui->orthoButton, &QPushButton::toggled, ctl, &ObjectCtrl::onOrthographic);
    connect(ui->localButton, &QPushButton::toggled, ctl, &ObjectCtrl::onLocal);
    connect(ui->localButton, &QPushButton::toggled, this, &SceneComposer::onLocal);

    connect(PluginManager::instance(), SIGNAL(pluginReloaded()), ctl, SLOT(onUpdateSelected()));

    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &SceneComposer::onRepickSelected);

    ui->orthoButton->setProperty("checkgreen", true);
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

void SceneComposer::onItemsSelected(const Object::ObjectList &objects) {
    if(!objects.empty()) {
        ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());

        m_properties->setObject(*objects.begin());

        connect(ctl, SIGNAL(objectsUpdated()), m_properties, SLOT(onUpdated()));

        connect(m_properties, SIGNAL(deleteComponent(QString)), ctl, SLOT(onDeleteComponent(QString)));
        connect(m_properties, SIGNAL(changed(Object*,QString)), ctl, SLOT(onUpdated()));
        connect(m_properties, SIGNAL(aboutToBeChanged(Object*,QString,Variant)), ctl, SLOT(onPropertyChanged(Object*,QString,Variant)), Qt::DirectConnection);
        connect(m_properties, SIGNAL(updated()), this, SIGNAL(itemUpdated()));

        //connect(ctl, SIGNAL(objectsChanged(Object::ObjectList,QString)), ui->timeline, SLOT(onChanged(Object::ObjectList,QString)));
        //connect(m_properties, SIGNAL(changed(Object*,QString)), ui->timeline, SLOT(onUpdated(Object*,QString)));
        //connect(ui->timeline, SIGNAL(moved()), m_properties, SLOT(onUpdated()));
    }

    emit itemSelected(!objects.empty() ? m_properties : nullptr);

    emit itemsSelected(objects);
}

void SceneComposer::onSelectActors(Object::ObjectList objects) {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctl->onSelectActor(objects);
}

void SceneComposer::onRemoveActors(Object::ObjectList objects) {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctl->onRemoveActor(objects);
}

void SceneComposer::onUpdated() {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctl->onUpdated();
    emit itemUpdated();
}

void SceneComposer::onParentActors(Object::ObjectList objects, Object *parent) {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctl->onParentActor(objects, parent);
}

void SceneComposer::onFocusActor(Object *actor) {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctl->onFocusActor(actor);
}

void SceneComposer::onRepickSelected() {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    onItemsSelected(ctl->selected());
}

void SceneComposer::backupScene() {
    ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    m_backupScene = Bson::save(Engine::toVariant(ctrl->map()));
}

void SceneComposer::restoreBackupScene() {
    Object *map = Engine::toObject(Bson::load(m_backupScene), nullptr);
    if(map) {
        ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
        ctrl->clear();
        ctrl->setMap(map);

        map->setParent(ui->viewport->scene()); // Set parent after detach previous one

        emit hierarchyCreated(ctrl->map());
    }
}

bool SceneComposer::isModified() const {
    return static_cast<ObjectCtrl *>(ui->viewport->controller())->isModified();
}

void SceneComposer::setModified(bool flag) {
    if(!flag) {
        static_cast<ObjectCtrl *>(ui->viewport->controller())->resetModified();
    }
}

QStringList SceneComposer::suffixes() const {
    return { "map" };
}

void SceneComposer::newAsset() {
    AssetEditor::newAsset();
    ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctrl->clear();
    ctrl->setMap(Engine::objectCreate<Actor>("Chunk", ui->viewport->scene()));

    emit hierarchyCreated(ctrl->map());
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
        ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
        ctrl->clear();
        ctrl->setMap(map);

        emit hierarchyCreated(ctrl->map());

        UndoManager::instance()->clear();
    }
}

void SceneComposer::saveAsset(const QString &path) {
    ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    Object *map = ctrl->map();
    if(map) {
        string data = Json::save(Engine::toVariant(map), 0);
        if(!data.empty()) {
            QFile file(path);
            if(file.open(QIODevice::WriteOnly)) {
                file.write(static_cast<const char *>(&data[0]), data.size());
                file.close();
            }
        }
        static_cast<ObjectCtrl *>(ui->viewport->controller())->resetModified();

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
