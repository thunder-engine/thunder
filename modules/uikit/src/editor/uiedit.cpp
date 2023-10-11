#include "uiedit.h"
#include "ui_uiedit.h"

#include <QSettings>
#include <QMenu>

#include <engine.h>

#include <world.h>
#include <scene.h>
#include <actor.h>
#include <camera.h>

#include <components/recttransform.h>
#include <components/layout.h>
#include <components/widget.h>
#include <components/frame.h>
#include <components/uiloader.h>

#include <editor/undomanager.h>
#include <editor/assetconverter.h>
#include <editor/viewport/cameracontroller.h>

#include "widgetcontroller.h"

const char *gUiLoader("UiLoader");

UiEdit::UiEdit() :
        ui(new Ui::UiEdit),
        m_world(Engine::objectCreate<World>("World")),
        m_scene(Engine::objectCreate<Scene>("Scene", m_world)),
        m_screenActor(Engine::composeActor(gUiLoader, "Screen", m_scene)),
        m_controller(new WidgetController(m_scene, this)),
        m_lastCommand(nullptr) {

    ui->setupUi(this);

    ui->preview->setController(m_controller);
    ui->preview->init();
    ui->preview->setWorld(m_world);
    ui->preview->setSceneView(true);
    ui->preview->setLiveUpdate(true);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    m_controller->frontSide();
    m_controller->blockRotations(true);
    m_controller->blockMovement(true);

    readSettings();

    connect(m_controller, &WidgetController::objectsSelected, this, &UiEdit::objectsSelected);
    connect(m_controller, &WidgetController::sceneUpdated, this, &UiEdit::updated);

}

UiEdit::~UiEdit() {
    writeSettings();

    delete ui;
}

void UiEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("ui.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void UiEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("ui.geometry", ui->splitter->saveState());
}

bool UiEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(this) != m_lastCommand);
}

QStringList UiEdit::suffixes() const {
    return { "ui" };
}

QStringList UiEdit::componentGroups() const {
    return {"Actor", "Components/UI"};
}

void UiEdit::onActivated() {
    emit objectsHierarchyChanged(m_scene);

    emit objectsSelected(m_controller->selected());
}

void UiEdit::onUpdated() {
    emit updated();
}

void UiEdit::onObjectCreate(QString type) {
    UndoManager::instance()->push(new CreateObject(type, m_scene, m_controller));
}

void UiEdit::onObjectsSelected(QList<Object *> objects, bool force) {
    m_controller->onSelectActor(objects);
}

void UiEdit::onObjectsDeleted(QList<Object *> objects) {

}

void UiEdit::onObjectsChanged(const QList<Object *> &objects, QString property, const Variant &value) {
    QString capital = property;
    capital[0] = capital[0].toUpper();
    QString name(QObject::tr("Change %1").arg(capital));

    UndoManager::instance()->push(new ChangeProperty(objects, property, value, m_controller, name));
}

void UiEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        m_settings = { settings };

        m_lastCommand = UndoManager::instance()->lastCommand(this);

        QFile loadFile(settings->source());
        if(!loadFile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open file.");
            return;
        }

        QByteArray data(loadFile.readAll());
        loadFile.close();

        UiLoader *loader = dynamic_cast<UiLoader *>(m_screenActor->component(gUiLoader));
        if(loader) {
            loader->loadFromBuffer(data.toStdString());
        }
    }
}

void UiEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {

        m_lastCommand = UndoManager::instance()->lastCommand(this);
    }
}

void UiEdit::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    m_controller->setSize(width(), height());
}

void UiEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
