#include "particleedit.h"
#include "ui_particleedit.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQmlEngine>
#include <QMessageBox>

#include "../converter/effectconverter.h"

#include <global.h>

#include <editor/viewport/cameractrl.h>

#include <components/scenegraph.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/particlerender.h>
#include <components/camera.h>

ParticleEdit::ParticleEdit() :
        m_modified(false),
        ui(new Ui::ParticleEdit),
        m_effect(nullptr),
        m_builder(new EffectConverter),
        m_controller(nullptr),
        m_render(nullptr),
        m_selectedItem(nullptr) {

    ui->setupUi(this);

    m_controller = new CameraCtrl();
    m_controller->blockMovement(true);
    m_controller->setFree(false);

    SceneGraph *graph = Engine::objectCreate<SceneGraph>("SceneGraph");
    Scene *scene = Engine::objectCreate<Scene>("Scene", graph);

    ui->preview->init();
    ui->preview->setController(m_controller);
    ui->preview->setSceneGraph(graph);

    m_effect = Engine::composeActor("ParticleRender", "ParticleEffect", scene);
    m_render = static_cast<ParticleRender *>(m_effect->component("ParticleRender"));

    startTimer(16);

    connect(m_builder, SIGNAL(effectUpdated()), this, SLOT(onUpdateTemplate()));

    ui->quickWidget->rootContext()->setContextProperty("effectModel", QVariant::fromValue(m_builder->children()));
    ui->quickWidget->setSource(QUrl("qrc:/qml/Emitters.qml"));

    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(emitterSelected(QString)), this, SLOT(onEmitterSelected(QString)));
    connect(item, SIGNAL(emitterCreate()), this, SLOT(onEmitterCreated()));
    connect(item, SIGNAL(emitterDelete(QString)), this, SLOT(onEmitterDeleted(QString)));

    connect(item, SIGNAL(functionSelected(QString,QString)), this, SLOT(onFunctionSelected(QString,QString)));
    connect(item, SIGNAL(functionCreate(QString,QString)), this, SLOT(onFunctionCreated(QString,QString)));
    connect(item, SIGNAL(functionDelete(QString,QString)), this, SLOT(onFunctionDeleted(QString,QString)));
}

ParticleEdit::~ParticleEdit() {
    delete ui;

    delete m_effect;
}

void ParticleEdit::timerEvent(QTimerEvent *) {
    if(m_render) {
        Camera::setCurrent(m_controller->camera());
        m_render->deltaUpdate(1.0f / 60.0f);
        Camera::setCurrent(nullptr);
    }
}

bool ParticleEdit::isModified() const {
    return m_modified;
}

QStringList ParticleEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_builder)->suffixes();
}

void ParticleEdit::onActivated() {
    emit itemSelected(m_selectedItem);
}

void ParticleEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        m_settings = { settings };

        m_render->setEffect(Engine::loadResource<ParticleEffect>(qPrintable(settings->destination())));
        m_builder->load(m_settings.first()->source());

        onNodeSelected(nullptr);

        onUpdateTemplate(false);
    }
}

void ParticleEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {
        m_builder->save(path.isEmpty() ? m_settings.first()->source() : path);
        m_modified = false;
    }
}

void ParticleEdit::onNodeSelected(void *node) {
    m_selectedItem = static_cast<QObject *>(node);
    emit itemSelected(m_selectedItem);
}

void ParticleEdit::onNodeDeleted() {
    onNodeSelected(nullptr);
}

void ParticleEdit::onEmitterSelected(QString emitter) {
    EffectEmitter *obj = m_builder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        onNodeSelected(obj);
    }
}

void ParticleEdit::onEmitterCreated() {
    onNodeSelected(m_builder->createEmitter());
}

void ParticleEdit::onEmitterDeleted(QString name) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Do you want to delete emitter?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int result = msgBox.exec();
    if(result == QMessageBox::Yes) {
        m_builder->deleteEmitter(name);
        onNodeSelected(nullptr);
    }
}

void ParticleEdit::onFunctionSelected(QString emitter, QString function) {
    EffectEmitter *obj = m_builder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        EffectFunction *func = obj->findChild<EffectFunction *>(function);
        if(func) {
            onNodeSelected(func);
        }
    }
}

void ParticleEdit::onFunctionCreated(QString emitter, QString function) {
    EffectEmitter *obj = m_builder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        EffectFunction *func = obj->findChild<EffectFunction *>(function);
        if(func == nullptr) {
            onNodeSelected(m_builder->createFunction(emitter, function));
        } else {
            QMessageBox msgBox(this);
            msgBox.setText(tr("This type of modifier already assigned."));
            msgBox.exec();
        }
    }
}

void ParticleEdit::onFunctionDeleted(QString emitter, QString function) {
    m_builder->deleteFunction(emitter, function);
}

void ParticleEdit::onUpdateTemplate(bool update) {
    m_render->effect()->loadUserData(m_builder->data().toMap());
    m_render->setEffect(m_render->effect());

    QObjectList list = m_builder->children();
    ui->quickWidget->rootContext()->setContextProperty("effectModel", QVariant::fromValue(list));

    m_modified = update;
}

void ParticleEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
