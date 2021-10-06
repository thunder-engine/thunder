#include "particleedit.h"
#include "ui_particleedit.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQmlEngine>
#include <QMessageBox>

#include "effectconverter.h"

#include <global.h>

#include "controllers/cameractrl.h"

#include <components/scene.h>
#include <components/actor.h>
#include <components/particlerender.h>

ParticleEdit::ParticleEdit() :
        m_Modified(false),
        ui(new Ui::ParticleEdit),
        m_pEffect(nullptr),
        m_pBuilder(new EffectConverter),
        m_pRender(nullptr) {

    ui->setupUi(this);

    CameraCtrl *ctrl = new CameraCtrl(ui->preview);
    ctrl->blockMovement(true);
    ctrl->setFree(false);
    ctrl->init(nullptr);

    ui->preview->setController(ctrl);
    ui->preview->setScene(Engine::objectCreate<Scene>("Scene"));

    m_pEffect = Engine::composeActor("ParticleRender", "ParticleEffect", ui->preview->scene());
    m_pRender = static_cast<ParticleRender *>(m_pEffect->component("ParticleRender"));

    startTimer(16);

    connect(m_pBuilder, SIGNAL(effectUpdated()), this, SLOT(onUpdateTemplate()));

    ui->quickWidget->rootContext()->setContextProperty("effectModel", QVariant::fromValue(m_pBuilder->children()));
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/Emitters.qml"));

    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(emitterSelected(QString)), this, SLOT(onEmitterSelected(QString)));
    connect(item, SIGNAL(emitterCreate()), this, SLOT(onEmitterCreated()));
    connect(item, SIGNAL(emitterDelete(QString)), this, SLOT(onEmitterDeleted(QString)));

    connect(item, SIGNAL(functionSelected(QString, QString)), this, SLOT(onFunctionSelected(QString, QString)));
    connect(item, SIGNAL(functionCreate(QString, QString)), this, SLOT(onFunctionCreated(QString, QString)));
    connect(item, SIGNAL(functionDelete(QString, QString)), this, SLOT(onFunctionDeleted(QString, QString)));
}

ParticleEdit::~ParticleEdit() {
    delete ui;

    delete m_pEffect;
}

void ParticleEdit::timerEvent(QTimerEvent *) {
    if(m_pRender) {
        m_pRender->deltaUpdate(1.0f / 60.0f);
    }
}

bool ParticleEdit::isModified() const {
    return m_Modified;
}

QStringList ParticleEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_pBuilder)->suffixes();
}

void ParticleEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings != settings) {
        m_pSettings = settings;

        m_pRender->setEffect(Engine::loadResource<ParticleEffect>(qPrintable(settings->destination())));
        m_pBuilder->load(m_pSettings->source());

        onNodeSelected(nullptr);

        onUpdateTemplate(false);
    }
}

void ParticleEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_pSettings->source().isEmpty()) {
        m_pBuilder->save(path.isEmpty() ? m_pSettings->source() : path);
        m_Modified = false;
    }
}

void ParticleEdit::onNodeSelected(void *node) {
    emit itemSelected(static_cast<QObject *>(node));
}

void ParticleEdit::onNodeDeleted() {
    onNodeSelected(nullptr);
}

void ParticleEdit::onEmitterSelected(QString emitter) {
    EffectEmitter *obj = m_pBuilder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        onNodeSelected(obj);
    }
}

void ParticleEdit::onEmitterCreated() {
    onNodeSelected(m_pBuilder->createEmitter());
}

void ParticleEdit::onEmitterDeleted(QString name) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Do you want to delete emitter?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int result = msgBox.exec();
    if(result == QMessageBox::Yes) {
        m_pBuilder->deleteEmitter(name);
        onNodeSelected(nullptr);
    }
}

void ParticleEdit::onFunctionSelected(QString emitter, QString function) {
    EffectEmitter *obj = m_pBuilder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        EffectFunction *func = obj->findChild<EffectFunction *>(function);
        if(func) {
            onNodeSelected(func);
        }
    }
}

void ParticleEdit::onFunctionCreated(QString emitter, QString function) {
    EffectEmitter *obj = m_pBuilder->findChild<EffectEmitter *>(emitter);
    if(obj) {
        EffectFunction *func = obj->findChild<EffectFunction *>(function);
        if(func == nullptr) {
            onNodeSelected(m_pBuilder->createFunction(emitter, function));
        } else {
            QMessageBox msgBox(this);
            msgBox.setText(tr("This type of modifier already assigned."));
            msgBox.exec();
        }
    }
}

void ParticleEdit::onFunctionDeleted(QString emitter, QString function) {
    m_pBuilder->deleteFunction(emitter, function);
}

void ParticleEdit::onUpdateTemplate(bool update) {
    m_pRender->effect()->loadUserData(m_pBuilder->data().toMap());
    m_pRender->setEffect(m_pRender->effect());

    QObjectList list = m_pBuilder->children();
    ui->quickWidget->rootContext()->setContextProperty("effectModel", QVariant::fromValue(list));

    m_Modified = update;
}

void ParticleEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
